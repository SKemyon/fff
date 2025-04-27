#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <queue>
#include <stack>
#include <optional>

class vertex {
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


class edge {
public:
    edge();
    edge(const vertex& u,const vertex& v);
    bool operator == (const edge& edge2) const;
    vertex getSource() const;
    vertex getDestination() const;
private:
    vertex source;
    vertex destination;
};


class Graph {
public:
    Graph();
    void addVertex(const vertex& newVer);

    void addEdge(const edge& Edge);

    std::unordered_map<vertex, std::vector<edge>, VertexHash> getAdjacencyList() const;
private:
    std::unordered_map<vertex, std::vector<edge>, VertexHash> graph;
};


class readFromUnweightedFile {
public:
    readFromUnweightedFile(const std::string& filename, Graph& gr);
};


class PathFinder {
public:
    PathFinder(const Graph& g, const vertex& A, const vertex& B);
    std::vector<vertex> getPath() const;

    void printPath() const;
    virtual ~PathFinder() = default;

protected:
    virtual std::vector<vertex> ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const = 0;


    bool empty() const;

    std::vector<vertex> path;
};


class dfsPathFinder final : public PathFinder {
public:
    dfsPathFinder(const Graph& g, const vertex& A, const vertex& B);
private:
    std::vector<vertex> ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const override;
};


class bfsPathFinder final : public PathFinder {
public:
    bfsPathFinder(const Graph& g, const vertex& A, const vertex& B);

private:
    std::vector<vertex> ShortestPath(const vertex& start, const vertex& goal, const Graph& g) const override;
};
