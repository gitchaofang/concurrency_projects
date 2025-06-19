#include<iostream>
#include<memory>
#include<atomic>


using Node = Node<T>
template<typename T>
struct Node{
	T val;
	std::atomic<Node*> next;
	Node():val(0), next(nullptr){} 
	Node(const T& in_val): val(in_val),next(nullptr){}
};

template<typename T>
class MsQue{
	private:
		std::atomic<Node*> head_;
		std::atomic<Node*> tail_;
	public:
		void enqueue(const T& value){
			Node* new_node = new Node(value);
			Node* next = nullptr;
			while(true){
				Node* tail = tail_.load(std::memory_order_acquire);
                        	Node* next = tail -> next.load(std::memory_order_acquire);
				if(tail == tail_.load(std::memory_order_aquire)){
					if(next == nullptr){
						if(tail -> next.compare_exchange_weak(next, new_node, std::memory_order_release, 
						std::memory_order_relaxed)){
							break;
						}
					}else{
						tail_.compare_exchange_weak(tail, next, std::memory_order_release, std::memory_order_relaxed);
					}
				}
			
			}

			tail_.compare_exchange_weak(tail, new_node,std::memory_order_release, std::memory_order_relaxed);
		}

		bool enqueue(T& result){
			Node* head;
			while(true){
				head = head_.load(std::memory_order_acquire);
				Node* tail = tail_.load(std::memory_order_acquire);
				Node* next = head -> next;
				if(head == head_.load(std::memory_order_acquire)){
					if(head == tail){
						if(next == nullptr){
							rerurn false;
						}else{
							tail_.compare_exchange_weak(tail, next, std::memory_order_release, std::memory_order_relaxed);
						}
						
					}else{
						result = next -> val;
						head_.compare_exchange_weak(head, next, std::memory_order_release, std::memory_order_relaxed);
						break;
					
					}
				}	
			
			}
			delete head;
			return true;
		}
};


int main(){


	return 0;
}
