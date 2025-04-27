#include "graph.h"

vertex::vertex(){}
vertex::vertex(const std::string& vername) : verName(vername) {}

bool vertex:: operator == (const vertex& ver2) const{
        return verName == ver2.verName;
    }


std::string vertex::takeName() const{
        return verName;
    }


std::size_t VertexHash::operator()(const vertex& v) const {
       return std::hash<std::string>()(v.takeName());
    }



readFromUnweightedFile::readFromUnweightedFile(const std::string& filename, Graph& gr) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Error: could not open " + filename);
        }
        std::string line;
        while (getline(file, line)) {
            std::stringstream ss(line);
            std::string u, v;

            if (getline(ss, u, '-') && getline(ss, v)) {
                if (!u.empty() && !v.empty()) {
                    vertex ver1(u);
                    vertex ver2(v);
                    edge uv(ver1, ver2);
                    edge vu(ver2, ver1);
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
               throw std::runtime_error("Warning: Invalid line format: " + filename);
            }
        }
        file.close();
    }





edge::edge() {}
edge::edge(const vertex& u,const vertex& v) : source(u), destination(v) {}
bool edge::operator == (const edge& edge2) const {
    return (source.takeName() == edge2.source.takeName()) && (destination.takeName() == edge2.destination.takeName()) ||
        (source.takeName() == edge2.source.takeName()) && (destination.takeName() == edge2.destination.takeName());
    }
vertex edge::getSource() const{
    return source;
}

vertex edge::getDestination() const{
    return destination;
}

namespace std {
    template <>
    struct hash<edge> {

        size_t operator()(const edge& key) const {
            return std::hash<std::string>()(key.getDestination().takeName());
        }
    };
}



Graph::Graph() {}
void Graph::addVertex(const vertex& newVer){
        if (graph.find(newVer) != graph.end()) {           
            return;
        }     
        graph[newVer];
    }

void Graph::addEdge(const edge& Edge) {
        addVertex(Edge.getSource());
        addVertex(Edge.getDestination());       
        if (std::find(graph[Edge.getSource()].begin(), graph[Edge.getSource()].end(), Edge) == graph[Edge.getSource()].end()) {
            graph[Edge.getSource()].push_back(Edge);
            return;
        }
        throw std::runtime_error("Warning: two same edges");
    }

std::unordered_map<vertex, std::vector<edge>, VertexHash> Graph::getAdjacencyList() const{
        return graph;
    }



PathFinder::PathFinder(const Graph& g, const vertex& A, const vertex& B) {
}
std::vector<vertex> PathFinder::getPath() const {
        return path;
    }

bool PathFinder::empty() const {
    if (path.empty()) {
        return  1;
    }
    return 0;
}

void PathFinder::printPath() const {
        if (!empty()) {
            std::cout << "Shortest path: ";
            for (const auto& node : path) {
                std::cout << node.takeName() << " ";
            }
            std::cout << std::endl;
        }
        else {
            std::cout << "No path found." << std::endl;
        }
    }





dfsPathFinder::dfsPathFinder(const Graph& g, const vertex& A, const vertex& B)
        : PathFinder(g, A, B) {
    try {
        path = ShortestPath(A.takeName(), B.takeName(), g);
        }
    catch (std::runtime_error& a) {
        std::cout<< a.what();
    }  
    }
std::vector<vertex> dfsPathFinder::ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const{
        const auto& AdjacencyList = g.getAdjacencyList();
        if (AdjacencyList.find(start) == AdjacencyList.end() || AdjacencyList.find(goal) == AdjacencyList.end()) {
            throw std::runtime_error("Error: Start or goal node does not exist in the graph.");
        }

        std::stack<std::vector<vertex>> stack;
        stack.push({ start });
        std::unordered_set<vertex, VertexHash> visited;

        while (!stack.empty()) {
            std::vector<vertex> path = stack.top();
            stack.pop();
            vertex node = path.back();

            if (visited.contains(node)) {
                continue;
            }
            visited.insert(node);

            if (node == goal) {
                return path; 
            }

            for (const edge& neighbor : AdjacencyList.at(node)) {
                if (visited.find(neighbor.getDestination()) == visited.end()) {
                    std::vector<vertex> new_path = path;
                    new_path.push_back(neighbor.getDestination());
                    stack.push(new_path);
                }
            }
        }
        throw std::runtime_error("Error: path does not exist in the graph.");

    }


bfsPathFinder::bfsPathFinder(const Graph& g, const vertex& A, const vertex& B)
        : PathFinder(g, A, B) {
    try {
        path = ShortestPath(A.takeName(), B.takeName(), g);
    }
    catch (std::runtime_error& a) {
        std::cout << a.what();
    }
    }

std::vector<vertex> bfsPathFinder::ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const{
        const auto& AdjacencyList = g.getAdjacencyList();
        if (AdjacencyList.find(start) == AdjacencyList.end() || AdjacencyList.find(goal) == AdjacencyList.end()) {
            throw std::runtime_error("Error: Start or goal node does not exist in the graph.");
        }
        std::queue<std::vector<vertex>> q;
        q.push({ start });
        std::unordered_set<vertex, VertexHash> visited;
        while (!q.empty()) {
            std::vector<vertex> path = q.front();
            q.pop();
            vertex node = path.back();

            if (visited.find(node) != visited.end()) {
                continue;
            }
            visited.insert(node);
            if (node == goal) {
                return path;
            }
            for (const edge& neighbor : AdjacencyList.at(node)) {
                std::vector<vertex> new_path = path;
                if (node.takeName() == neighbor.getDestination().takeName()) {
                    continue;
                }
                new_path.push_back(neighbor.getDestination());
                q.push(new_path);
            }
        }
        throw std::runtime_error("Error: path does not exist in the graph.");
    }










