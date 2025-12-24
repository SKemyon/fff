#include <gtest/gtest.h>
#include "Exchange.h" // Убедитесь, что ваш Exchange.h подключается корректно
#include <thread>
#include <chrono>

TEST(OrderTest, ConstructorAndFields) {
    Order order(10, 100.5, OrderType::BUY, 123);

    EXPECT_EQ(order.amount, 10);
    EXPECT_DOUBLE_EQ(order.Price, 100.5);
    EXPECT_EQ(order.Type, OrderType::BUY);
    EXPECT_EQ(order.OwnerId, 123);
}


TEST(DealTest, ConstructorAndFields) {
    Order sellOrder(5, 99.0, OrderType::SELL, 1);
    Order buyOrder(5, 101.0, OrderType::BUY, 2);
    Deal deal(sellOrder, buyOrder, 5, 100.0);

    EXPECT_EQ(deal.SellId, 1);
    EXPECT_EQ(deal.BuyId, 2);
    EXPECT_DOUBLE_EQ(deal.Price, 100.0);
    EXPECT_EQ(deal.Amount, 5);
}


TEST(PriceLevelTest, InitializationAndPopFrontIfEmpty) {
    PriceLevel level(100.0);

    EXPECT_DOUBLE_EQ(level.price, 100.0);
    EXPECT_EQ(level.totalQuantity, 0);
    EXPECT_TRUE(level.orders.empty());

    level.orders.emplace_back(10, 100.0, OrderType::BUY, 1);
    level.totalQuantity += 10;

    EXPECT_EQ(level.orders.size(), 1);
    EXPECT_EQ(level.totalQuantity, 10);

    level.orders.front().amount = 0;
    level.popFrontIfEmpty();

    EXPECT_TRUE(level.orders.empty());
}


TEST(BrokerTest, InitializationAndStatus) {
    auto strategy = std::make_unique<BigShotStrategy>(0.1);
    Broker broker(1000.0, 10, std::move(strategy));

    auto status = broker.getStatus();
    EXPECT_DOUBLE_EQ(status.first, 1000.0);
    EXPECT_EQ(status.second, 10);
    EXPECT_GT(broker.getId(), 0);
}

TEST(BrokerTest, GiveAndTakeMoney) {
    auto strategy = std::make_unique<BigShotStrategy>(0.1);
    Broker broker(1000.0, 10, std::move(strategy));

    EXPECT_DOUBLE_EQ(broker.giveMoney(100.0), 100.0);
    auto status = broker.getStatus();
    EXPECT_DOUBLE_EQ(status.first, 900.0);

    EXPECT_DOUBLE_EQ(broker.takeMoney(50.0), 50.0);
    status = broker.getStatus();
    EXPECT_DOUBLE_EQ(status.first, 950.0);

    EXPECT_DOUBLE_EQ(broker.giveMoney(1000.0), -1.0); // Недостаточно средств
}

TEST(BrokerTest, GiveAndTakeProd) {
    auto strategy = std::make_unique<BigShotStrategy>(0.1);
    Broker broker(1000.0, 10, std::move(strategy));

    EXPECT_EQ(broker.giveProd(3), 7);
    auto status = broker.getStatus();
    EXPECT_EQ(status.second, 7);

    EXPECT_EQ(broker.takeProd(2), 9);
    status = broker.getStatus();
    EXPECT_EQ(status.second, 9);

    EXPECT_EQ(broker.giveProd(20), -1); // Недостаточно товара
}


TEST(OrderBookTest, AddOrderAndMakeDeals) {
    OrderBook book;

    book.addOrder(5, 100.0, OrderType::BUY, 1);
    book.addOrder(3, 90.0, OrderType::SELL, 2);

    auto deals = book.MakeDeals();

    EXPECT_EQ(deals.size(), 1);
    EXPECT_EQ(deals[0].Amount, 3);
    EXPECT_DOUBLE_EQ(deals[0].Price, 90.0);
    EXPECT_EQ(deals[0].BuyId, 1);
    EXPECT_EQ(deals[0].SellId, 2);
}

