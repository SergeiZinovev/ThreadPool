#include "ThreadPool.h"

OptimizedThreadPool::OptimizedThreadPool() :
//number of threads that can be executed simultaneously
   m_thread_count(thread::hardware_concurrency() != 0 ? thread::hardware_concurrency() : 4)
    ,
    m_thread_queues(m_thread_count) {}

void OptimizedThreadPool::start() {
    for (int i = 0; i < m_thread_count; i++) {
        m_threads.emplace_back(&OptimizedThreadPool::threadFunc, this, i);
    }
}

void OptimizedThreadPool::stop() {
    for (int i = 0; i < m_thread_count; i++) {
        // put a pacifier task in each queue
        //   to end the thread
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads) {
        t.join();
    }

}

void OptimizedThreadPool::push_task(FuncType f, int* array, int left, int right) {

    // calculate the index of the queue where we will put the task
    int queue_to_push = m_index++ % m_thread_count;
    // form a functor
    task_type task = [=] {f(array, left, right);  };
    // queue up 
    m_thread_queues[queue_to_push].push(task);
}

void OptimizedThreadPool::threadFunc(int qindex) {
    while (true) {
        // task processing
        task_type task_to_do;
        bool res;
        int i = 0;
        for (; i < m_thread_count; i++) {
            // attempt to quickly pick up a task from any queue
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }

        if (!res) {
            // blocking method of getting an item from the queue
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do) {
            // to keep the thread from freezing
            // put the pacifier task back in
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (!task_to_do) {
            return;
        }
        // perform a task
        task_to_do();
    }
}
