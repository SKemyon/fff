#include "WeightedGraph.h"
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
    EXPECT_THROW(readFromWeightedFile("file.txt"), std::runtime_error);
}

TEST_F(GraphTest, NoStartOrGoalVerInGraph) {
    createGraphFile("test.txt", { "A-B:10", "B-C:5" });
    readFromWeightedFile File("test.txt");
    Graph g = File.getGrph();

    vertex start("A");
    vertex goal("D");
    EXPECT_THROW(dijkstraPathFinder finderNoPath(g, start, goal), std::runtime_error);
}

TEST_F(GraphTest, NoPath) {
    createGraphFile("noPath.txt", { "A-B:4", "C-D:22", "V-M:3" });
    readFromWeightedFile File("noPath.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex C("C");
    vertex V("V");
    EXPECT_THROW(dijkstraPathFinder finderNoPath(Gr, A, C), std::runtime_error);
    EXPECT_THROW(dijkstraPathFinder finderNoPath(Gr, V, A), std::runtime_error);
}

TEST_F(GraphTest, Path) {
    createGraphFile("test.txt", { "A-B:33", "B-C:32", "C-D:11" });
    readFromWeightedFile File("test.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex B("D");

    dijkstraPathFinder finder1(Gr, A, B);

    vertex start1 = finder1.getPath()[0];
    vertex goal1 = finder1.getPath().back();
    EXPECT_EQ(start1, A);
    EXPECT_EQ(goal1, B);
    EXPECT_EQ(finder1.getPathWght(), 33 + 32 + 11);
}

TEST_F(GraphTest, CircPath) {
    createGraphFile("test.txt", { "A-B:5", "B-C:333", "C-A:2" });
    readFromWeightedFile File("test.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex C("C");

    dijkstraPathFinder finder1(Gr, A, C);

    vertex start1 = finder1.getPath()[0];
    vertex goal1 = finder1.getPath().back();

    EXPECT_EQ(start1, A);
    EXPECT_EQ(goal1, C);
    EXPECT_EQ(finder1.getPathWght(), 2);
}

TEST_F(GraphTest, EmptyGraph) {
    createGraphFile("test.txt", {});
    readFromWeightedFile File("test.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex B("B");

    EXPECT_THROW(dijkstraPathFinder finderNoPath(Gr, A, B), std::runtime_error);
}

TEST_F(GraphTest, SingleEdge) {
    createGraphFile("test.txt", { "A-B:2" });
    readFromWeightedFile File("test.txt");
    Graph Gr = File.getGrph();

    vertex A("A");
    vertex B("B");

    dijkstraPathFinder finder1(Gr, A, B);

    vertex start1 = finder1.getPath()[0];
    vertex goal1 = finder1.getPath().back();

    EXPECT_EQ(start1, A);
    EXPECT_EQ(goal1, B);


    EXPECT_EQ(finder1.getPathWght(), 2);
}