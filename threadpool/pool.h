#ifndef POOL_H
#define POOL_H


#include <vector>
#include <queue>
#include <thread>

class Pool {

private:
    bool is_alive;

public:
    Pool(int num_threads);

    int num_threads;

    std::vector<std::thread> pool;

    std::queue<void()> job_queue;

    void thread_loop();
    
    void pushJob(function<void()> job);


    ~Pool();
}

#endif
