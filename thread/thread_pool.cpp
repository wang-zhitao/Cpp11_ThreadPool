#include "thread_pool.h"
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
class ThreadPool::impl {
  public:
    void Init(size_t min, size_t max);
    void Shutdown();
    void AddJob(std::function<void()> &&job, Priority priority);
    void start();

  private:
    void worker();
    void manager();
    std::mutex m_jobsMutex;
    std::condition_variable m_cvSleepCtrl;
    std::unordered_map<std::thread::id, std::thread> m_workers;
    std::thread m_manager;
    // std::map<Priority, std::queue<std::function<void()>>,
    //          std::greater<Priority>>
    //     m_jobsByPriority;
    std::map<Priority, std::queue<std::function<void()>>> m_jobsByPriority;
    std::mutex m_idsMutex;
    std::vector<std::thread::id> m_ids; // 待删除线程ID集合
    int m_minThreads;                   // 最小线程数
    int m_maxThreads;                   // 最大线程数
    std::atomic<bool> m_stop;           // 停止标志
    std::atomic<int> m_curThreads;      // 当前线程数
    std::atomic<int> m_idleThreads;     // 空闲线程数
    std::atomic<int> m_exitNumber;      // 退出线程数
};

ThreadPool::ThreadPool(size_t min, size_t max)
    : m_impl(std::make_unique<ThreadPool::impl>()) {
    m_impl->Init(min, max);
}

ThreadPool::~ThreadPool() { m_impl->Shutdown(); }

void ThreadPool::run() { m_impl->start(); }

void ThreadPool::AddJob(std::function<void()> job, Priority priority) {
    m_impl->AddJob(std::move(job), priority);
}

void ThreadPool::impl::Init(size_t min, size_t max) {
    m_jobsByPriority[Priority::Normal] = {};
    m_jobsByPriority[Priority::High] = {};
    m_jobsByPriority[Priority::Critical] = {};

    m_minThreads = min;
    m_curThreads = min;
    m_idleThreads = min;
    m_maxThreads = max;
    m_stop = true;
    m_exitNumber = 0;
}

void ThreadPool::impl::start() {
    if (!m_stop.load()) {
        std::cout << "线程池已经启动" << std::endl;
        return;
    }
    m_stop.store(false);
    m_idleThreads = m_curThreads = m_minThreads;
    std::cout << "启动线程池，线程数量: " << m_curThreads << std::endl;

    // 启动管理线程
    std::thread manager(&ThreadPool::impl::manager, this);
    m_manager = std::move(manager);
    // 启动最小数量的工作线程
    for (int i = 0; i < m_curThreads; ++i) {
        std::thread t(&ThreadPool::impl::worker, this);
        m_workers.insert(make_pair(t.get_id(), move(t)));
    }
}

void ThreadPool::impl::worker() {
    while (!m_stop.load()) {
        std::function<void()> job;

        {
            std::unique_lock<std::mutex> locker(m_jobsMutex);
            while (!m_stop.load() && m_jobsByPriority.empty()) {
                m_cvSleepCtrl.wait(locker);
                if (m_exitNumber > 0) {
                    m_exitNumber--;
                    m_curThreads--;
                    m_idleThreads--;
                    std::lock_guard<std::mutex> lck(m_idsMutex);
                    m_ids.emplace_back(std::this_thread::get_id());
                    return;
                }
            }

            for (auto &kvp : m_jobsByPriority) {
                if (!kvp.second.empty()) {
                    job = std::move(kvp.second.front());
                    kvp.second.pop();
                    break;
                }
            }
        }

        if (job) {
            m_idleThreads--;
            job();
            m_idleThreads++;
        }
    }
}

void ThreadPool::impl::manager() {
    while (!m_stop.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        if (m_idleThreads > m_curThreads / 2 &&
            m_curThreads > m_minThreads + 2) {
            m_exitNumber.store(2); // 设置退出数量
            m_cvSleepCtrl.notify_all();
            std::lock_guard<std::mutex> lck(m_idsMutex);
            for (const auto &id : m_ids) {
                auto it = m_workers.find(id);
                if (it != m_workers.end()) {
                    it->second.join();
                    m_workers.erase(it);
                }
            }
            m_ids.clear();
        } else if (m_idleThreads == 1 && m_curThreads < m_maxThreads) {
            std::thread t(&ThreadPool::impl::worker, this);
            m_workers.insert(make_pair(t.get_id(), move(t)));
            m_curThreads++;
            m_idleThreads++;
        }
    }
}

void ThreadPool::impl::Shutdown() {
    m_stop.store(true);
    m_cvSleepCtrl.notify_all();

    for (auto &it : m_workers) {
        if (it.second.joinable()) {
            it.second.join();
        }
    }
    if (m_manager.joinable()) {
        m_manager.join();
    }
}

void ThreadPool::impl::AddJob(std::function<void()> &&job, Priority priority) {
    std::lock_guard<std::mutex> ul(m_jobsMutex);
    m_jobsByPriority[priority].emplace(std::move(job));
    m_cvSleepCtrl.notify_one();
}