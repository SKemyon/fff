#include "Exchange.h"

std::atomic<size_t> Broker::nextId{ 1 };


Broker::Broker(double initialCash, int initialProd, std::unique_ptr<BrokerStrategy> strategy)
    : id(nextId++), cash(initialCash), prodAmount(initialProd), strategy_(std::move(strategy)) {}

Broker::~Broker() {
    stopTrading();
}

double Broker::giveMoney(double value) {
    std::lock_guard<std::mutex> lock(mtx);
    if (cash >= value) {
        cash -= value;
        return value;
    }
    return -1.0;
}

size_t Broker::getId() const { return id; }

double Broker::takeMoney(double value) {
    std::lock_guard<std::mutex> lock(mtx);
    cash += value;
    return value;
}

int Broker::takeProd(int value) {
    std::lock_guard<std::mutex> lock(mtx);
    prodAmount += value;
    return prodAmount;
}

int Broker::giveProd(int value) {
    std::lock_guard<std::mutex> lock(mtx);
    if (prodAmount >= value) {
        prodAmount -= value;
        return prodAmount;
    }
    return -1;
}

std::pair<double, int> Broker::getStatus() const {
    std::lock_guard<std::mutex> lock(mtx);
    return { cash, prodAmount };
}

bool Broker::placeOrder(const std::shared_ptr<Exchange>& ex, int prAmount, double price, OrderType type) {
    if (prAmount <= 0) return false;

    std::lock_guard<std::mutex> lock(mtx);

    if (type == OrderType::SELL) {
        if (prodAmount < prAmount) return false;
        prodAmount -= prAmount;
    }
    else {
        double totalCost = price * prAmount;
        if (cash < totalCost) return false;
        cash -= totalCost;
    }

    return ex->addOrder(prAmount, price, type, id);
}

void Broker::processDeal(const Deal& deal, bool isBuyer) {
    std::lock_guard<std::mutex> lock(mtx);
    double totalValue = deal.Price * deal.Amount;

    if (isBuyer) {
        prodAmount += deal.Amount;

        if (deal.Amount > 0) lastTradePrice_ = totalValue / deal.Amount;
    }
    else {
        cash += totalValue;
        prodAmount -= deal.Amount;

        if (deal.Amount > 0) lastTradePrice_ = totalValue / deal.Amount;
    }
}

double Broker::getLastTradePrice() const {
    std::lock_guard<std::mutex> lock(mtx);
    return lastTradePrice_;
}

void Broker::updateLastTradePrice(double newPrice) {
    std::lock_guard<std::mutex> lock(mtx);
    lastTradePrice_ = newPrice;
}

void Broker::startTrading(const std::shared_ptr<Exchange>& exchange) {
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) return;

    tradingThread = std::thread([this, exchange]() {
        while (running_) {
            if (strategy_) {
                strategy_->execute(*this, exchange);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });
}

void Broker::stopTrading() {
    bool expected = true;
    if (!running_.compare_exchange_strong(expected, false)) return;

    if (tradingThread.joinable()) {
        tradingThread.join();
    }
}


void OrderBook::addOrder(int prodAmount, double price, OrderType type, size_t id) {
    std::unique_lock<std::shared_mutex> lock(mtx);

    if (type == OrderType::BUY) {
        auto& level = BuyLevels[price];
        level.price = price;
        level.orders.emplace_back(prodAmount, price, type, id);
        level.totalQuantity += prodAmount;
    }
    else {
        auto& level = SellLevels[price];
        level.price = price;
        level.orders.emplace_back(prodAmount, price, type, id);
        level.totalQuantity += prodAmount;
    }
}

std::vector<Deal> OrderBook::MakeDeals() {
    std::unique_lock<std::shared_mutex> lock(mtx);
    std::vector<Deal> deals;

    while (!BuyLevels.empty() && !SellLevels.empty()) {
        auto buyIt = BuyLevels.begin();
        auto sellIt = SellLevels.begin();
        double bestBuy = buyIt->first;
        double bestSell = sellIt->first;

        if (bestBuy < bestSell) break;

        PriceLevel& bLevel = buyIt->second;
        PriceLevel& sLevel = sellIt->second;

        while (!bLevel.orders.empty() && !sLevel.orders.empty() &&
            bLevel.totalQuantity > 0 && sLevel.totalQuantity > 0) {

            Order& bOrder = bLevel.orders.front();
            Order& sOrder = sLevel.orders.front();

            int qty = std::min(bOrder.amount, sOrder.amount);
            double price = bestSell;

            deals.emplace_back(sOrder, bOrder, qty, price);

            bOrder.amount -= qty;
            sOrder.amount -= qty;
            bLevel.totalQuantity -= qty;
            sLevel.totalQuantity -= qty;

            bLevel.popFrontIfEmpty();
            sLevel.popFrontIfEmpty();
        }

        if (bLevel.totalQuantity <= 0 || bLevel.orders.empty()) {
            BuyLevels.erase(buyIt);
        }
        if (sLevel.totalQuantity <= 0 || sLevel.orders.empty()) {
            SellLevels.erase(sellIt);
        }
    }

    return deals;
}

double OrderBook::getBestBid() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    return BuyLevels.empty() ? 0.0 : BuyLevels.begin()->first;
}

