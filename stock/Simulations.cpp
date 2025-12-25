#include "Exchange.h"

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