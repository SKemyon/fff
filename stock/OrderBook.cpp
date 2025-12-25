#include "Exchange.h"

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