double OrderBook::getBestAsk() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    return SellLevels.empty() ? std::numeric_limits<double>::max() : SellLevels.begin()->first;
}

std::pair<double, double> OrderBook::getSpread() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    double bid = getBestBid();
    double ask = getBestAsk();
    return { bid, ask };
}


Exchange::Exchange(std::string name) : id(0), productName(name) {}

Exchange::~Exchange() {
    stop();
}

bool Exchange::addOrder(int prodAmount, double price, OrderType type, size_t brokerId) {
    if (prodAmount <= 0 || price <= 0) return false;

    {
        std::shared_lock<std::shared_mutex> lock(brokers_mtx);
        if (brokers.find(brokerId) == brokers.end()) return false;
    }

    book.addOrder(prodAmount, price, type, brokerId);

    std::lock_guard<std::mutex> lock(cv_m);
    cv.notify_one();

    return true;
}

void Exchange::processMatching() {
    while (running) {
        auto deals = book.MakeDeals();

        if (!deals.empty()) {
            std::lock_guard<std::mutex> lock(deals_mtx);
            for (auto& deal : deals) {
                recentDeals.push_back(deal);
                processSingleDeal(deal);

                double midPrice = (book.getBestBid() + book.getBestAsk()) / 2.0;
                if (midPrice > 0) {
                    curPrice.store(midPrice, std::memory_order_relaxed);
                }
            }

            if (recentDeals.size() > 1000) {
                recentDeals.erase(recentDeals.begin(), recentDeals.begin() + 500);
            }
        }

        std::unique_lock<std::mutex> lock(cv_m);
        cv.wait_for(lock, std::chrono::milliseconds(100), [this]() { return !running; });
    }
}

void Exchange::processSingleDeal(const Deal& deal) {
    std::shared_lock<std::shared_mutex> lock(brokers_mtx);

    auto buyerIt = brokers.find(deal.BuyId);
    auto sellerIt = brokers.find(deal.SellId);

    if (buyerIt != brokers.end() && sellerIt != brokers.end()) {
        buyerIt->second->processDeal(deal, true);
        sellerIt->second->processDeal(deal, false);

        double fee = deal.Price * deal.Amount * 0.001;
        totalFees += fee;
    }
}

void Exchange::registerBroker(const std::shared_ptr<Broker>& b) {
    std::unique_lock<std::shared_mutex> lock(brokers_mtx);
    brokers[b->getId()] = b;
}

void Exchange::unregisterBroker(size_t brokerId) {
    std::unique_lock<std::shared_mutex> lock(brokers_mtx);
    brokers.erase(brokerId);
}

void Exchange::feeCollectorLoop() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::minutes(1));

        std::shared_lock<std::shared_mutex> lock(brokers_mtx);
        double feePerBroker = BROKER_FEE_PER_MINUTE;

        for (auto& [id, broker] : brokers) {
            broker->giveMoney(-feePerBroker);
            totalFees += feePerBroker;
        }
    }
}

void Exchange::start() {
    bool expected = false;
    if (!running.compare_exchange_strong(expected, true)) return;

    matchingThread = std::thread([this]() { processMatching(); });
    feeThread = std::thread([this]() { feeCollectorLoop(); });

    std::cout << "Exchange started. product: " << productName << std::endl;
}

