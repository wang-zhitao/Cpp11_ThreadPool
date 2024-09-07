#pragma once

#include <functional>
#include <future>

template <typename F, typename... Args>
using JobReturnType = typename std::result_of<F(Args...)>::type;

// 定义任务优先级枚举类型
enum class Priority : int { Normal, High, Critical };

class ThreadPool {
  public:
    // 默认构造函数，线程数量默认为硬件线程数
    ThreadPool(size_t min, size_t max = std::thread::hardware_concurrency());

    // 移动构造函数和移动赋值运算符默认实现
    ThreadPool(ThreadPool &&) = default;
    ThreadPool &operator=(ThreadPool &&) = default;

    // 析构函数
    ~ThreadPool();

    // 禁止复制构造函数和复制赋值运算符
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    void run();

    // 添加具有指定优先级的任务，并返回一个 future 对象
    template <typename F, typename... Args>
    auto addTask(Priority priority, F &&f,
                 Args &&...args) -> std::future<JobReturnType<F, Args...>> {
        // 创建一个 shared_ptr 指向 packaged_task 对象，用于封装任务函数和参数
        auto job =
            std::make_shared<std::packaged_task<JobReturnType<F, Args...>()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        // 添加任务到线程池，并指定优先级
        AddJob([job] { (*job)(); }, priority);

        // 返回任务的 future 对象，用于获取任务执行结果
        return job->get_future();
    }

    // 添加默认优先级（Normal）的任务，并返回一个 future 对象
    template <typename F, typename... Args>
    auto addTask(F &&f,
                 Args &&...args) -> std::future<JobReturnType<F, Args...>> {
        // 调用具有优先级参数的 Schedule 函数，使用默认优先级 Normal
        return addTask(Priority::Normal, std::forward<F>(f),
                       std::forward<Args>(args)...);
    }

  private:
    // 内部使用，添加任务函数，接受一个无参数无返回值的函数对象和优先级参数
    void AddJob(std::function<void()> job, Priority priority);

    // 实现类，使用 impl 技术隐藏实现细节
    class impl;

    std::unique_ptr<impl> m_impl;
};