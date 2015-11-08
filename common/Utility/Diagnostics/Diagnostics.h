#pragma once

#define DIAGNOSTICS_ON 1

enum class DiagnosticsType
{
    TRIANGLE_INTERSECTIONS = 0,
    BOX_INTERSECTIONS,
    RAYS_CREATED,
    MAX
};

#if DIAGNOSTICS_ON
#define DIAGNOSTICS_STAT(t) Diagnostics::Get()->IncrementStat(t)
#define DIAGNOSTICS_PRINT() Diagnostics::Get()->Print()
#define DIAGNOSTICS_FILE_PRINT(fileName) Diagnostics::Get()->FilePrint(fileName)
#define DIAGNOSTICS_TIMER(N,D,F) Timer N(D,F)
#define DIAGNOSTICS_END_TIMER(N) N.Tock()
#define DIAGNOSTICS_LOG(S) Diagnostics::Get()->Log(S)

#include <memory>
#include <map>

class Diagnostics
{
public:
    Diagnostics();

    static Diagnostics* Get();

    void IncrementStat(DiagnosticsType type);
    void Print();
	void FilePrint(const std::string& fileName = "log.txt");
    void Log(const std::string& log);
private:

    std::map<DiagnosticsType, int> statisticsAggregator;
};

#else
#define DIAGNOSTICS_STAT(t)
#define DIAGNOSTICS_PRINT
#define DIAGNOSTICS_TIMER(N,D)
#define DIAGNOSTICS_END_TIMER(N)
#define DIAGNOSTICS_LOG(S)
#endif