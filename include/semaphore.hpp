#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore
{
    public:
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
        
    private:
        int count = 0;
        std::mutex mtx;
        std::condition_variable cv;
};

#endif
