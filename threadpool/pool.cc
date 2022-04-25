#include "pool.h"
#include <mutex>
#include <condition_variable>

Pool::Pool(int num) {
    num_threads = num;
    job_queue = queue<void()>();
    is_alive = true;
    for (int i = 0; i < num; i++) {
        pool.push_back(std::thread(&Pool::job, this));
    }
}

void Pool::thread_loop() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            this.wait(lock, [this](){
                return !queue.empty() || is_alive;
            });
            Job = queue.front();
            queue.pop();
        }

        Job(); // function<void()> type
    }
}

void Pool::pushJob() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue.push(New_Job);
    }

    std::condition_variable::notify_one();
}