#include "common/Utility/Timer/Timer.h"

Timer::Timer(const std::string& descriptor, const std::string& logFile):
    storedDescriptor(descriptor), fileName(logFile), tickHandled(false)
{
    Tick(fileName);
}

Timer::~Timer()
{
    Tock();
}

void Timer::Tick()
{
    startTime = std::chrono::high_resolution_clock::now();
    tickHandled = false;
}

void Timer::Tick(const std::string& file)
{
	std::fstream fcout;
	fcout.open(file, std::fstream::out | std::fstream::app);

	fcout << "START " << storedDescriptor << std::endl;
	startTime = std::chrono::high_resolution_clock::now();
}

void Timer::Tock()
{
    if (tickHandled)  
	{
        return;
    }
    tickHandled = false;

	if (0)
	{
		auto endTime = std::chrono::high_resolution_clock::now();
		auto totalElapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
		std::cout << "END " << storedDescriptor << ": " << totalElapsedTime.count() << " seconds" << std::endl;
	}
	else
	{
		std::fstream fcout;
		fcout.open(fileName, std::fstream::out | std::fstream::app);

		auto endTime = std::chrono::high_resolution_clock::now();
		auto totalElapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
		fcout << "END " << storedDescriptor << ": " << totalElapsedTime.count() << " seconds" << std::endl;
	}
}