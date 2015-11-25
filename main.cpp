#include "common/RayTracer.h"

#define ASSIGNMENT 8
#if ASSIGNMENT == 5
#define APPLICATION Assignment5
#include "assignment5/Assignment5.h"
#elif ASSIGNMENT == 6
#define APPLICATION Assignment6
#include "assignment6/Assignment6.h"
#elif ASSIGNMENT == 7
#define APPLICATION Assignment7
#include "assignment7/Assignment7.h"
#elif ASSIGNMENT == 8
#define APPLICATION Assignment8
#include "assignment8/Assignment8.h"
#endif

#ifdef _WIN32
#define WAIT_ON_EXIT 1
#else
#define WAIT_ON_EXIT 0
#endif

int main(int argc, char** argv)  
{
    std::unique_ptr<APPLICATION> currentApplication = make_unique<APPLICATION>();
	currentApplication->SetImageOutputResolution(glm::vec2(1024, 768)); // (glm::vec2(1600, 1200)) (1280, 960) (1024, 768)
	currentApplication->SetSamplesPerPixel(2);
	currentApplication->SetMinSamplesPerPixel(1);
	currentApplication->SetAdaptiveCoef(10.f);
	currentApplication->SetGridSize(glm::ivec3(1, 1, 1));
	currentApplication->SetUseAdaptiveSampler(false);
	currentApplication->SetOutputFilename("Assignment8/Caustics/A8 500000 transparent + reflection 2.0 2 samples 2 lights.png");
	currentApplication->SetMaxReflectionBounces(3);
	currentApplication->SetMaxRefractionBounces(5);
	currentApplication->SetAcceleratingStructureType(1);

	const std::string logFile = "Assignment8/Caustics/Caustics.txt";

	std::fstream fcout;
	fcout.open(logFile, std::fstream::out | std::fstream::app);
	fcout << "Samples per pixel " << currentApplication->GetSamplesPerPixel() << std::endl;
	fcout << "Min samples per pixel " << currentApplication->GetMinSamplesPerPixel() << std::endl;
	fcout << "Max reflections bounces " << currentApplication->GetMaxReflectionBounces() << std::endl;
	fcout << "Max refraction bounces " << currentApplication->GetMaxRefractionBounces() << std::endl;
	fcout << "Acceleration structure ";
	switch (static_cast<int>(currentApplication->GetAcceleratingStructureType()))
	{
	case 0:
		fcout << "NONE";
		break;
	case 1:
		fcout << "UNIFORM_GRID";
		break;
	case 2:
		fcout << "BVH";
		break;
	default:
		fcout << "Unknown type";
		break;
	} 
	fcout << std::endl;
	fcout << "Threads number " << numThreads << std::endl;

    RayTracer rayTracer(std::move(currentApplication));

	DIAGNOSTICS_TIMER(timer, "Initialization", logFile);
	rayTracer.Init();
	DIAGNOSTICS_END_TIMER(timer);
	DIAGNOSTICS_TIMER(timer2, "Ray Tracer", logFile);
    rayTracer.Run();
    DIAGNOSTICS_END_TIMER(timer2);

    DIAGNOSTICS_PRINT();
	DIAGNOSTICS_FILE_PRINT(logFile);

#if defined(_WIN32) && WAIT_ON_EXIT
    int exit = 0;
    std::cin >> exit;
#endif

    return 0;
}