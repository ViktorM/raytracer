#pragma once

#include "common/common.h"

enum class AccelerationTypes;

class Application : public std::enable_shared_from_this<Application>
{
public:
	Application() : samplesPerPixel(1), minSamplesPerPixel(1), maxReflectionBounces(0), maxRefractionBounces(0),
		gridSize(1, 1, 1), usePoissonDisksSampler(false), useAdaptiveSampler(false), imageResolution(1024, 768),
		fileName("output.png")
	{
	}
    virtual ~Application() {}

    virtual std::shared_ptr<class Camera> CreateCamera() const = 0;
    virtual std::shared_ptr<class Scene> CreateScene() const = 0;
    virtual std::shared_ptr<class ColorSampler> CreateSampler() const = 0;
    virtual std::shared_ptr<class Renderer> CreateRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler) const = 0;

    // Ray tracing properties
	virtual void SetMaxReflectionBounces(int outputMaxReflectionBounces)
	{
		maxReflectionBounces = outputMaxReflectionBounces;
	}
	virtual int GetMaxReflectionBounces() const
	{
		return maxReflectionBounces;
	}

	virtual void SetMaxRefractionBounces(int outputMaxRefractionBounces)
	{
		maxRefractionBounces = outputMaxRefractionBounces;
	}
	virtual int GetMaxRefractionBounces() const
	{
		return maxRefractionBounces;
	}

	// Sampling Properties
	virtual void SetSamplesPerPixel(int numSamples);
	virtual int GetSamplesPerPixel() const;

	virtual void SetMinSamplesPerPixel(int numMinSamples);
	virtual int GetMinSamplesPerPixel() const;

	virtual void SetGridSize(const glm::ivec3& grid);
	virtual glm::ivec3 GetGridSize() const;

	virtual void SetUsePoissonDisksSampler(bool usePoissonDisks)
	{
		usePoissonDisksSampler = usePoissonDisks;
	}
	virtual bool GetUsePoissonDisksSampler() const
	{
		return usePoissonDisksSampler;
	}

	// Adaptive sampler
	virtual void SetUseAdaptiveSampler(bool useAdaptive)
	{
		useAdaptiveSampler = useAdaptive;
	}
	virtual bool GetUseAdaptiveSampler() const
	{
		return useAdaptiveSampler;
	}

	virtual void SetAdaptiveCoef(float coef)
	{
		adaptiveCoef = coef;
	}
	virtual float GetAdaptiveCoef() const
	{
		return adaptiveCoef;
	}

	// Whether or not to continue sampling the scene from the camera.
	virtual bool NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex) = 0;

	virtual void SetAcceleratingStructureType(int accelerationStructureType)
	{
		accelerationStructure = static_cast<AccelerationTypes>(accelerationStructureType);
	}

	virtual void SetAcceleratingStructureType(AccelerationTypes accelerationStructureType)
	{
		accelerationStructure = accelerationStructureType;
	}

	virtual AccelerationTypes GetAcceleratingStructureType() const
	{
		return accelerationStructure;
	}

	// Postprocessing
	virtual void PerformImagePostprocessing(class ImageWriter& imageWriter);

	// Output
	virtual void SetImageOutputResolution(const glm::vec2& imageSize)
	{
		imageResolution = imageSize;
	}
	virtual glm::vec2 GetImageOutputResolution() const;

	virtual void SetOutputFilename(const std::string& file);
	virtual std::string GetOutputFilename() const;

private:
	int			samplesPerPixel;
	int			minSamplesPerPixel;

	int			maxReflectionBounces;
	int			maxRefractionBounces;

	glm::ivec3	gridSize;

	bool		usePoissonDisksSampler;
	bool		useAdaptiveSampler;
	float		adaptiveCoef;

	AccelerationTypes accelerationStructure;

	glm::vec2	imageResolution;
	std::string	fileName;
};