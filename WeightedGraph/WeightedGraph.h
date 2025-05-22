#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <queue>
#include <stack>
#include <optional>

class vertex final{
public:
    vertex();
    vertex(const std::string& vername);
    bool operator == (const vertex& ver2) const;
    std::string takeName() const;
private:
    std::string verName;
};

struct VertexHash {
    std::size_t operator()(const vertex& v) const;
};


class edge final{
public:
    edge();
    edge(const vertex& u,const vertex& v, size_t wght);
    bool operator == (const edge& edge2) const;
    vertex getSource() const;
    vertex getDestination() const;
    size_t getWeight() const;
private:
    vertex source;
    vertex destination;
    size_t weight;
};


struct EdgeHash {
    std::size_t operator()(const edge& v) const;
};



class Graph final{
public:
    Graph();
    void addVertex(const vertex& newVer);

    void addEdge(const edge& Edge);

    std::unordered_map<vertex, std::unordered_set<edge, EdgeHash>, VertexHash> getAdjacencyList() const;
private:
    std::unordered_map<vertex, std::unordered_set<edge, EdgeHash>, VertexHash> graph;
};




class readFromWeightedFile final{
public:
    readFromWeightedFile() = delete;
    static Graph getGrph(const std::string& filename);
};




class PathFinder {
public:
    PathFinder(const Graph& g, const vertex& A, const vertex& B);
    std::vector<vertex> getPath() const;

    void printPath() const;
    virtual ~PathFinder() = default;
    int getPathWght() const{
        return pathWght;
    }
protected:
    virtual std::vector<vertex> ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const = 0;
    bool empty() const;
    std::vector<vertex> path;
    int pathWght;
};




class dijkstraPathFinder final : public PathFinder {
public:

    dijkstraPathFinder(const Graph& g, const vertex& A, const vertex& B);
    std::vector<vertex> ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const override;
    size_t WeightOfPath(const Graph& g) const;
private:
    std::vector<vertex> reconstructPath(const std::unordered_map<vertex, vertex, VertexHash>& previous, const vertex& start, const vertex& goal) const;
};