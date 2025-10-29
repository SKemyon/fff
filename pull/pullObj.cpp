#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <concepts>



template<class T, size_t N>
class pullObj {

	struct pullNode {
		T* node;
		bool used = false;
		void* memory;

		pullNode() : memory(operator new(sizeof(T))), node(nullptr), used(false) {} 

        //для vector
        pullNode(pullNode&& other) noexcept{
            std::swap(node, other.node);
            std::swap(used, other.used);
            std::swap(memory, other.memory);
        }//std::swap
        

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


