void Exchange::stop() {
    bool expected = true;
    if (!running.compare_exchange_strong(expected, false)) return;

    cv.notify_all();

    if (matchingThread.joinable()) matchingThread.join();
    if (feeThread.joinable()) feeThread.join();

    std::cout << "Exchange stopped. product: " << productName << std::endl;
}

double Exchange::getCurrentPrice() const {
    return curPrice.load(std::memory_order_relaxed);
}

std::pair<double, double> Exchange::getSpread() const {
    return book.getSpread();
}

std::vector<Deal> Exchange::getRecentDeals(int count) const {
    std::lock_guard<std::mutex> lock(deals_mtx);
    count = std::min(count, static_cast<int>(recentDeals.size()));
    return std::vector<Deal>(recentDeals.end() - count, recentDeals.end());
}

double Exchange::getTotalFees() const {
    return totalFees.load();
}

size_t Exchange::getBrokerCount() const {
    std::shared_lock<std::shared_mutex> lock(brokers_mtx);
    return brokers.size();
}






















BigShotStrategy::BigShotStrategy(double threshold) : profitThreshold_(threshold), gen(std::random_device{}()),
dis_price(1.0, 10.0), dis_amount(1, 5) {
    lastTradeTime = std::chrono::steady_clock::now();
}

GamblerStrategy::GamblerStrategy() : gen(std::random_device{}()), dis_price(1.0, 10.0), dis_amount(1, 3), dis_action(0, 1) {}

AnalystStrategy::AnalystStrategy() : gen(std::random_device{}()), dis_amount(1, 2) {}









