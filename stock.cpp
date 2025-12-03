

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <map>
#include <queue>
#include <random>
#include <chrono>
#include <future>
#include <algorithm>
#include <condition_variable>
#include <string>
#include <memory>
#include <unordered_map>
#include <deque>
#include <limits>
#include <shared_mutex>



const double BROKER_FEE_PER_MINUTE = 1.0;

enum class OrderType {
    BUY,
    SELL
};

class Order {
public:
    Order(int prodAmount, double price, OrderType type, size_t id)
        : amount(prodAmount), Price(price), Type(type), OwnerId(id),
        timestamp(std::chrono::system_clock::now()) {}

    Order() = default;

    size_t OwnerId;
    OrderType Type;
    double Price;
    int amount;
    std::chrono::system_clock::time_point timestamp;
};

class Deal {
public:
    Deal(const Order& sellOrder, const Order& buyOrder, int amount, double price)
        : SellId(sellOrder.OwnerId), BuyId(buyOrder.OwnerId),
        Price(price), Amount(amount),
        timestamp(std::chrono::system_clock::now()) {}

    Deal() = default;

    size_t SellId;
    size_t BuyId;
    double Price;//-
    int Amount;//-
    std::chrono::system_clock::time_point timestamp;
};

class PriceLevel {
public:
    double price;
    int totalQuantity;
    std::deque<Order> orders;

    PriceLevel() : price(0.0), totalQuantity(0) {}
    PriceLevel(double p) : price(p), totalQuantity(0) {}

    void popFrontIfEmpty() {
        while (!orders.empty() && orders.front().amount <= 0) {
            orders.pop_front();
            
        }
    }
};


class Exchange;

class Broker : public std::enable_shared_from_this<Broker> {
private:
    size_t id;
    double cash;
    int prodAmount;
    mutable std::mutex mtx;
    static std::atomic<size_t> nextId;

public:
    Broker(double initialCash, int initialProd) : id(nextId++), cash(initialCash), prodAmount(initialProd) {}

    Broker(const Broker&) = delete;
    Broker& operator=(const Broker&) = delete;

    double giveMoney(double value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (cash >= value) {
            cash -= value;
            return value;
        }
        return -1.0;
    }

    size_t getId() const { return id; }

    double takeMoney(double value) {
        std::lock_guard<std::mutex> lock(mtx);
        cash += value;
        return value;
    }

    int takeProd(int value) {
        std::lock_guard<std::mutex> lock(mtx);
        prodAmount += value;
        return prodAmount;
    }

    int giveProd(int value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (prodAmount >= value) {
            prodAmount -= value;
            return prodAmount;
        }
        return -1;
    }

    std::pair<double, int> getStatus() const {
        std::lock_guard<std::mutex> lock(mtx);
        return { cash, prodAmount };
    }

    bool addOrder(std::shared_ptr<Exchange> ex, int prAmount, double price, OrderType type);

    void processDeal(const Deal& deal, bool isBuyer) {
        std::lock_guard<std::mutex> lock(mtx);
        double totalValue = deal.Price * deal.Amount;

        if (isBuyer) {
            prodAmount += deal.Amount;
        }
        else {
            cash += totalValue;
            prodAmount -= deal.Amount;
        }
    }
};

std::atomic<size_t> Broker::nextId{ 1 };

class OrderBook {
private:
    std::map<double, PriceLevel, std::greater<double>> BuyLevels;
    std::map<double, PriceLevel> SellLevels;
    mutable std::shared_mutex mtx;

public:
    void addOrder(int prodAmount, double price, OrderType type, size_t id) {
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

    std::vector<Deal> MakeDeals() {
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

    double getBestBid() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return BuyLevels.empty() ? 0.0 : BuyLevels.begin()->first;
    }

    double getBestAsk() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return SellLevels.empty() ? std::numeric_limits<double>::max() : SellLevels.begin()->first;
    }

    std::pair<double, double> getSpread() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        double bid = getBestBid();
        double ask = getBestAsk();
        return { bid, ask };
    }
};

