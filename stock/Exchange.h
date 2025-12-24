
#define EXCHANGE_H

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
#include <functional>

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
    double Price;
    int Amount;
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
class Broker;

class BrokerStrategy {
public:
    virtual ~BrokerStrategy() = default;
    virtual void execute(Broker& broker, const std::shared_ptr<Exchange>& exchange) = 0;
};

class Broker : public std::enable_shared_from_this<Broker> {
private:
    size_t id;
    double cash;
    int prodAmount;
    mutable std::mutex mtx;
    static std::atomic<size_t> nextId;

    std::thread tradingThread;
    std::atomic<bool> running_{ false };
    std::unique_ptr<BrokerStrategy> strategy_;

    double lastTradePrice_;

public:
    Broker(double initialCash, int initialProd, std::unique_ptr<BrokerStrategy> strategy);

    Broker(const Broker&) = delete;
    Broker& operator=(const Broker&) = delete;

    ~Broker();

    double giveMoney(double value);
    size_t getId() const;
    double takeMoney(double value);
    int takeProd(int value);
    int giveProd(int value);
    std::pair<double, int> getStatus() const;

    bool placeOrder(const std::shared_ptr<Exchange>& ex, int prAmount, double price, OrderType type);

    void processDeal(const Deal& deal, bool isBuyer);

    double getLastTradePrice() const;
    void updateLastTradePrice(double newPrice);

    void startTrading(const std::shared_ptr<Exchange>& exchange);
    void stopTrading();
};



class OrderBook {
private:
    std::map<double, PriceLevel, std::greater<double>> BuyLevels;
    std::map<double, PriceLevel> SellLevels;
    mutable std::shared_mutex mtx;

public:
    void addOrder(int prodAmount, double price, OrderType type, size_t id);
    std::vector<Deal> MakeDeals();
    double getBestBid() const;
    double getBestAsk() const;
    std::pair<double, double> getSpread() const;
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
    Exchange(std::string name);

    Exchange(const Exchange&) = delete;
    Exchange& operator=(const Exchange&) = delete;

    ~Exchange();

    bool addOrder(int prodAmount, double price, OrderType type, size_t brokerId);
    void processMatching();
    void processSingleDeal(const Deal& deal);
    void registerBroker(const std::shared_ptr<Broker>& b);
    void unregisterBroker(size_t brokerId);
    void feeCollectorLoop();
    void start();
    void stop();

    double getCurrentPrice() const;
    std::pair<double, double> getSpread() const;
    std::vector<Deal> getRecentDeals(int count) const;
    double getTotalFees() const;
    size_t getBrokerCount() const;
};





























class BigShotStrategy : public BrokerStrategy {
private:
    double profitThreshold_;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis_price;
    std::uniform_int_distribution<> dis_amount;
    std::chrono::steady_clock::time_point lastTradeTime;
    const std::chrono::seconds maxWaitForProfit = std::chrono::seconds(15);

public:
    BigShotStrategy(double threshold);

    void execute(Broker& broker, const std::shared_ptr<Exchange>& exchange) override;
};

class GamblerStrategy : public BrokerStrategy {
private:
    std::mt19937 gen;
    std::uniform_real_distribution<> dis_price;
    std::uniform_int_distribution<> dis_amount;
    std::uniform_int_distribution<> dis_action;

public:
    GamblerStrategy();

    void execute(Broker& broker, const std::shared_ptr<Exchange>& exchange) override;
};

class AnalystStrategy : public BrokerStrategy {
private:
    std::vector<Deal> historicalDeals;
    std::mutex histMtx;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis_amount;

public:
    AnalystStrategy();

    void execute(Broker& broker, const std::shared_ptr<Exchange>& exchange) override;
};

void simulateBrokers();

