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