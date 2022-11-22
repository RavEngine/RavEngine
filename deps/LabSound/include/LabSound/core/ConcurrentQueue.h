// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace lab
{

template <typename Data>
class ConcurrentQueue
{
    std::queue<Data> the_queue;
    mutable std::mutex the_mutex;
    std::condition_variable the_condition_variable;

public:
    void push(Data const & data)
    {
        {
            std::lock_guard<std::mutex> lock(the_mutex);
            the_queue.push(data);
        }
        the_condition_variable.notify_one();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(Data & popped_value)
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        if (the_queue.empty())
        {
            return false;
        }

        popped_value = the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_pop(Data & popped_value)
    {
        std::unique_lock<std::mutex> lock(the_mutex);
        while (the_queue.empty())
        {
            the_condition_variable.wait(lock);
        }

        popped_value = the_queue.front();
        the_queue.pop();
    }
};
}