class Exchange {
private:
    size_t id;
    std::string productName;
    std::atomic<double> curPrice{ 0.0 };
    OrderBook book;
    std::thread matchingThread;
    std::thread feeThread;
    std::atomic<bool> running{ false };
    std::condition_variable cv;
    std::mutex cv_m;
    mutable std::shared_mutex brokers_mtx;
    std::unordered_map<size_t, std::shared_ptr<Broker>> brokers;
    std::vector<Deal> recentDeals;
    mutable std::mutex deals_mtx;
    std::atomic<double> totalFees{ 0.0 };

public:
    Exchange(std::string name) : id(0), productName(name) {}

    Exchange(const Exchange&) = delete;
    Exchange& operator=(const Exchange&) = delete;

    ~Exchange() {
        stop();
    }

    bool addOrder(int prodAmount, double price, OrderType type, size_t brokerId) {
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

    void processMatching() {
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

    void processSingleDeal(const Deal& deal) {
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

    void registerBroker(const std::shared_ptr<Broker>& b) {
        std::unique_lock<std::shared_mutex> lock(brokers_mtx);
        brokers[b->getId()] = b;
    }

    void unregisterBroker(size_t brokerId) {
        std::unique_lock<std::shared_mutex> lock(brokers_mtx);
        brokers.erase(brokerId);
    }

    void feeCollectorLoop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::minutes(1));

            std::shared_lock<std::shared_mutex> lock(brokers_mtx);
            double feePerBroker = BROKER_FEE_PER_MINUTE;

            for (auto& [id, broker] : brokers) {
                broker->giveMoney(feePerBroker);
                totalFees += feePerBroker;
            }
        }
    }

    void start() {
        bool expected = false;
        if (!running.compare_exchange_strong(expected, true)) return;

        matchingThread = std::thread([this]() { processMatching(); });
        feeThread = std::thread([this]() { feeCollectorLoop(); });

        std::cout << "Exchange started for product: " << productName << std::endl;
    }

    void stop() {
        bool expected = true;
        if (!running.compare_exchange_strong(expected, false)) return;

        cv.notify_all();

        if (matchingThread.joinable()) matchingThread.join();
        if (feeThread.joinable()) feeThread.join();

        std::cout << "Exchange stopped for product: " << productName << std::endl;
    }

    double getCurrentPrice() const {
        return curPrice.load(std::memory_order_relaxed);
    }

    std::pair<double, double> getSpread() const {
        return book.getSpread();
    }

    std::vector<Deal> getRecentDeals(int count) const {
        std::lock_guard<std::mutex> lock(deals_mtx);
        count = std::min(count, static_cast<int>(recentDeals.size()));
        return std::vector<Deal>(recentDeals.end() - count, recentDeals.end());
    }

    double getTotalFees() const {
        return totalFees.load();
    }

    size_t getBrokerCount() const {
        std::shared_lock<std::shared_mutex> lock(brokers_mtx);
        return brokers.size();
    }
};



bool Broker::addOrder(std::shared_ptr<Exchange> ex, int prAmount, double price, OrderType type) {
    if (prAmount <= 0) return false;

    std::lock_guard<std::mutex> lock(mtx);

    if (type == OrderType::SELL) {
        if (prodAmount < prAmount) return false;
        prodAmount -= prAmount;
    }
    else { // BUY
        double totalCost = price * prAmount;
        if (cash < totalCost) return false;
        cash -= totalCost;
    }

    ex->addOrder(prAmount, price, type, id);
    return true;
}


int mmmain() {
    auto exchange = std::make_shared<Exchange>("AAAAAA");

    auto broker1 = std::make_shared<Broker>(10000.0, 100);
    auto broker2 = std::make_shared<Broker>(15000.0, 50);
    auto broker3 = std::make_shared<Broker>(20000.0, 200);

    exchange->registerBroker(broker1);
    exchange->registerBroker(broker2);
    exchange->registerBroker(broker3);

    exchange->start();

 
    broker1->addOrder(exchange, 10, 150.0, OrderType::SELL);
    broker2->addOrder(exchange, 5, 149.0, OrderType::SELL);
    broker3->addOrder(exchange, 15, 148.0, OrderType::BUY);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto spread = exchange->getSpread();
    std::cout << "Bid: " << spread.first << ", Ask: " << spread.second << std::endl;
    std::cout << "Curr price: " << exchange->getCurrentPrice() << std::endl;

    auto deals = exchange->getRecentDeals(5);
    std::cout << "Recent deals: " << deals.size() << std::endl;

    auto status1 = broker1->getStatus();
    auto status2 = broker2->getStatus();
    auto status3 = broker3->getStatus();

    std::cout << "Broker1 - Cash: " << status1.first << ", Prod: " << status1.second << std::endl;
    std::cout << "Broker2 - Cash: " << status2.first << ", Prod: " << status2.second << std::endl;
    std::cout << "Broker3 - Cash: " << status3.first << ", Prod: " << status3.second << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(3));
    exchange->stop();

    return 0;
}