//���������
void BigShotStrategy::execute(Broker& broker, const std::shared_ptr<Exchange>& exchange) {
    auto [cash, prod] = broker.getStatus();
    double currentPrice = exchange->getCurrentPrice();

    auto [bid, ask] = exchange->getSpread();

    if (currentPrice > 0 && broker.getLastTradePrice() > 0 &&
        currentPrice > broker.getLastTradePrice() * (1.0 + profitThreshold_) && prod > 0) {
        if (exchange->addOrder(prod, currentPrice, OrderType::SELL, broker.getId())) {
            broker.updateLastTradePrice(currentPrice);
            lastTradeTime = std::chrono::steady_clock::now();
            std::cout << "[BigShot " << broker.getId() << "] Sold at " << currentPrice << " for profit. Prod: " << prod << std::endl;
        }
    }
    else if (currentPrice > 0 && cash >= currentPrice &&
        ask > 0 &&
        ask >= currentPrice * (1.0 + profitThreshold_)) {
        int amount = std::min(static_cast<int>(cash / currentPrice), dis_amount(gen));
        if (amount > 0 && exchange->addOrder(amount, currentPrice, OrderType::BUY, broker.getId())) {
            std::cout << "[BigShot " << broker.getId() << "] Placed BUY order at " << currentPrice << ", expecting high profit." << std::endl;
        }
    }
    else if (currentPrice > 0 && broker.getLastTradePrice() > 0 &&
        currentPrice < broker.getLastTradePrice() * (1.0 - profitThreshold_) && prod > 0) {
        if (exchange->addOrder(prod, currentPrice, OrderType::SELL, broker.getId())) {
            broker.updateLastTradePrice(currentPrice);
            lastTradeTime = std::chrono::steady_clock::now();
            std::cout << "[BigShot " << broker.getId() << "] Sold at " << currentPrice << " to minimize loss. Prod: " << prod << std::endl;
        }
    }
    else if (std::chrono::steady_clock::now() - lastTradeTime > maxWaitForProfit) {
        if (prod > 0) {
            if (exchange->addOrder(prod, currentPrice > 0 ? currentPrice : 1.0, OrderType::SELL, broker.getId())) {
                broker.updateLastTradePrice(currentPrice > 0 ? currentPrice : 1.0);
                lastTradeTime = std::chrono::steady_clock::now();
                std::cout << "[BigShot " << broker.getId() << "] Minimizing loss by selling all." << std::endl;
            }
        }
    }
    else {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void GamblerStrategy::execute(Broker& broker, const std::shared_ptr<Exchange>& exchange) {
    auto [cash, prod] = broker.getStatus();
    double currentPrice = exchange->getCurrentPrice();

    int action = dis_action(gen);
    double price = dis_price(gen);
    int amount = dis_amount(gen);

    if (action == 0) {
        amount = std::min(amount, static_cast<int>(cash / price));
        if (amount > 0 && exchange->addOrder(amount, price, OrderType::BUY, broker.getId())) {
            std::cout << "[Gambler " << broker.getId() << "] Placed random BUY order." << std::endl;
        }
    }
    else {
        amount = std::min(amount, prod);
        if (amount > 0 && exchange->addOrder(amount, price, OrderType::SELL, broker.getId())) {
            std::cout << "[Gambler " << broker.getId() << "] Placed random SELL order." << std::endl;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

void AnalystStrategy::execute(Broker& broker, const std::shared_ptr<Exchange>& exchange) {
    auto [cash, prod] = broker.getStatus();
    double currentPrice = exchange->getCurrentPrice();

    auto recentDeals = exchange->getRecentDeals(50);
    {
        std::lock_guard<std::mutex> lock(histMtx);
        historicalDeals = recentDeals;
    }

    double avgPrice = 0.0;
    if (!historicalDeals.empty()) {
        double sum = 0.0;
        for (const auto& d : historicalDeals) {
            sum += d.Price;
        }
        avgPrice = sum / historicalDeals.size();
    }

    if (avgPrice > 0 && currentPrice < avgPrice * 0.95 && cash >= currentPrice) {
        int amount = std::min(dis_amount(gen), static_cast<int>(cash / currentPrice));
        if (amount > 0 && exchange->addOrder(amount, currentPrice, OrderType::BUY, broker.getId())) {
            std::cout << "[Analyst " << broker.getId() << "] BUY signal based on analysis. Avg: " << avgPrice << ", Current: " << currentPrice << std::endl;
        }
    }
    else if (avgPrice > 0 && currentPrice > avgPrice * 1.05 && prod > 0) {
        int amount = std::min(dis_amount(gen), prod);
        if (amount > 0 && exchange->addOrder(amount, currentPrice, OrderType::SELL, broker.getId())) {
            std::cout << "[Analyst " << broker.getId() << "] SELL signal based on analysis. Avg: " << avgPrice << ", Current: " << currentPrice << std::endl;
        }
    }
    else {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void simulateBrokers() {
    auto exchange = std::make_shared<Exchange>("SampleProduct");
    exchange->start();

    std::vector<std::shared_ptr<Broker>> brokerList;

    for (int i = 0; i < 2; ++i) {
        auto bigShot = std::make_shared<Broker>(1000.0, 10, std::make_unique<BigShotStrategy>(0.1));
        exchange->registerBroker(bigShot);
        brokerList.push_back(bigShot);
        bigShot->startTrading(exchange);
        std::cout << "Started BigShot Broker " << bigShot->getId() << std::endl;
    }

    for (int i = 0; i < 2; ++i) {
        auto gambler = std::make_shared<Broker>(500.0, 5, std::make_unique<GamblerStrategy>());
        exchange->registerBroker(gambler);
        brokerList.push_back(gambler);
        gambler->startTrading(exchange);
        std::cout << "Started Gambler Broker " << gambler->getId() << std::endl;
    }

    for (int i = 0; i < 2; ++i) {
        auto analyst = std::make_shared<Broker>(800.0, 8, std::make_unique<AnalystStrategy>());
        exchange->registerBroker(analyst);
        brokerList.push_back(analyst);
        analyst->startTrading(exchange);
        std::cout << "Started Analyst Broker " << analyst->getId() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(30));

    for (auto& broker : brokerList) {
        broker->stopTrading();
        std::cout << "Stopped Broker " << broker->getId() << std::endl;
    }

    exchange->stop();

    std::cout << "\n--- Final Broker Status ---" << std::endl;
    for (const auto& broker : brokerList) {
        auto status = broker->getStatus();
        std::cout << "Broker " << broker->getId() << ": Cash=" << status.first << ", Prod=" << status.second << std::endl;
    }
    std::cout << "Total Fees Collected: " << exchange->getTotalFees() << std::endl;
}

