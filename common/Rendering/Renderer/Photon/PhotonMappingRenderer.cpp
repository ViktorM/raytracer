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

#define VISUALIZE_PHOTON_MAPPING 0
#define ENABLE_GATHER 1


PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(1000000), // 2000000
    maxPhotonBounces(20) // 1000
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
        totalLightIntensity += glm::length(currentLight->GetLightColor());
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
	const Material* hitMaterial = hitMeshObject->GetMaterial();
	const glm::vec3 diffuseColor = hitMaterial->GetBaseDiffuseReflection();
	const glm::vec3 specularColor = hitMaterial->GetBaseSpecularReflection();
	const glm::vec3 transmittanceColor = hitMaterial->GetBaseTransmittance();

	float Pr = std::max(diffuseColor.x + specularColor.x, std::max(diffuseColor.y + specularColor.y, diffuseColor.z + specularColor.z));
	float total = diffuseColor.x + diffuseColor.y + diffuseColor.z + specularColor.x + specularColor.y + specularColor.z;

	float diffusePr = (diffuseColor.x + diffuseColor.y + diffuseColor.z) * Pr / total;
	float specularPr = (specularColor.x + specularColor.y + specularColor.z) * Pr / total;


	glm::vec3 newLightIntensity = lightIntensity / Pr;
	newLightIntensity *= diffuseColor;

	Photon myPhoton;
	myPhoton.position = intersectionPoint;
	myPhoton.normal = norm;
	myPhoton.intensity = lightIntensity; // newLightIntensity; 
	myPhoton.toLightRay = Ray(intersectionPoint, -photonRay->GetRayDirection());

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.0, std::nextafter(1.f, FLT_MAX));

	float rx = dist(mt);
	if (rx > Pr)
	{
		if (path.size() > 1)
			photonMap.insert(myPhoton);

		return;
	}

	myPhoton.intensity = newLightIntensity;

	if (path.size() > 1)
	{
		photonMap.insert(myPhoton);
	}

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

	path.push_back('D');
	TracePhoton(photonMap, &reflectionRay, newLightIntensity, path, currentIOR, remainingBounces);
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);
	glm::vec3 additionalColor = glm::vec3(0.f, 0.f, 0.f);

#if VISUALIZE_PHOTON_MAPPING
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

    std::vector<Photon> foundPhotons;
    diffuseMap.find_within_range(intersectionVirtualPhoton, 0.001f, std::back_inserter(foundPhotons)); // 0.003f
    if (!foundPhotons.empty()) 
	{
        finalRenderColor += glm::vec3(1.f, 1.f, 1.f); // glm::vec3(1.f, 0.f, 0.f);
    }

#else 
#if ENABLE_GATHER
	if (intersection.hasIntersection)
	{
		glm::vec3 intersectionPoint = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
		Photon intersectionVirtualPhoton;
		intersectionVirtualPhoton.position = intersectionPoint;

		std::vector<Photon> foundPhotons;

		float radius = 0.05f; // 0.01 - good result
		int n = 100; // 300/2000000 150/1000000;

		float area = radius * radius;

		diffuseMap.find_within_range(intersectionVirtualPhoton, radius, std::back_inserter(foundPhotons));
#if 0
		std::sort(foundPhotons.begin(), foundPhotons.end(), [intersectionPoint](Photon a, Photon b)
		{
			return glm::dot(a.position - intersectionPoint, a.position - intersectionPoint) < dot(b.position - intersectionPoint, b.position - intersectionPoint);
		});
#endif

		const MeshObject* hitMeshObject = intersection.intersectedPrimitive->GetParentMeshObject();
		const Material* hitMaterial = hitMeshObject->GetMaterial();

		int sz = foundPhotons.size();	// temp
		float r = radius;				// temp
		int count = 0;
		float minArea = 0.f;
		for (int i = 0; i < sz; ++i)
		{
			Photon photon = foundPhotons[i];

			const glm::vec3 N = intersection.ComputeNormal();
			if (std::abs(glm::dot(photon.normal, N) - 1.f) > 1000.f * LARGE_EPSILON)
				continue;

			const glm::vec3 L = photon.toLightRay.GetRayDirection();
			const glm::vec3 V = -1.f * fromCameraRay.GetRayDirection();
			const glm::vec3 H = glm::normalize(L + V);

			const float NdL = std::min(std::max(glm::dot(N, L), 0.f), 1.f);
			const float NdH = std::min(std::max(glm::dot(N, H), 0.f), 1.f);
			const float NdV = std::min(std::max(glm::dot(N, V), 0.f), 1.f);
			const float VdH = std::min(std::max(glm::dot(V, H), 0.f), 1.f);

			glm::vec3 diffuseColor = hitMaterial->ComputeDiffuse(intersection, photon.intensity, NdL, NdH, NdV, VdH);
			glm::vec3 specularColor = hitMaterial->ComputeSpecular(intersection, photon.intensity, NdL, NdH, NdV, VdH);

			glm::vec3 brdfColor = hitMaterial->ComputeBRDF(intersection, photon.intensity, photon.toLightRay, fromCameraRay, 1.f, true, true);

			float modV = glm::dot(brdfColor, brdfColor);
			float maxC = std::max(brdfColor.x, std::max(brdfColor.y, brdfColor.z));

			float scale = 2.7f * maxC * maxC / modV; // 3.f
			if (scale > 0.1f && scale < 4.f)
			{
				brdfColor = brdfColor * scale;
			}

			float distSqr = glm::dot(intersectionPoint - photon.position, intersectionPoint - photon.position);
			float dist = sqrt(0.5f * distSqr); // square to sphere

			minArea = distSqr > minArea ? distSqr : minArea;

		//	float dist = std::max(std::abs((intersectionPoint - photon.position).x),
		//				std::max(std::abs((intersectionPoint - photon.position).y), std::abs((intersectionPoint - photon.position).z)));

			count++;
			float noweight = 1.f;

			// Cone filter
			float k = 1;
			float norm = 1.f - 2.f / (3.f * k);
			float weight = 1.f / norm * (1.f - k * dist / r); 

			// Gauss filter
			float alpha = 0.918;
			float beta = 1.953;
//			float gaussW = alpha * (1.f - (1.f - pow(e())/());
			
			float weight2 = 3.f * (1.f - sqrt(dist / r));
			float weight4 = 5.f * (1.f - sqrt(sqrt(dist / r)));
			additionalColor += weight * brdfColor; // photon.intensity;
#if 0
			if (count > n)
				break;
#endif
		}
		if (minArea > 0.f && minArea < area && count > 2)
			area = minArea;

		finalRenderColor += additionalColor / area;
	}
#else

//	finalRenderColor += 0.1f * glm::normalize(additionalColor);
	if (additionalColor.x > additionalColor.y)
	{
		if (additionalColor.x > additionalColor.z)
		{
			finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
		} 
		else
		{
			finalRenderColor += glm::vec3(0.f, 0.f, 1.f);
		}
	}
	else if (additionalColor.y > additionalColor.x)
	{
		if (additionalColor.y > additionalColor.z)
		{
			finalRenderColor += glm::vec3(0.f, 1.f, 0.f);
		}
		else
		{
			finalRenderColor += glm::vec3(0.f, 0.f, 1.f);
		}
	}
	else
	{
//		finalRenderColor += glm::vec3(0.8f, 0.8f, 0.8f);
	}
#endif
#endif

	return finalRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}