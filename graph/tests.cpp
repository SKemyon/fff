#include "graph.h"
#include <gtest/gtest.h>
#include <stdexcept>

class GraphTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }

    void createGraphFile(const std::string& filename, const std::vector<std::string>& edges) {
        std::ofstream file(filename);
        for (const auto& edge : edges) {
            file << edge << "\n";
        }
    }
};

TEST_F(GraphTest, FileNotOpen) {
    EXPECT_THROW(readFromUnweightedFile("file.txt"), std::runtime_error);
}

TEST_F(GraphTest, NoStartOrGoalVerInGraph) {
    createGraphFile("test.txt", { "A-B", "B-C" });
    readFromUnweightedFile File("test.txt");
    Graph g = File.getGrph();

    vertex start("A");
    vertex goal("D");

    EXPECT_THROW(dfsPathFinder f(g, start, goal), std::runtime_error);
    EXPECT_THROW(bfsPathFinder f(g, start, goal), std::runtime_error);
}

TEST_F(GraphTest, NoPath) {
    createGraphFile("noPath.txt", { "A-B", "C-D" , "V-M"});
    readFromUnweightedFile File("noPath.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex C("C");
    vertex V("V");


    EXPECT_THROW(dfsPathFinder finderNoPath(Gr, A, C), std::runtime_error);
    EXPECT_THROW(dfsPathFinder finderNoPath(Gr, V, A), std::runtime_error);

    EXPECT_THROW(bfsPathFinder finderNoPath(Gr, A, C), std::runtime_error);
    EXPECT_THROW(bfsPathFinder finderNoPath(Gr, V, A), std::runtime_error);
}

TEST_F(GraphTest, Path) {
    createGraphFile("test.txt", { "A-B", "B-C", "C-D" });
    readFromUnweightedFile File("test.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex B("D");

    dfsPathFinder finder1(Gr, A, B);
    bfsPathFinder finder2(Gr, A, B);

    vertex start1 = finder1.getPath()[0];
    vertex goal1 = finder1.getPath().back();

    vertex start2 = finder2.getPath()[0];
    vertex goal2 = finder2.getPath().back();

    EXPECT_EQ(start1, A);
    EXPECT_EQ(goal1, B);
    EXPECT_EQ(start2, A);
    EXPECT_EQ(goal2, B);
}

TEST_F(GraphTest, CircPath) {
    createGraphFile("test.txt", { "A-B", "B-C", "C-A" });
    readFromUnweightedFile File("test.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex C("C");

    dfsPathFinder finder1(Gr, A, C);
    bfsPathFinder finder2(Gr, A, C);

    vertex start1 = finder1.getPath()[0];
    vertex goal1 = finder1.getPath().back();

    vertex start2 = finder2.getPath()[0];
    vertex goal2 = finder2.getPath().back();

    EXPECT_EQ(start1, A);
    EXPECT_EQ(goal1, C);
    EXPECT_EQ(start2, A);
    EXPECT_EQ(goal2, C);
}

TEST_F(GraphTest, EmptyGraph) {
    createGraphFile("test.txt", {});
    readFromUnweightedFile File("test.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex B("B");

    EXPECT_THROW(dfsPathFinder f(Gr, A, B), std::runtime_error);
}

TEST_F(GraphTest, SingleEdge) {
    createGraphFile("test.txt", { "A-B" });
    readFromUnweightedFile File("test.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex B("B");

    dfsPathFinder finder1(Gr, A, B);
    bfsPathFinder finder2(Gr, A, B);

    vertex start1 = finder1.getPath()[0];
    vertex goal1 = finder1.getPath().back();

    vertex start2 = finder2.getPath()[0];
    vertex goal2 = finder2.getPath().back();

    EXPECT_EQ(start1, A);
    EXPECT_EQ(goal1, B);
    EXPECT_EQ(start2, A);
    EXPECT_EQ(goal2, B);
}



