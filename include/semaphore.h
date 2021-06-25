#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

class Semaphore
{
    public:
        Semaphore() : count(0) {}
        Semaphore(int count_) : count(count_) {}
        
        inline void notify()
        {
            std::unique_lock<std::mutex> lock(mtx);
            count++;
            cv.notify_one();
        }

        inline void wait()
        {
            std::unique_lock<std::mutex> lock(mtx);
            while (count == 0)
            {
                cv.wait(lock);
            }
            count--;
        }

        static void clock_function(Semaphore* sem)
        {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
                sem->notify();
            }
        }

    private:
        std::mutex mtx;
        std::condition_variable cv;
        int count;
};

#endif
