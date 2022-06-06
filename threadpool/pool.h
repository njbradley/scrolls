#ifndef POOL_H
#define POOL_H


#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

class Pool {

private:
    bool is_alive;

public:
    Pool(int num_threads);

    int num_threads;

    std::vector<std::thread> pool;

    std::queue< std::function<void()> > job_queue;

    void thread_loop();
    
    void pushJob(const std::function<void()>& job);

    std::mutex queue_mutex;

    std::condition_variable v;


    ~Pool();
};

#endif
