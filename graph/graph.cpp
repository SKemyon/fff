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

std::size_t EdgeHash::operator()(const edge& v) const {
    std::vector<std::string> ver = { v.getSource().takeName(), v.getDestination().takeName()};
    std::sort(ver.begin(), ver.end());
    size_t hashValue = std::hash<std::string>()(ver[0]) ^ (std::hash<std::string>()(ver[1]) << 1);
    return hashValue;
}



readFromUnweightedFile::readFromUnweightedFile(const std::string& filename) {
    Graph gr;
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("error: dont open " + filename);
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
               throw std::runtime_error("warning: invalid format: " + filename);
            }
        }
        Grph = gr;
        file.close();
    }

Graph readFromUnweightedFile::getGrph() const{
    return Grph;
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


Graph::Graph() {}
void Graph::addVertex(const vertex& newVer){
        if (graph.find(newVer) != graph.end()) {           
            return;
        }     
        graph[newVer];
    }

//void Graph::addEdge(const edge& Edge) {
//        addVertex(Edge.getSource());
//        addVertex(Edge.getDestination());     //find
//        if (std::find(graph[Edge.getSource()].begin(), graph[Edge.getSource()].end(), Edge) == graph[Edge.getSource()].end()) {
//            graph[Edge.getSource()].push_back(Edge);
//            return;
//        }
//        throw std::runtime_error("Warning: two same edges");
//    }


void Graph::addEdge(const edge& Edge) {
    addVertex(Edge.getSource());
    addVertex(Edge.getDestination());

    auto res = graph[Edge.getSource()].insert(Edge);
    if (!res.second) {
        throw std::runtime_error("warning: two same edges");
    }
}


std::unordered_map<vertex, std::unordered_set<edge, EdgeHash>, VertexHash> Graph::getAdjacencyList() const{
        return graph;
    }



PathFinder::PathFinder(const Graph& g, const vertex& A, const vertex& B) {}
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
            std::cout << std::endl;
        }
        else {
            std::cout << "No path" << std::endl;
        }
    }





dfsPathFinder::dfsPathFinder(const Graph& g, const vertex& A, const vertex& B)
        : PathFinder(g, A, B) {
    try {
        path = ShortestPath(A.takeName(), B.takeName(), g);
        }
    catch (std::runtime_error& a) {
        std::cout<< a.what();
        throw;
    }  
    }
std::vector<vertex> dfsPathFinder::ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const{
        const auto& AdjacencyList = g.getAdjacencyList();
        if (AdjacencyList.find(start) == AdjacencyList.end() || AdjacencyList.find(goal) == AdjacencyList.end()) {
            throw std::runtime_error("error: start or goal node not exist");
        }

        std::stack<vertex> stack;
        std::unordered_set<vertex, VertexHash> visited;
        std::unordered_map<vertex, vertex, VertexHash> preds;

        stack.push(start);

        while (!stack.empty()) {
            vertex node = stack.top();
            stack.pop();

            if (visited.contains(node)) {
                continue;
            }
            visited.insert(node);

            if (node == goal) {
                std::vector<vertex> path;
                for (vertex v = goal; v != start; v = preds[v]) {
                    path.push_back(v);
                }
                path.push_back(start);
                std::reverse(path.begin(), path.end()); 
                return path;
            }

            for (const edge& neighbor : AdjacencyList.at(node)) {
                vertex destination = neighbor.getDestination();
                if (visited.find(destination) == visited.end()) {
                    stack.push(destination);
                    preds[destination] = node; 
                }
            }
        }
        throw std::runtime_error("error: path not exist");

}



bfsPathFinder::bfsPathFinder(const Graph& g, const vertex& A, const vertex& B)
        : PathFinder(g, A, B) {
    try {
        path = ShortestPath(A.takeName(), B.takeName(), g);
    }
    catch (std::runtime_error& a) {
        std::cout << a.what();
        throw;
    }
    }
std::vector<vertex> bfsPathFinder::ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const{
        const auto& AdjacencyList = g.getAdjacencyList();
        if (AdjacencyList.find(start) == AdjacencyList.end() || AdjacencyList.find(goal) == AdjacencyList.end()) {
            throw std::runtime_error("error: start or goal node not exist");
        }
        std::queue<vertex> que;
        std::unordered_set<vertex, VertexHash> visited;
        std::unordered_map<vertex, vertex, VertexHash> preds;

        que.push(start);

        while (!que.empty()) {
            vertex node = que.front();
            que.pop();

            if (visited.find(node) != visited.end()) {
                continue;
            }
            visited.insert(node);

            if (node == goal) {
                std::vector<vertex> path;
                for (vertex v = goal; v != start; v = preds[v]) {
                    path.push_back(v);
                }
                path.push_back(start);
                std::reverse(path.begin(), path.end()); 
                return path;
            }

            for (const edge& neighbor : AdjacencyList.at(node)) {
                vertex destination = neighbor.getDestination();
                if (visited.find(destination) == visited.end()) {
                    que.push(destination);
                    preds[destination] = node;
                    
                }
            }
        }
        throw std::runtime_error("error: path not exist");
    }






















        /*std::queue<std::vector<vertex>> q;
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
        }*/
        ////////////////////////////////
            //    std::stack<std::vector<vertex>> stack;
            //    stack.push({ start });
            //    std::unordered_set<vertex, VertexHash> visited;
            //    //сохранять вершину плюс вершину из которой мы попали в вершину
            //    while (!stack.empty()) {
            //        std::vector<vertex> path = stack.top();
            //        stack.pop();
            //        vertex node = path.back();

            //        if (visited.contains(node)) {
            //            continue;
            //        }
            //        visited.insert(node);

            //        if (node == goal) {
            //            return path; 
            //        }

            //        for (const edge& neighbor : AdjacencyList.at(node)) {
            //            if (visited.find(neighbor.getDestination()) == visited.end()) {
            //                std::vector<vertex> new_path = path;
            //                new_path.push_back(neighbor.getDestination());
            //                stack.push(new_path);
            //            }
            //        }
            //    }
            //    throw std::runtime_error("Error: path does not exist in the graph.");

            //}