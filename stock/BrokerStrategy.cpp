#include "Exchange.h"

BigShotStrategy::BigShotStrategy(double threshold) : profitThreshold_(threshold), gen(std::random_device{}()),
dis_price(1.0, 10.0), dis_amount(1, 5) {
    lastTradeTime = std::chrono::steady_clock::now();
}

GamblerStrategy::GamblerStrategy() : gen(std::random_device{}()), dis_price(1.0, 10.0), dis_amount(1, 3), dis_action(0, 1) {}

AnalystStrategy::AnalystStrategy() : gen(std::random_device{}()), dis_amount(1, 2) {}

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