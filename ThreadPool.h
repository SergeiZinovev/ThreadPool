#pragma once
#include <queue>
#include <future>
#include <vector>
#include <iostream>
#include <mutex>
#include <functional>
#include <thread>

using namespace std;

typedef function<void()> task_type;
typedef future<void> res_type;

template<class T>
class BlockedQueue {
public:
    void push(T& item) {
        lock_guard<mutex> l(m_locker);
        // thread-safe push
        m_task_queue.push(item);
        // make a notification so that the thread that called the pop wakes up and takes the item from the queue
        m_notifier.notify_one();
    }
    // blocking method of getting an item from the queue
    void pop(T& item) {
        unique_lock<mutex> l(m_locker);
        if (m_task_queue.empty())
            m_notifier.wait(l, [this] {return !m_task_queue.empty(); });
        item = m_task_queue.front();
        m_task_queue.pop();
    }
    // non-blocking method of getting an item from the queue
    // returns false if the queue is empty
    bool fast_pop(T& item) {
        lock_guard<mutex> l(m_locker);
        if (m_task_queue.empty())
            return false;
        // get an element
        item = m_task_queue.front();
        m_task_queue.pop();
        return true;
    }
private:
    mutex m_locker;
    // task queue
    queue<T> m_task_queue;
    condition_variable m_notifier;
};

// type pointer to a function that is a pattern for task functions
typedef void (*FuncType) (int*, int, int);
// thread pool
class OptimizedThreadPool {
public:
    OptimizedThreadPool();
    void start();
    void stop();
    // task forwarding
    void push_task(FuncType f, int* array, int left, int right);
    // input function for a thread
    void threadFunc(int qindex);
private:
    int m_thread_count;
    // thread counter
    vector<thread> m_threads;
    // task queues for threads
    vector<BlockedQueue<task_type>> m_thread_queues;
    // to distribute tasks evenly
    int m_index;
};

class RequestHandler {
public:
    RequestHandler() {
        m_tpool.start();
    }
    ~RequestHandler() {
        m_tpool.stop();
    }
    void pushRequest(FuncType f, int* array, int left, int right) {
        m_tpool.push_task(f, array, left,right);
    }
private:
    // thread pool
    OptimizedThreadPool m_tpool;
};
