#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <thread>
#include <mutex>
#include <stack>
#include <queue>
#include <map>
#include <condition_variable>
#include <chrono>

using namespace std::chrono_literals;

class Tester
{
public:
    using mutexUniqueLock = std::unique_lock<std::mutex>;
    Tester(int n)
    {

        threads.push_back(std::thread{[this, n]()
                                      {
                                          for (int i = 0; i < n; i++)
                                          {
                                              mutexUniqueLock lck{mtx};

                                              if (!start)
                                              {

                                                  cv0.wait(lck, [this]()
                                                           { return step == 0; });
                                              }
                                              else
                                              {
                                                  start = false;
                                              }

                                              std::cerr << "0 ";
                                              {
                                                  lck.unlock();
                                                  std::lock_guard<std::mutex> guard{mtx};

                                                  if (isOdd)
                                                  {
                                                      step = 1;
                                                  }
                                                  else
                                                  {
                                                      step = 2;
                                                  }
                                              }
                                              isOdd ? cv1.notify_one() : cv2.notify_one();
                                              isOdd = !isOdd;
                                          }
                                      }});
        threads.push_back(std::thread{[this, n]()
                                      {
                                          for (int i = 1; i <= n; i += 2)
                                          {

                                              mutexUniqueLock lck{mtx};

                                              cv1.wait(lck, [this]()
                                                       { return step == 1; });

                                              std::cerr << i << " ";
                                              {
                                                  lck.unlock();
                                                  std::lock_guard<std::mutex> guard{mtx};
                                                  step = 0;
                                              }
                                              cv0.notify_one();
                                          }
                                      }});
        threads.push_back(std::thread{[this, n]()
                                      {
                                          for (int i = 2; i <= n; i += 2)
                                          {
                                              mutexUniqueLock lck{mtx};

                                              cv2.wait(lck, [this]()
                                                       { return step == 2; });

                                              std::cerr << i << " ";
                                              {
                                                  lck.unlock();
                                                  std::lock_guard<std::mutex> guard{mtx};
                                                  step = 0;
                                              }
                                              cv0.notify_one();
                                          }
                                      }});
    }
    ~Tester()
    {
        for (auto &t : threads)
        {
            t.join();
        }
    }

private:
    std::vector<std::thread> threads{};
    std::condition_variable cv0{};
    std::condition_variable cv1{};
    std::condition_variable cv2{};
    bool start{true};
    bool isOdd{true};
    std::mutex mtx{};
    unsigned int step{};
};

int main()
{
    Tester tester{20};

    return 0;
}