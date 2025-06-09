#include<filesystem>
#include<iostream>
#include<thread>
#include<mutex>
#include<unordered_set>
#include<future>
#include<algorithm>
#include<numeric>
#include<chrono>
#include<string_view>
#include<condition_variable>
#include<stack>
#include<atomic>

namespace fs = std::filesystem;

class FileList{
	private:
		std::mutex lk;
		std::stack<std::string> list;
		std::condition_variable cv;
		std::atomic<bool> done;
	public:
		FileList(): done(false){}
		void pushFile(const std::string& file_path){
			std::vector<std::future<void>> futures;
			for(const auto& entry: fs::directory_iterator(file_path)){
				if(fs::is_directory(entry.path())){
					auto fut =  std::async(std::launch::async, &FileList::pushFile, this, entry.path().string());
					futures.push_back(std::move(fut));
				}else{
					std::lock_guard<std::mutex> lg(lk);
					std::string file_name = file_path + "/" + entry.path().string();
					list.push(std::move(file_name));
					cv.notify_one();
				}
			}
			for_each(futures.begin(), futures.end(), [&](std::future<void>& fut){fut.wait();});
		}

		void outPut(){
			while(!done){
				std::unique_lock<std::mutex> lg(lk);
				cv.wait(lg,[&](){return done || !list.empty();});
				while(!list.empty()){
					std::cout << list.top() << std::endl;
					list.pop();
				}
			}
		} 

		void endProduce(){
			done.store(true);
			cv.notify_one();
		}

};
	

int main(){
	// local root_path you want to start with
	std::string root_path =  "/Users/chaofang/Documents";
	FileList file_list;
	std::thread th_consume(&FileList::outPut, &file_list);
	std::future<void> fut = std::async(std::launch::async,&FileList::pushFile, &file_list, root_path);
	fut.wait();
	file_list.endProduce();
	th_consume.join();	
	return 0;
}