// Ïðèìåð èñïîëüçîâàíèÿ
int main() {

    setlocale(LC_ALL, "Ru");
    std::cout << "=== Çàïóñê áèðæåâîé ñèñòåìû ===" << std::endl;

    // Ñîçäàåì áèðæó äëÿ àêöèé Apple
    auto exchange = std::make_shared<Exchange>("AAPL");

    // Ñîçäàåì áðîêåðîâ ñ ðàçíûì êàïèòàëîì
    auto broker1 = std::make_shared<Broker>(100000.0, 1000);  // 100ê äåíåã, 1000 àêöèé
    auto broker2 = std::make_shared<Broker>(150000.0, 500);   // 150ê äåíåã, 500 àêöèé
    auto broker3 = std::make_shared<Broker>(200000.0, 1500);  // 200ê äåíåã, 1500 àêöèé
    auto broker4 = std::make_shared<Broker>(50000.0, 2000);   // 50ê äåíåã, 2000 àêöèé

    std::cout << "\nÁðîêåðû ñîçäàíû:" << std::endl;
    std::cout << "Áðîêåð 1: ID=" << broker1->getId() << ", Äåíüãè=" << broker1->getStatus().first
        << ", Àêöèè=" << broker1->getStatus().second << std::endl;
    std::cout << "Áðîêåð 2: ID=" << broker2->getId() << ", Äåíüãè=" << broker2->getStatus().first
        << ", Àêöèè=" << broker2->getStatus().second << std::endl;
    std::cout << "Áðîêåð 3: ID=" << broker3->getId() << ", Äåíüãè=" << broker3->getStatus().first
        << ", Àêöèè=" << broker3->getStatus().second << std::endl;
    std::cout << "Áðîêåð 4: ID=" << broker4->getId() << ", Äåíüãè=" << broker4->getStatus().first
        << ", Àêöèè=" << broker4->getStatus().second << std::endl;

    // Ðåãèñòðèðóåì áðîêåðîâ íà áèðæå
    exchange->registerBroker(broker1);
    exchange->registerBroker(broker2);
    exchange->registerBroker(broker3);
    exchange->registerBroker(broker4);

    std::cout << "\nÇàïóñê áèðæè..." << std::endl;
    exchange->start();

    // Äàåì áèðæå âðåìÿ íà èíèöèàëèçàöèþ
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "\n=== Íà÷àëî òîðãîâ ===" << std::endl;

    // ÐÀÓÍÄ 1: Ïðîäàâöû âûñòàâëÿþò îðäåðà
    std::cout << "\n1. Ïðîäàâöû âûñòàâëÿþò îðäåðà íà ïðîäàæó:" << std::endl;

    // Ïðîäàâåö 1: ïðîäàåò äîðîãî
    if (broker1->addOrder(exchange, 100, 155.0, OrderType::SELL)) {
        std::cout << "   Áðîêåð " << broker1->getId() << ": SELL 100 @ $155.00" << std::endl;
    }

    // Ïðîäàâåö 2: ïðîäàåò ÷óòü äåøåâëå
    if (broker2->addOrder(exchange, 50, 154.5, OrderType::SELL)) {
        std::cout << "   Áðîêåð " << broker2->getId() << ": SELL 50 @ $154.50" << std::endl;
    }

    // Ïðîäàâåö 3: ïðîäàåò åùå äåøåâëå
    if (broker3->addOrder(exchange, 150, 153.0, OrderType::SELL)) {
        std::cout << "   Áðîêåð " << broker3->getId() << ": SELL 150 @ $153.00" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto spread1 = exchange->getSpread();
    std::cout << "   Ñïðåä: Bid=" << spread1.first << ", Ask=" << spread1.second << std::endl;

    // ÐÀÓÍÄ 2: Ïîêóïàòåëè âûõîäÿò íà ðûíîê
    std::cout << "\n2. Ïîêóïàòåëè âûñòàâëÿþò îðäåðà íà ïîêóïêó:" << std::endl;

    // Ïîêóïàòåëü 4: õî÷åò êóïèòü äåøåâî (íå ñìàò÷èòñÿ)
    if (broker4->addOrder(exchange, 200, 152.0, OrderType::BUY)) {
        std::cout << "   Áðîêåð " << broker4->getId() << ": BUY 200 @ $152.00 (íå ñìàò÷èòñÿ)" << std::endl;
    }

    // Ïîêóïàòåëü 3: àãðåññèâíàÿ ïîêóïêà (ñìàò÷èòñÿ ñ ïðîäàâöîì 3)
    if (broker3->addOrder(exchange, 100, 154.0, OrderType::BUY)) {
        std::cout << "   Áðîêåð " << broker3->getId() << ": BUY 100 @ $154.00" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    auto deals1 = exchange->getRecentDeals(10);
    if (!deals1.empty()) {
        std::cout << "\n   Ñäåëêè âûïîëíåíû: " << deals1.size() << " øò." << std::endl;
        for (const auto& deal : deals1) {
            std::cout << "   - Ïðîäàâåö " << deal.SellId << " -> Ïîêóïàòåëü " << deal.BuyId
                << ": " << deal.Amount << " àêöèé @ $" << deal.Price
                << " (Ñóììà: $" << deal.Price * deal.Amount << ")" << std::endl;
        }
    }

    std::cout << "\n   Òåêóùàÿ öåíà: $" << exchange->getCurrentPrice() << std::endl;

    // ÐÀÓÍÄ 3: Áîëüøàÿ ñäåëêà
    std::cout << "\n3. Áîëüøàÿ ñäåëêà:" << std::endl;

    // Ïðîäàâåö 4: ïðîäàåò ìíîãî àêöèé
    if (broker4->addOrder(exchange, 500, 152.5, OrderType::SELL)) {
        std::cout << "   Áðîêåð " << broker4->getId() << ": SELL 500 @ $152.50" << std::endl;
    }

    // Ïîêóïàòåëü 1: ïîêóïàåò ìíîãî àêöèé
    if (broker1->addOrder(exchange, 300, 153.0, OrderType::BUY)) {
        std::cout << "   Áðîêåð " << broker1->getId() << ": BUY 300 @ $153.00" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    // ÐÀÓÍÄ 4: Íåñêîëüêî ìåëêèõ ñäåëîê
    std::cout << "\n4. Íåñêîëüêî ìåëêèõ ñäåëîê:" << std::endl;

    // Âûñòàâëÿåì íåñêîëüêî îðäåðîâ ñ çàäåðæêîé
    std::vector<std::thread> orderThreads;

    orderThreads.emplace_back([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        broker2->addOrder(exchange, 25, 152.8, OrderType::SELL);
        std::cout << "   Áðîêåð " << broker2->getId() << ": SELL 25 @ $152.80" << std::endl;
        });

    orderThreads.emplace_back([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        broker3->addOrder(exchange, 10, 153.2, OrderType::BUY);
        std::cout << "   Áðîêåð " << broker3->getId() << ": BUY 10 @ $153.20" << std::endl;
        });

    orderThreads.emplace_back([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        broker4->addOrder(exchange, 15, 152.9, OrderType::SELL);
        std::cout << "   Áðîêåð " << broker4->getId() << ": SELL 15 @ $152.90" << std::endl;
        });

    orderThreads.emplace_back([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        broker1->addOrder(exchange, 20, 153.1, OrderType::BUY);
        std::cout << "   Áðîêåð " << broker1->getId() << ": BUY 20 @ $153.10" << std::endl;
        });

    // Æäåì çàâåðøåíèÿ âñåõ îðäåðíûõ ïîòîêîâ
    for (auto& t : orderThreads) {
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // ÐÀÓÍÄ 5: Ìàðêåò-îðäåðà (ïî òåêóùåé öåíå)
    std::cout << "\n5. Àãðåññèâíûå îðäåðà (ïî ðûíêó):" << std::endl;

    // Ïîëó÷àåì òåêóùèé ñïðåä
    auto currentSpread = exchange->getSpread();
    double marketBuyPrice = currentSpread.second + 0.1;  // ×óòü âûøå ask
    double marketSellPrice = currentSpread.first - 0.1;  // ×óòü íèæå bid

    std::cout << "   Òåêóùèé ñïðåä: Bid=$" << currentSpread.first
        << ", Ask=$" << currentSpread.second << std::endl;

    // Àãðåññèâíàÿ ïîêóïêà
    if (broker3->addOrder(exchange, 75, marketBuyPrice, OrderType::BUY)) {
        std::cout << "   Áðîêåð " << broker3->getId() << ": BUY 75 @ $" << marketBuyPrice
            << " (àãðåññèâíî)" << std::endl;
    }

    // Àãðåññèâíàÿ ïðîäàæà
    if (broker4->addOrder(exchange, 60, marketSellPrice, OrderType::SELL)) {
        std::cout << "   Áðîêåð " << broker4->getId() << ": SELL 60 @ $" << marketSellPrice
            << " (àãðåññèâíî)" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Ïîêàçàòü èòîãè
    std::cout << "\n=== Èòîãè òîðãîâ ===" << std::endl;

    auto allDeals = exchange->getRecentDeals(100);
    std::cout << "\nÂñåãî ñäåëîê: " << allDeals.size() << std::endl;

    double totalVolume = 0.0;
    for (const auto& deal : allDeals) {
        totalVolume += deal.Price * deal.Amount;
    }
    std::cout << "Îáùèé îáúåì òîðãîâ: $" << totalVolume << std::endl;

    std::cout << "Êîìèññèÿ áèðæè: $" << exchange->getTotalFees() << std::endl;
    std::cout << "Òåêóùàÿ öåíà: $" << exchange->getCurrentPrice() << std::endl;

    auto finalSpread = exchange->getSpread();
    std::cout << "Ôèíàëüíûé ñïðåä: Bid=$" << finalSpread.first
        << ", Ask=$" << finalSpread.second << std::endl;

    std::cout << "\nÑîñòîÿíèå áðîêåðîâ:" << std::endl;
    auto s1 = broker1->getStatus();
    auto s2 = broker2->getStatus();
    auto s3 = broker3->getStatus();
    auto s4 = broker4->getStatus();

    std::cout << "Áðîêåð " << broker1->getId() << ": Äåíüãè=$" << s1.first
        << ", Àêöèè=" << s1.second << std::endl;
    std::cout << "Áðîêåð " << broker2->getId() << ": Äåíüãè=$" << s2.first
        << ", Àêöèè=" << s2.second << std::endl;
    std::cout << "Áðîêåð " << broker3->getId() << ": Äåíüãè=$" << s3.first
        << ", Àêöèè=" << s3.second << std::endl;
    std::cout << "Áðîêåð " << broker4->getId() << ": Äåíüãè=$" << s4.first
        << ", Àêöèè=" << s4.second << std::endl;

    // Ïîäîæäåì ñáîð êîìèññèè (ðàç â ìèíóòó)
    std::cout << "\nÎæèäàíèå ñáîðà êîìèññèé (60 ñåêóíä)..." << std::endl;
    std::cout << "(Íàæìèòå Ctrl+C äëÿ äîñðî÷íîé îñòàíîâêè)" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "\nÊîìèññèÿ ïîñëå ñáîðà: $" << exchange->getTotalFees() << std::endl;

    // Îñòàíàâëèâàåì áèðæó
    std::cout << "\nÎñòàíîâêà áèðæè..." << std::endl;
    exchange->stop();

    std::cout << "\n=== Òîðãè çàâåðøåíû ===" << std::endl;

    return 0;

}
