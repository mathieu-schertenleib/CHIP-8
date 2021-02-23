#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

void start_timer(const std::function<void(void)> &func,
                 std::chrono::milliseconds interval)
{
    std::thread([func, interval]() {
        while (true)
        {
            const auto end_time {std::chrono::steady_clock::now() + interval};
            func();
            std::this_thread::sleep_until(end_time);
        }
    }).detach();
}

void do_something()
{
    std::cout << "I am doing something" << std::endl;
}

auto main() -> int
{
    using namespace std::chrono_literals;
    start_timer(do_something, 500ms);
    std::this_thread::sleep_for(5s);
    return EXIT_SUCCESS;
}