#include "common/Application.h"
#include "common/Acceleration/AccelerationCommon.h"
#include "common/Output/ImageWriter.h"


void Application::SetOutputFilename(const std::string& file)
{
	fileName = file;
}

std::string Application::GetOutputFilename() const
{
	return fileName;
}

void Application::SetSamplesPerPixel(int numSamples)
{
	samplesPerPixel = numSamples;
}

int Application::GetSamplesPerPixel() const
{
	return samplesPerPixel;
}

void Application::SetMinSamplesPerPixel(int numMinSamples)
{
	minSamplesPerPixel = numMinSamples;
}

int Application::GetMinSamplesPerPixel() const
{
	return minSamplesPerPixel;
}

void Application::SetGridSize(const glm::ivec3& grid)
{
	gridSize = grid;
}

glm::ivec3 Application::GetGridSize() const
{
	return gridSize;
}

glm::vec2 Application::GetImageOutputResolution() const
{
	return imageResolution;
}

void Application::PerformImagePostprocessing(class ImageWriter&)
{
}