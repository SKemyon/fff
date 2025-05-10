#include "WeightedGraph.h"

vertex::vertex() {}
vertex::vertex(const std::string& vername) : verName(vername) {}

bool vertex:: operator == (const vertex& ver2) const {
    return verName == ver2.verName;
}


std::string vertex::takeName() const {
    return verName;
}


std::size_t VertexHash::operator()(const vertex& v) const {
    return std::hash<std::string>()(v.takeName());
}


std::size_t EdgeHash::operator()(const edge& v) const {
    std::vector<std::string> ver = { v.getSource().takeName(), v.getDestination().takeName() };
    std::sort(ver.begin(), ver.end());
    size_t hashValue = std::hash<std::string>()(ver[0]) ^ (std::hash<std::string>()(ver[1]) << 1);
    return hashValue;
}



readFromWeightedFile::readFromWeightedFile(const std::string& filename) {
    Graph gr;
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("error: dont open " + filename);
        }
        std::string line;
        while (getline(file, line)) {
            std::stringstream ss(line);
            std::string u, v, weightStr;

            if (getline(ss, u, '-') && getline(ss, v, ':') && getline(ss, weightStr)) {
                if (!u.empty() && !v.empty() && !weightStr.empty()) {
                    vertex ver1(u);
                    vertex ver2(v);
                    size_t weight = std::stod(weightStr);
                    edge uv(ver1, ver2, weight);
                    edge vu(ver2, ver1, weight);
                    try {
                        gr.addEdge(uv);
                        gr.addEdge(vu);
                    }
                    catch (std::runtime_error& a) {
                        std::cout << a.what();
                    }
                }
            }
            else {
                throw std::runtime_error("warning: invalid format: " + filename);
            }
        }
        Grph = gr;
        file.close();
    }

Graph readFromWeightedFile::getGrph() const {
    return Grph;
}



edge::edge() {}
edge::edge(const vertex& u,const vertex& v,const size_t wght) : source(u), destination(v), weight(wght) {}
bool edge::operator == (const edge& edge2) const {
    return ((source.takeName() == edge2.source.takeName()) && (destination.takeName() == edge2.destination.takeName())) ||
        ((source.takeName() == edge2.source.takeName()) && (destination.takeName() == edge2.destination.takeName()));
}

vertex edge::getSource() const {
    return source;
}

vertex edge::getDestination() const {
    return destination;
}

size_t edge::getWeight() const {
    return weight;
}



Graph::Graph() {}
void Graph::addVertex(const vertex& newVer) {
    if (graph.find(newVer) != graph.end()) {
        return;
    }
    graph[newVer];
}

void Graph::addEdge(const edge& Edge) {
    addVertex(Edge.getSource());
    addVertex(Edge.getDestination());
    auto res = graph[Edge.getSource()].insert(Edge);
    if (!res.second) {
        throw std::runtime_error("warning: two same edges");
    }
}


std::unordered_map<vertex, std::unordered_set<edge, EdgeHash>, VertexHash> Graph::getAdjacencyList() const {
    return graph;
}



PathFinder::PathFinder(const Graph& g, const vertex& A, const vertex& B): pathWght(0) {}
std::vector<vertex> PathFinder::getPath() const {
    return path;
}

bool PathFinder::empty() const {
    return path.empty();
}

void PathFinder::printPath() const {
    if (!empty()) {
        std::cout << "Shortest path: ";
        for (const auto& node : path) {
            std::cout << node.takeName() << " ";
        }
        std::cout << pathWght << std::endl;
    }
    else {
        std::cout << "No path found." << std::endl;
    }
}



bool operator>(const std::pair<int, vertex>& lhs, const std::pair<int, vertex>& rhs) {
    if (lhs.first != rhs.first) {
        return lhs.first > rhs.first;
    }
}

dijkstraPathFinder::dijkstraPathFinder(const Graph& g, const vertex& A, const vertex& B): PathFinder(g, A, B) {
    try {
        path = ShortestPath(A.takeName(), B.takeName(), g);
        pathWght = WeightOfPath(g);
    }
    catch (std::runtime_error& a) {
        std::cout << a.what();
        throw;
    }
    }


size_t dijkstraPathFinder::WeightOfPath(const Graph& g) const {
    int totalWght = 0;
    const auto& AdjacencyList = g.getAdjacencyList();
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        vertex u = path[i];
        vertex v = path[i+1];
        bool fl = false;
        for (const edge& edge : AdjacencyList.at(u)) {
            if (edge.getDestination() == v) {
                totalWght += edge.getWeight();
                fl = true;
                break;
            }
        }
        if (!fl) {
            throw std::runtime_error("edge not in path");
        }
    }
    return totalWght;
}

std::vector<vertex> dijkstraPathFinder::ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const{
        const auto& AdjacencyList = g.getAdjacencyList();
        if (AdjacencyList.find(start) == AdjacencyList.end() || AdjacencyList.find(goal) == AdjacencyList.end()) {
            throw std::runtime_error("error: start or goal node not exist");
        }
        

        std::priority_queue<std::pair<int, vertex>, std::vector<std::pair<int, vertex>>, std::greater<std::pair<int, vertex>>> pq;
        pq.push({ 0, start });

        std::unordered_map<vertex, int, VertexHash> dist;
        std::unordered_map<vertex, vertex, VertexHash> prev;
        for (const auto& edge : AdjacencyList) {
            dist[edge.first] = std::numeric_limits<int>::max();
        }
        dist[start] = 0;

        while (!pq.empty()) {
            int currentDist = pq.top().first;
            vertex currentVer = pq.top().second;
            pq.pop();

            if (currentVer == goal) {
                return reconstructPath(prev, start, goal);
            }

            if (currentDist > dist[currentVer]) {
                continue;
            }
            


            for (const edge& neighbor : AdjacencyList.at(currentVer)) {

                int newDist = currentDist + neighbor.getWeight();

                if (newDist < dist[neighbor.getDestination()]) {
                    dist[neighbor.getDestination()] = newDist;
                    prev[neighbor.getDestination()] = currentVer;
                    pq.push({ newDist, neighbor.getDestination()});
                }
            }
        }
        throw std::runtime_error("error: no path");
    }


std::vector<vertex> dijkstraPathFinder::reconstructPath(const std::unordered_map<vertex, vertex, VertexHash>& prev, const vertex& start, const vertex& goal) const {
        std::vector<vertex> path;
        for (vertex at = goal; at != start; at = prev.at(at)) {
            path.push_back(at);
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }






