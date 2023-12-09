#include <future>
#include <iostream>
#include <ctime>
#include <cstring>
#include <cassert>
#include <chrono>
#include <vector>
#include <iomanip>
#include <condition_variable>
#include <unistd.h>

inline void run_timing_test(unsigned int);

int main(int argc, char* argv[]){
    if(argc > 2 && !strncmp(argv[1], "--test", 6)){
        unsigned int i = std::stoi(argv[2]);
        run_timing_test(i);
    }
}

inline void run_timing_test(unsigned int i){
    static std::atomic<unsigned int> cnt = 0; // std::atomic so that reading the value of `cnt` while a thread is writing to it is well-defined behavior
    static std::mutex _cnt_mut;
    static std::condition_variable cv;

    auto start_wc_time = std::chrono::system_clock::now();
    
    unsigned int num_cores = std::thread::hardware_concurrency();
    std::vector<std::future<void>> v(num_cores);

    for(auto& ft : v){
        ft = std::async(std::launch::async, [](unsigned int s){
            sleep(s);
            std::lock_guard<std::mutex> lock(_cnt_mut); // Will hold until destructor is called at the end of this lambda scope
            ++cnt;
            cv.notify_one();
            // Now allow lock_guard's destructor to be called.
        }, i);
    }

    static std::mutex _wait_mut;
    std::unique_lock<std::mutex> lock(_wait_mut);
    cv.wait(lock, [num_cores]() { return cnt.load() >= num_cores;});

    std::chrono::duration<double> wc_time_duration = (std::chrono::system_clock::now() - start_wc_time);
    double time = wc_time_duration.count();

    std::cout << "[Wall-Clock-Time] " << std::setprecision(5) << time << "\n" << std::flush;
    

    assert(std::less_equal<double>{}(time, static_cast<double>(i) * 1.1));
}