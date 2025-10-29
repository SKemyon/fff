#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <concepts>
#include <gtest/gtest.h>


template<class T, size_t N>
class pullObj {

	struct pullNode {
		T* node;
		bool used = false;
		void* memory;

		pullNode() : memory(operator new(sizeof(T))), node(nullptr), used(false) {} //нью

        //для std::vector.................................................................
        //pullNode(pullNode&& other) noexcept
        //    : node(other.node), used(other.used), memory(other.memory) {
        //    other.node = nullptr;
        //    other.used = false;
        //    other.memory = nullptr;
        //}//std::swap

        pullNode(pullNode&& other) noexcept{
            std::swap(node, other.node);
            std::swap(used, other.used);
            std::swap(memory, other.memory);
        }//std::swap

        /*pullNode& operator=(pullNode&& other) noexcept {
            if (this != &other) {
                if (node) {
                    node->~T();
                }
                if (memory) {
                    delete(memory);
                }

                
                node = other.node;
                used = other.used;
                memory = other.memory;

                
                other.node = nullptr;
                other.used = false;
                other.memory = nullptr;
            }
            return *this;
        }*/

        pullNode& operator=(pullNode&& other) noexcept {
            if (this != &other) {
                if (node) {
                    node->~T();
                }
                if (memory) {
                    delete(memory);  
                }

                std::swap(node, other.node);
                std::swap(used, other.used);
                std::swap(memory, other.memory);
            }
            return *this;
        }
        //..................................................................

		template<typename... Args>
		T* push(Args&&... args) {	
			node = new(memory) T(std::forward<Args>(args)...);
            used = true;
			return node;
		}

		void pop() {
			if (used) {
				node->~T();
                used = false;
			}
		}

		~pullNode() {
			pop();
			delete(memory);
		}

        pullNode(const pullNode&) = delete;
        pullNode& operator=(const pullNode&) = delete;
        //pullNode(pullNode&&) = delete;
        //pullNode& operator=(pullNode&&) = delete;
	};

	std::vector<pullNode> objs;
  
public:
	pullObj() {
		objs.reserve(N); 
		for (size_t i = 0; i < N; ++i) {
			objs.emplace_back();
		}
	}

private:
	int findFree() {
		for (size_t i = 0; i < N; ++i) {
			if (!objs[i].used) {
				return i;
			}
		}
        throw std::runtime_error("No free obj");
	}

public:
	/*template<typename... Args>
	T* alloc(Args&&... args) {
		int pos = findFree();
		if (pos != -1) {
			return objs[pos].push(std::forward<Args>(args)...);  
		}
		throw std::runtime_error("No free obj");  
	}*/

    template<typename... Args>
    T* alloc(Args&&... args) {
        int pos = findFree();
        return objs[pos].push(std::forward<Args>(args)...);
    }


	int isInPull(T* obj) {  
		for (int i = 0; i < static_cast<int>(N); ++i) {
			if (obj == objs[i].node) {  
				return i;
			}
		}
		return -1;  
	}

	void free(T* obj) {  
		int pos = isInPull(obj);
		if (pos != -1) {
			objs[pos].pop();
		}
		
	}

	size_t freeObjNum() {
		size_t num = 0;
		for (size_t i = 0; i < N; ++i) {  
			if (!objs[i].used) {
				num += 1;
			}
		}
		return num;
	}
	
	/*~pullObj() {
		for (size_t i = 0; i < N; ++i) {
			objs.~pullNode();
		}
	}*/
    ~pullObj() = default;

    pullObj(const pullObj&) = delete;
    pullObj& operator=(const pullObj&) = delete;
    pullObj(pullObj&&) = delete;
    pullObj& operator=(pullObj&&) = delete;
};











//TESTS

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




















