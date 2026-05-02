#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

class TaskQueue{
    std::queue<int> q;
    std::condition_variable cond_var;
    std::mutex mutex;
    bool full_stop = false;

    public:
        TaskQueue() {}
        void add(int task);
        bool extract(int& task);
        void stop_all();


};
void TaskQueue::add(int task){
    std::unique_lock<std::mutex> lock(mutex);

    q.push(task);

    cond_var.notify_one();

}

bool TaskQueue::extract(int& task){
    std::unique_lock<std::mutex> lock(mutex);
    cond_var.wait(lock, [this](){return full_stop || !q.empty();});

    if (full_stop && q.empty())
    return false;

    task = q.front();
    q.pop();

    return true;
}

void TaskQueue::stop_all(){
    std::unique_lock<std::mutex> lock(mutex);
    full_stop = true;
    cond_var.notify_all();
}

std::mutex m;
void thread_func(int id, TaskQueue& q){
    int task;
    while(q.extract(task)){
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lock(m);
        std::cout << "[Worker-" << id << "] обработал задачу " << task << std::endl;
    }
}

int main(){

    int N = 10;
    TaskQueue q;
    for (int i = 1; i <= 20; i++)
       q.add(i);


    std::vector<std::thread> threads;
    
    for (int i = 0; i < N; ++i) {
        std::thread thread(thread_func, i + 1, std::ref(q));
        threads.push_back(std::move(thread));
    }

    q.stop_all();
    for (auto i = threads.begin(); i != threads.end(); i++){
        (*i).join();
    }
    

    return 0;
}

