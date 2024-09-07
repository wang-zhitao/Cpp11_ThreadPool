#include "thread_pool.h"
#include <future>
#include <iostream>
#include <vector>
const int N = 100;

#if 0
void testTask(int priority, int taskId) {
    std::cout << "Task " << taskId << " (Priority: " << priority
              << ") is executing " << std::endl;
}

int main() {
    ThreadPool thread_pool(2, 10);
    srand(time(0));

    for (size_t i = 0; i < N; i++) {
        int p = rand() % 3;
        thread_pool.addTask(static_cast<Priority>(p), testTask, p, i);
    }

    thread_pool.run();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}
#else
std::string calc(int priority, int x, int y) {
    int res = x + y;
    // cout << "res = " << res << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return "priority:" + std::to_string(priority) +
           "   result:" + std::to_string(res);
}

int main() {
    srand(time(0));
    ThreadPool pool(2, 4);
    std::vector<std::future<std::string>> results;

    for (int i = 0; i < N; ++i) {
        int p = rand() % 3;
        auto future = pool.addTask(static_cast<Priority>(p), calc, p, i, i * 2);
        results.push_back(std::move(future));
    }
    pool.run();
    // 不阻塞等待，持续检查是否有结果就绪并输出
    while (!results.empty()) {
        for (auto it = results.begin(); it != results.end();) {
            if (it->wait_for(std::chrono::milliseconds(0)) ==
                std::future_status::ready) {
                std::cout << "线程函数返回值: " << it->get() << std::endl;
                it = results.erase(it);
            } else {
                ++it;
            }
        }
    }

    return 0;
}
#endif
