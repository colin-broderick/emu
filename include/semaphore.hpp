#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <iostream>

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

        static inline void clock_function(Semaphore* sem, unsigned int cycles)
        {
            while (cycles--)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
                sem->notify();
            }
        }
        
        int count;
    private:
        std::mutex mtx;
        std::condition_variable cv;
};

#endif
