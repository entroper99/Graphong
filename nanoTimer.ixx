export module nanoTimer;

import std;

static std::chrono::high_resolution_clock::time_point programStartTime = std::chrono::high_resolution_clock::now();

export void initNanoTimer()
{
    programStartTime = std::chrono::high_resolution_clock::now();
}

export __int64 getNanoTimer()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now - programStartTime).count();
}