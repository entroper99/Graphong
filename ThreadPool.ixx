export module ThreadPool;

import std;

export class ThreadPool
{
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();
    void addTask(std::function<void()> func);
    size_t getAvailableThreads();
    size_t getRemainingTasks(); // 추가된 메서드
    void waitForThreads();
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable completion_condition;
    bool stop;
    size_t available_threads;
    size_t active_tasks;
    size_t remaining_tasks; // 남은 작업 수를 추적하기 위한 변수
    void worker_thread();
};

ThreadPool::ThreadPool(size_t num_threads) : stop(false), available_threads(num_threads), active_tasks(0), remaining_tasks(0)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers.emplace_back([this] { worker_thread(); });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers)
    {
        worker.join();
    }
}

void ThreadPool::addTask(std::function<void()> func)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop) throw std::runtime_error("ThreadPool is stopping");
        tasks.emplace(func);
        --available_threads;
        ++active_tasks;
        ++remaining_tasks; // 작업이 추가될 때 증가
    }
    condition.notify_one();
}

size_t ThreadPool::getAvailableThreads()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    return available_threads;
}

size_t ThreadPool::getRemainingTasks() // 남은 작업 수를 반환하는 메서드
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    return remaining_tasks;
}

void ThreadPool::waitForThreads()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    while (active_tasks > 0)
    {
        completion_condition.wait_for(lock, std::chrono::seconds(1), [this] { return active_tasks == 0; });
        std::wprintf(L"Remaining tasks: %zu\n", remaining_tasks);
    }
}

void ThreadPool::worker_thread()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            ++available_threads;
            --active_tasks;
            --remaining_tasks; // 작업이 완료될 때 감소
            if (active_tasks == 0) {
                completion_condition.notify_all();
            }
        }
    }
}
