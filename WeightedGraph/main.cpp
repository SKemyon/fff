#include "WeightedGraph.h"

int mmain() {
    Graph Gr;
    std::string sss = "weightedGraph.txt"; 
    Gr = readFromWeightedFile(sss).getGrph();
    /*for (const auto& pair : Gr.getAdjacencyList()) {
        std::cout << "Key: " << pair.first.takeName() << std::endl;

        for (size_t i = 0; i < pair.second.size(); ++i) {
            std::cout << " source: " << pair.second[i].getSource().takeName();
            std::cout << " destination: " << pair.second[i].getDestination().takeName();
            std::cout << " weight: " << pair.second[i].getWeight() << std::endl;
        }
    }*/
    vertex A("A");
    vertex B("L");
    dijkstraPathFinder pathFinder(Gr, A, B);
    std::cout << pathFinder.getPathWght();
    pathFinder.printPath();


    return 0;
}