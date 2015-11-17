#include "common/Rendering/Renderer/Photon/PhotonMappingRenderer.h"
#include "common/Scene/Scene.h"
#include "common/Sampling/ColorSampler.h"
#include "common/Scene/Lights/Light.h"
#include "common/Scene/Geometry/Primitives/Primitive.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "common/Intersection/IntersectionState.h"
#include "common/Scene/SceneObject.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "glm/gtx/component_wise.hpp"

#define VISUALIZE_PHOTON_MAPPING 1
#define STORE_ABSORBED 1

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(1000000), // 1000000
    maxPhotonBounces(1000) // 1000
{
    srand(static_cast<unsigned int>(time(NULL)));
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
    GenericPhotonMapGeneration(diffuseMap, diffusePhotonNumber);
    diffuseMap.optimise();
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(PhotonKdtree& photonMap, int totalPhotons)
{
    float totalLightIntensity = 0.f;
    size_t totalLights = storedScene->GetTotalLights();

    for (size_t i = 0; i < totalLights; ++i) 
	{
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) 
		{
            continue;
        }
        totalLightIntensity = glm::length(currentLight->GetLightColor());
    }

    // Shoot photons -- number of photons for light is proportional to the light's intensity relative to the total light intensity of the scene.
    for (size_t i = 0; i < totalLights; ++i) 
	{
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) 
		{
            continue;
        }

        const float proportion = glm::length(currentLight->GetLightColor()) / totalLightIntensity;
        const int totalPhotonsForLight = static_cast<const int>(proportion * totalPhotons);

		if (totalPhotonsForLight > 0)
		{
			const glm::vec3 photonIntensity = currentLight->GetLightColor() / static_cast<float>(totalPhotonsForLight);
			for (int j = 0; j < totalPhotonsForLight; ++j)
			{
				Ray photonRay;
				std::vector<char> path;
				path.push_back('L');
				currentLight->GenerateRandomPhotonRay(photonRay);
				TracePhoton(photonMap, &photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
			}
		}
    }
}


void PhotonMappingRenderer::TracePhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces)
{
	if (remainingBounces < 0)
	{
		return;
	}

    assert(photonRay);
    IntersectionState state(0, 0);
    state.currentIOR = currentIOR;

	bool hit = storedScene->Trace(photonRay, &state);
	if (!hit)
	{
		return;
	}

	glm::vec3 intersectionPoint = state.intersectionRay.GetRayPosition(state.intersectionT);
	glm::vec3 norm = state.ComputeNormal();

	// Move intersection point above the surface
	intersectionPoint += LARGE_EPSILON * norm;

	const MeshObject* hitMeshObject = state.intersectedPrimitive->GetParentMeshObject();
	const Material* hitMaterial = hitMeshObject->GetMaterial();	const glm::vec3 diffuseColor = hitMaterial->GetBaseDiffuseReflection();

	Photon myPhoton;
	myPhoton.position = intersectionPoint;
	myPhoton.intensity = lightIntensity;
	myPhoton.toLightRay = Ray(intersectionPoint, -photonRay->GetRayDirection());

#if STORE_ABSORBED
	if (path.size() > 1)
	{
		photonMap.insert(myPhoton);
	}
#endif

	float reflectionPr = std::max(diffuseColor.x, std::max(diffuseColor.y, diffuseColor.z));	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.0, std::nextafter(1.f, FLT_MAX));

	float rx = dist(mt);
	if (rx > reflectionPr)
	{
		return;
	}

#if !STORE_ABSORBED
	if (path.size() > 1)
	{
		photonMap.insert(myPhoton);
	}
#endif

	Ray reflectionRay;
	std::random_device rd2;
	std::mt19937 mt2(rd2());
	std::uniform_real_distribution<float> distXY(-1.0, std::nextafter(1.f, FLT_MAX));
	std::uniform_real_distribution<float> distZ(0.0, std::nextafter(1.f, FLT_MAX));

	float x, y, z;
	do
	{
		x = distXY(mt2);
		y = distXY(mt2);
		z = distZ(mt2);
	} while (x*x + y*y + z*z > 1.f);

	glm::vec3 rayDirection = glm::normalize(glm::vec3(x, y, z));

	glm::vec3 mult = glm::vec3(1.f, 0.f, 0.f);
	float areParallel = std::abs(dot(norm, mult));

	if (std::abs(1.f - areParallel) <= 100.f * LARGE_EPSILON)
	{
		mult = glm::vec3(0.f, 1.f, 0.f);
	}

	glm::vec3 tang = cross(norm, mult);
	glm::vec3 bitang = cross(norm, tang);

	glm::mat3x3 transform = glm::mat3x3(glm::normalize(tang), glm::normalize(bitang), glm::normalize(norm));
	rayDirection = transform * rayDirection;

	reflectionRay.SetRayPosition(intersectionPoint);
	reflectionRay.SetRayDirection(rayDirection);

	--remainingBounces;

	path.push_back('R');
	TracePhoton(photonMap, &reflectionRay, lightIntensity, path, currentIOR, remainingBounces); // lightIntensity?
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);

#if VISUALIZE_PHOTON_MAPPING
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

    std::vector<Photon> foundPhotons;
    diffuseMap.find_within_range(intersectionVirtualPhoton, 0.003f, std::back_inserter(foundPhotons));
    if (!foundPhotons.empty()) 
	{
        finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
    }
#endif

    return finalRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}