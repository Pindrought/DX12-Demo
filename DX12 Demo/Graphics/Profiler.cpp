#include "pch.h"
#include "Profiler.h"


std::unordered_map<std::string, Profiler::ProfileStats>
Profiler::s_Stats;


std::mutex Profiler::s_Mutex;



void Profiler::Record(
    const char* name,
    double milliseconds)
{
    std::lock_guard lock(s_Mutex);


    auto& stats = s_Stats[name];

    stats.AddSample(milliseconds);
}

void SetConsoleColor(WORD color)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, color);
}

void Profiler::Print()
{
    std::lock_guard lock(s_Mutex);


    std::cout
        << "\n========== PROFILER ==========\n";


    for (const auto& [name, stats] : s_Stats)
    {
        std::cout
            << "\n"
            << name
            << "\n";

        std::cout
            << " Calls: "
            << stats.Calls
            << "\n";

        std::cout
            << " Avg:   "
            << stats.AverageMilliseconds()
            << " ms\n";

        std::cout
            << " Min:   "
            << stats.MinMilliseconds
            << " ms\n";

        if (stats.MaxMilliseconds > 5)
        {
            SetConsoleColor(
                FOREGROUND_RED | FOREGROUND_INTENSITY);
            std::cout
                << " Max:   "
                << stats.MaxMilliseconds
                << " ms\n";
            SetConsoleColor(
                FOREGROUND_RED |
                FOREGROUND_GREEN |
                FOREGROUND_BLUE);
        }
        else
        {
            std::cout
                << " Max:   "
                << stats.MaxMilliseconds
                << " ms\n";
        }
        

        std::cout
            << " Total: "
            << stats.TotalMilliseconds
            << " ms\n";
    }


    std::cout
        << "\n==============================\n";
}



void Profiler::Reset()
{
    std::lock_guard lock(s_Mutex);

    s_Stats.clear();
}