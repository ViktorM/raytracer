#include "common/Scene/Lights/Point/PointLight.h"


void PointLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
    const float distanceToOrigin = glm::distance(origin, lightPosition);
    output.emplace_back(origin, rayDirection, distanceToOrigin);
}

float PointLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    return 1.f;
}

void PointLight::GenerateRandomPhotonRay(Ray& ray) const
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(-1.0, std::nextafter(1.f, FLT_MAX));

	float x, y, z;
	do 
	{
		x = dist(mt);
		y = dist(mt);
		z = dist(mt);
	} while (x*x + y*y + z*z > 1.f);

	const glm::vec3 lightPosition = glm::vec3(GetPosition());
	const glm::vec3 rayDirection = glm::normalize(glm::vec3(x, y, z));

	ray.SetRayPosition(lightPosition);
	ray.SetRayDirection(rayDirection);
}