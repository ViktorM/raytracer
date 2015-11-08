#include "common/Sampling/PoissonDisks/PoissonDisksColorSampler.h"


void PoissonDisksColorSampler::SetRadius(float inputRadius)
{
	assert(inputRadius >= 0.f);
	radius = inputRadius;
}

glm::vec3 PoissonDisksColorSampler::ComputeSampleCoordinate(SamplerState& state) const
{
	bool goodPlacement = false;

	glm::vec3 random = ColorSampler::ComputeSampleCoordinate(state);
	glm::vec3 newSample = (1.f - 2.f*radius) * random;
	while (!goodPlacement)
	{
		bool exited = false;
		for (auto pos: state.positionHistory)
		{
			float dist = sqrt((pos - newSample - radius).x * (pos - newSample - radius).x + (pos - newSample).y * (pos - newSample).y);
			if (dist < 2.f * radius)
			{
				random = ColorSampler::ComputeSampleCoordinate(state);
				newSample = (1.f - 2.f*radius) * random;

				exited = true;
				break;
			}
		}
		goodPlacement = !exited;
	}
	
	state.positionHistory.push_back(newSample + glm::vec3(radius));

	return newSample;
}

std::unique_ptr<SamplerState> PoissonDisksColorSampler::CreateSampler(std::random_device& randomDevice, const int maxSamples, const int dimensions) const
{
	std::unique_ptr< PoissonDisksSamplerState> state = make_unique< PoissonDisksSamplerState>(randomDevice, maxSamples, dimensions);
	state->numSamples = maxSamples;

	assert(state->numSamples > 0);

	return std::move(state);
}