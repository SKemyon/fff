#include <gtest/gtest.h>
#include <vector>
#include <list>
#include <forward_list>
#include <algorithm>
#include "filter.cpp"


class TestClass {
public:
    int value;
    std::string name;

    TestClass() : value(0), name("deflt") {}
    TestClass(int v, const std::string& n) : value(v), name(n) {}
};


class PullObjTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};


TEST_F(PullObjTest, ConstructorDestructor) {
    {
        pullObj<TestClass, 10> pool;
        EXPECT_EQ(pool.freeObjNum(), 10);
    }
}


TEST_F(PullObjTest, SingleAllocation) {
    pullObj<TestClass, 5> pool;

    TestClass* obj = pool.alloc(42, "test");
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->value, 42);
    EXPECT_EQ(obj->name, "test");
    EXPECT_EQ(pool.freeObjNum(), 4);

    pool.free(obj);
}


TEST_F(PullObjTest, MultipleAllocations) {
    pullObj<TestClass, 3> pool;

    TestClass* obj1 = pool.alloc(1, "fst");
    TestClass* obj2 = pool.alloc(2, "snd");

    EXPECT_NE(obj1, nullptr);
    EXPECT_NE(obj2, nullptr);
    EXPECT_NE(obj1, obj2);
    EXPECT_EQ(pool.freeObjNum(), 1);

    pool.free(obj1);
    pool.free(obj2);
}


TEST_F(PullObjTest, FreeAndReuse) {
    pullObj<TestClass, 2> pool;

    TestClass* obj1 = pool.alloc(1, "fst");
    EXPECT_EQ(pool.freeObjNum(), 1);

    pool.free(obj1);
    EXPECT_EQ(pool.freeObjNum(), 2);

    TestClass* obj2 = pool.alloc(3, "reuse");
    EXPECT_EQ(pool.freeObjNum(), 1);
    EXPECT_EQ(obj2->value, 3);
    EXPECT_EQ(obj2->name, "reuse");

    pool.free(obj2);
}


TEST_F(PullObjTest, PoolExhaustion) {
    pullObj<TestClass, 2> pool;

    TestClass* obj1 = pool.alloc(1, "fst");
    TestClass* obj2 = pool.alloc(2, "snd");

    EXPECT_EQ(pool.freeObjNum(), 0);
    EXPECT_THROW(pool.alloc(3, "thd"), std::runtime_error);

    pool.free(obj1);
    pool.free(obj2);
}


TEST_F(PullObjTest, DifferentTypes) {
    pullObj<int, 5> intPool;

    int* intObj = intPool.alloc(42);
    EXPECT_EQ(*intObj, 42);
    intPool.free(intObj);

    pullObj<double, 3> doublePool;
    double* doubleObj = doublePool.alloc(3.14);
    EXPECT_DOUBLE_EQ(*doubleObj, 3.14);
    doublePool.free(doubleObj);
}


TEST_F(PullObjTest, MemoryReuse) {
    pullObj<TestClass, 10> pool;


    for (int i = 0; i < 100; ++i) {
        TestClass* obj = pool.alloc(i, "test");
        pool.free(obj);
    }

    EXPECT_EQ(pool.freeObjNum(), 10);
}


TEST_F(PullObjTest, ExceptionSafety) {
    pullObj<TestClass, 2> pool;

    TestClass* obj1 = pool.alloc(1, "norm");
    EXPECT_NE(obj1, nullptr);


    TestClass* obj2 = pool.alloc(2, "snd");

    EXPECT_THROW(pool.alloc(3, "thd"), std::runtime_error);

    pool.free(obj1);
    pool.free(obj2);
}


TEST_F(PullObjTest, EdgeCases) {
    pullObj<TestClass, 1> pool;

    TestClass* obj = pool.alloc(1, "sing");
    EXPECT_EQ(pool.freeObjNum(), 0);

    pool.free(obj);
    EXPECT_EQ(pool.freeObjNum(), 1);
}


TEST_F(PullObjTest, FreeNullptr) {
    pullObj<TestClass, 5> pool;


    pool.free(nullptr);
    EXPECT_EQ(pool.freeObjNum(), 5);
}


TEST_F(PullObjTest, DoubleFree) {
    pullObj<TestClass, 5> pool;

    TestClass* obj = pool.alloc(1, "test");
    pool.free(obj);


    pool.free(obj);
    EXPECT_EQ(pool.freeObjNum(), 5);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}