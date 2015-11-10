#pragma once

#include "common/common.h"
#include "common/Output/ImageWriter.h"

static const int numThreads = 8;

class ImageWriter;

class RayTracer 
{
public:
    RayTracer(std::unique_ptr<class Application> app);

	void Init();

	void CalculatePixels(int ymin, int ymax);
    void Run();
	void Run2();

private:
    std::unique_ptr<class Application> storedApplication;

	std::shared_ptr<class Camera>		currentCamera;
	std::shared_ptr<class Scene>		currentScene;
	std::shared_ptr<class ColorSampler>	currentSampler;
	std::shared_ptr<class Renderer>		currentRenderer;

	glm::vec2		currentResolution;
	ImageWriter		imageWriter;
	int				maxSamplesPerPixel;

	std::vector<std::thread> vThreads;
};