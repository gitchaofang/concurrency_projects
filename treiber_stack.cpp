#include<iostream>
#include<atomic>
#include<thread>
#include<future>
#include<memory>
#include<utility>

template<typename T>
struct Node{
	Node():val(0){}
	Node(const T& input):val(input){}
	Node(Node&& node) noexcept{
		val = node.val;
		delete next;
		next = node.next;
	}

	Node (const Node& node){
		val = node.val;
		next = new Node(node.val);
	}

	Node& operator= (const Node& node){
        	val = node.val;
                next = new Node(node.val);
		return *this;	
        }
	T val;
	Node* next = nullptr;
};


template<typename T>
class TreiberStack{
	private:
		std::mutex lk;
		std::atomic<T*> head;
		std::vector<T> final_result;

	public:
		TreiberStack(): head(nullptr){}
		void push(T&& input){
			T* new_head = new T(input);
			T* old_head = nullptr;
			do{
				old_head = head.load(std::memory_order::relaxed);
				new_head -> next = old_head;
			}while(!head.compare_exchange_weak(old_head, new_head, std::memory_order::release, std::memory_order_relaxed));
		}
		
		bool pop(T& result){
			T* old_head = nullptr;
			do{
				old_head = head.load(std::memory_order::acquire);
				if(!old_head){
					return false;
				}
			
			}while(!head.compare_exchange_weak(old_head, old_head-> next, std::memory_order::release, std::memory_order::relaxed));
			
			result = *old_head;
			delete old_head;
			return true;
		} 
		
		
		 void push_result(T&& result_ptr){
                            std::lock_guard<std::mutex> lg(lk);
                            final_result.push_back(result_ptr);
                    }

		void show(){
			for_each(final_result.begin(), final_result.end(), [](const T res){
				std::cout << res.val << std::endl;
			});
		}

};

int main(){	
	TreiberStack<Node<int>> st;

        auto pushFunc = [&](Node<int>&& input){
		st.push(std::forward<Node<int>&&>(input));
	};		
	
	
	auto popFunc = [&](std::atomic<int>& cnt){
		Node<int> res;
		bool success  = st.pop(res);
		if(success){
			st.push_result(std::move(res));
			cnt.fetch_add(1);
		}
	};
	
	std::vector<std::future<void>> futs;
	futs.reserve(10);
	for(int i = 0; i < 10; ++i){
		Node nd(rand() % 100);
		std::future<void> fut = std::async(std::launch::async, pushFunc, std::move(nd));
		futs.push_back(std::move(fut));
	}
	for_each(futs.begin(), futs.end(), [](std::future<void>& fut){fut.get();});

	// pop
	std::atomic<int> cnt = 0;
	std::vector<std::future<void>> pop_vecs;
	while(cnt != 10){
		std::future<void> fut = std::async(std::launch::async, popFunc, std::ref(cnt));		
		pop_vecs.push_back(std::move(fut));
	}
	for_each(pop_vecs.begin(), pop_vecs.end(), [](std::future<void>& fut){fut.get();});
	st.show();
	std::cout << " cnt = " << cnt << std::endl;
	


	
	return 0;
}
