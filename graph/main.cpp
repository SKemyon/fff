#include "graph.h"
int mymain() {
    std::string sss1 = "graph.txt";
    readFromUnweightedFile File(sss1);
    /*for (const auto& pair : Gr1.getAdjacencyList()) {
        std::cout << "Key: " << pair.first.takeName() << std::endl;

        for (size_t i = 0; i < pair.second.size(); ++i) {
            std::cout << " source: " << pair.second[i].getSource().takeName();
            std::cout << " destination: " << pair.second[i].getDestination().takeName() << std::endl;
        }
    }*/
    vertex C("A");
    vertex D("l");
    bfsPathFinder bfsPathFinder(File.getGrph(), C, D);
    bfsPathFinder.printPath();
    return 0;
}