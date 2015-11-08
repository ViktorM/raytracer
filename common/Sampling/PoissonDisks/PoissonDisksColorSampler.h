#pragma once

#include "common/Sampling/ColorSampler.h"


struct PoissonDisksSamplerState : public SamplerState
{
	PoissonDisksSamplerState(std::random_device& device, int inputMax, int inputDim) :
		SamplerState(device, inputMax, inputDim), numSamples(0)
	{
	}

	int numSamples;
};

class PoissonDisksColorSampler : public ColorSampler
{
public:
	void SetRadius(float inputRadius);

	virtual std::unique_ptr<SamplerState> CreateSampler(std::random_device& randomDevice, const int maxSamples, const int dimensions) const override;
	virtual glm::vec3 ComputeSampleCoordinate(SamplerState& state) const override;
private:
	float radius;
};