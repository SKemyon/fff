#include "Exchange.h"

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