TEST(OrderBookTest, GetSpread) {
    OrderBook book;

    book.addOrder(5, 100.0, OrderType::BUY, 1);
    book.addOrder(3, 105.0, OrderType::SELL, 2);

    auto [bid, ask] = book.getSpread();
    EXPECT_DOUBLE_EQ(bid, 100.0);
    EXPECT_DOUBLE_EQ(ask, 105.0);
}


TEST(ExchangeTest, StartStopAndGetCurrentPrice) {
    Exchange exchange("TestProduct");

    exchange.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    exchange.stop();

    EXPECT_EQ(exchange.getCurrentPrice(), 0.0); 
}

TEST(ExchangeTest, RegisterAndUnregisterBroker) {
    Exchange exchange("TestProduct");
    auto broker = std::make_shared<Broker>(1000.0, 10, std::make_unique<BigShotStrategy>(0.1));

    exchange.registerBroker(broker);
    EXPECT_EQ(exchange.getBrokerCount(), 1);

    exchange.unregisterBroker(broker->getId());
    EXPECT_EQ(exchange.getBrokerCount(), 0);
}

TEST(ExchangeTest, AddOrderAndMatching) {
    Exchange exchange("TestProduct");
    exchange.start();

    auto broker1 = std::make_shared<Broker>(1000.0, 0, std::make_unique<BigShotStrategy>(0.1));
    auto broker2 = std::make_shared<Broker>(0.0, 10, std::make_unique<BigShotStrategy>(0.1));

    exchange.registerBroker(broker1);
    exchange.registerBroker(broker2);

  
    bool sellAdded = exchange.addOrder(5, 100.0, OrderType::SELL, broker2->getId());
    EXPECT_TRUE(sellAdded);


    bool buyAdded = exchange.addOrder(5, 100.0, OrderType::BUY, broker1->getId());
    EXPECT_TRUE(buyAdded);


    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto recentDeals = exchange.getRecentDeals(1);
    EXPECT_EQ(recentDeals.size(), 1);
    if (!recentDeals.empty()) {
        EXPECT_EQ(recentDeals[0].Amount, 5);
        EXPECT_EQ(recentDeals[0].BuyId, broker1->getId());
        EXPECT_EQ(recentDeals[0].SellId, broker2->getId());
    }

 
    auto status1 = broker1->getStatus();
    auto status2 = broker2->getStatus();
    
    EXPECT_EQ(status1.second, 5);
    
    EXPECT_GT(status2.first, 0.0);

    exchange.stop();
}


TEST(ExchangeIntegrationTest, SimulateBrokersLogic) {
    auto exchange = std::make_shared<Exchange>("IntegrationTestProduct");
    exchange->start();

    std::vector<std::shared_ptr<Broker>> brokerList;

   
    for (int i = 0; i < 1; ++i) { 
        auto bigShot = std::make_shared<Broker>(1000.0, 10, std::make_unique<BigShotStrategy>(0.1));
        exchange->registerBroker(bigShot);
        brokerList.push_back(bigShot);
        bigShot->startTrading(exchange);
    }

    for (int i = 0; i < 1; ++i) {
        auto gambler = std::make_shared<Broker>(500.0, 5, std::make_unique<GamblerStrategy>());
        exchange->registerBroker(gambler);
        brokerList.push_back(gambler);
        gambler->startTrading(exchange);
    }

    for (int i = 0; i < 1; ++i) {
        auto analyst = std::make_shared<Broker>(800.0, 8, std::make_unique<AnalystStrategy>());
        exchange->registerBroker(analyst);
        brokerList.push_back(analyst);
        analyst->startTrading(exchange);
    }

    
    std::this_thread::sleep_for(std::chrono::seconds(5));

    
    for (auto& broker : brokerList) {
        broker->stopTrading();
    }

   
    exchange->stop();

    
    EXPECT_EQ(exchange->getBrokerCount(), brokerList.size()); 
    
    for (const auto& broker : brokerList) {
        auto status = broker->getStatus();
       
        EXPECT_GE(status.first, 0.0); 
        EXPECT_GE(status.second, 0);  
    }

    std::cout << "Total Fees Collected: " << exchange->getTotalFees() << std::endl;
}