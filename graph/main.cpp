#include "graph.h"
int main() {
    Graph Gr1;
    std::string sss1 = "graph.txt";
    readFromUnweightedFile(sss1, Gr1);
    for (const auto& pair : Gr1.getAdjacencyList()) {
        std::cout << "Key: " << pair.first.takeName() << std::endl;

        for (size_t i = 0; i < pair.second.size(); ++i) {
            std::cout << " source: " << pair.second[i].getSource().takeName();
            std::cout << " destination: " << pair.second[i].getDestination().takeName() << std::endl;
        }
    }
    vertex C("A");
    vertex D("F");
    bfsPathFinder bfsPathFinder(Gr1, C, D);
    bfsPathFinder.printPath();
    return 0;
}