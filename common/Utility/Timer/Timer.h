#pragma once

#include "common/common.h"
#include <chrono>

class Timer
{
public:
    Timer(const std::string& descriptor, const std::string& logFile);
    ~Timer();

    // Starts the timer
    void Tick();
	void Tick(const std::string& file);

    // Ends the timer
    void Tock();
private:
    std::string storedDescriptor;
	std::string fileName;

    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    bool tickHandled;
};