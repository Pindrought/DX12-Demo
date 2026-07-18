#pragma once
#include "pch.h"
#include <chrono>
#include <unordered_map>
#include <string>
#include <mutex>
#include <iostream>
#include <limits>

class Profiler
{
public:

    struct ProfileStats
    {
        uint64_t Calls = 0;

        double TotalMilliseconds = 0.0;

        double MinMilliseconds =
            std::numeric_limits<double>::max();

        double MaxMilliseconds = 0.0;


        void AddSample(double milliseconds)
        {
            Calls++;

            TotalMilliseconds += milliseconds;

            if (milliseconds < MinMilliseconds)
                MinMilliseconds = milliseconds;

            if (milliseconds > MaxMilliseconds)
                MaxMilliseconds = milliseconds;
        }


        double AverageMilliseconds() const
        {
            if (Calls == 0)
                return 0.0;

            return TotalMilliseconds / Calls;
        }
    };


public:

    static void Record(
        const char* name,
        double milliseconds);


    static void Print();


    static void Reset();


private:

    static std::unordered_map<std::string, ProfileStats> s_Stats;

    static std::mutex s_Mutex;
};



class ScopedTimer
{
public:

    ScopedTimer(const char* name)
        :
        m_Name(name),
        m_Start(
            std::chrono::high_resolution_clock::now())
    {
    }


    ~ScopedTimer()
    {
        auto end =
            std::chrono::high_resolution_clock::now();


        double milliseconds =
            std::chrono::duration<double, std::milli>(
                end - m_Start)
            .count();


        Profiler::Record(
            m_Name,
            milliseconds);
    }


private:

    const char* m_Name;

    std::chrono::high_resolution_clock::time_point m_Start;
};



#define PROFILE_SCOPE(name) \
    ScopedTimer profileTimer##__LINE__(name)


#define PROFILE_FUNCTION() \
    PROFILE_SCOPE(__FUNCTION__)