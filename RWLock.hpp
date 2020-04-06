//
// Created by Administrator on 2020/4/3.
//

#ifndef RWLOCK_HPP
#define RWLOCK_HPP

#include <mutex>
#include <condition_variable>

class RWLock {
public:
    RWLock() : m_read_count(0), m_write_count(0), is_writing(false) {
    }

    void lockRead() {
        std::unique_lock<std::mutex> gurad(m_mutex);
        m_read_cond.wait(gurad, [=] { return 0 == m_write_count; });
        ++m_read_count;
    }

    void unlockRead() {
        std::unique_lock<std::mutex> gurad(m_mutex);
        if (0 == (--m_read_count)
            && m_write_count > 0) {
            // One write can go on
            m_write_cond.notify_one();
        }
    }

    void lockWrite() {
        std::unique_lock<std::mutex> gurad(m_mutex);
        ++m_write_count;
        m_write_cond.wait(gurad, [=] { return (0 == m_read_count) && !is_writing; });
        is_writing = true;
    }

    void unlockWrite() {
        std::unique_lock<std::mutex> gurad(m_mutex);
        is_writing = false;
        if (0 == (--m_write_count)) {
            // All read can go on
            m_read_cond.notify_all();
        } else {
            // One write can go on
            m_write_cond.notify_one();
        }
    }

private:
    volatile int m_read_count;
    volatile int m_write_count;
    volatile bool is_writing;
    std::mutex m_mutex;
    std::condition_variable m_read_cond;
    std::condition_variable m_write_cond;
};

#endif

