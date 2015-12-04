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
#define DIRECT_VISUALIZATION 0
#define WITHOUT_CAUSTICS 0


PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(300000), // 2000000
	causticPhotonNumber(600000), // 100000
    maxPhotonBounces(10), // 1000
	gatherSamplesNumber(96)
{
    srand(static_cast<unsigned int>(time(NULL)));
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
	GenericPhotonMapGeneration(diffusePhotonNumber, 36.f, false);
#if !WITHOUT_CAUSTICS
	GenericPhotonMapGeneration(causticPhotonNumber, 1.f, false);
#endif

    diffuseMap.optimise();
	causticMap.optimise();
}

float PhotonMappingRenderer::SampleRangeLess(const float x, const float y) const
{
	assert(x < y);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(x, y);

	return dist(mt);
}

float PhotonMappingRenderer::SampleRange(const float x, const float y) const
{
	assert(x < y);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(x, std::nextafter(y, FLT_MAX));

	return dist(mt);
}

glm::vec3 PhotonMappingRenderer::SampleHemisphereRayDirection() const
{
	/*
	float x, y, z;
	do
	{
		x = SampleRange(-1.f, 1.f);
		y = SampleRange(-1.f, 1.f);
		z = SampleRange(0.f, 1.f);
	} while (x*x + y*y + z*z > 1.f);

	if (x*x + y*y + z*z < SMALL_EPSILON)
	{
		z += LARGE_EPSILON;
	}

	return glm::normalize(glm::vec3(x, y, z));
	*/

	float u = SampleRange(0.f, 1.f);
	float r = sqrt(u);
	float theta = 2.f * PI * SampleRangeLess(0.f, 1.f);

	float x = r * cos(theta);
	float y = r * sin(theta);
	float z = sqrt(1.f - u);

	return glm::vec3(x, y, z);
}

glm::vec3 PhotonMappingRenderer::SampleHemisphereRayDirectionGlobalSpace(const glm::vec3& normal) const
{
	assert(glm::length(normal) > 10.f * LARGE_EPSILON);
	glm::vec3 norm = glm::normalize(normal); // For safety
	glm::vec3 mult = glm::vec3(1.f, 0.f, 0.f);

	float areParallel = std::abs(dot(norm, mult));
	if (std::abs(1.f - areParallel) <= 10000.f * LARGE_EPSILON)
	{
		mult = glm::vec3(0.f, 1.f, 0.f);
	}

	glm::vec3 tang = glm::normalize(cross(norm, mult));
	glm::vec3 bitang = glm::normalize(cross(norm, tang));
	assert(glm::length(tang) > 10.f * LARGE_EPSILON);
	assert(glm::length(bitang) > 10.f * LARGE_EPSILON);

	glm::mat3x3 transform = glm::mat3x3(tang, bitang, norm);

	glm::vec3 rayDirection = SampleHemisphereRayDirection();
	rayDirection = transform * rayDirection;

	return rayDirection;
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(int totalPhotons, float lightingScale, bool includeDirect)
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
			glm::vec3 photonIntensity = lightingScale * currentLight->GetLightColor() / static_cast<float>(totalPhotonsForLight);

			int photonCount = totalPhotonsForLight;
			for (int i = 0; i < totalPhotonsForLight; ++i)
			{
				Ray photonRay;
				currentLight->GenerateRandomPhotonRay(photonRay);
				
				std::vector<char> path;
				path.push_back('L');
				TracePhoton(&photonRay, photonIntensity, path, 1.f, maxPhotonBounces, includeDirect);
			}
		}
    }
}

void PhotonMappingRenderer::CausticPhotonMapGeneration(int totalPhotons, float lightingScale, bool includeDirect)
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
			glm::vec3 photonIntensity = lightingScale * currentLight->GetLightColor() / static_cast<float>(totalPhotonsForLight);

			int photonCount = totalPhotonsForLight;
			for (int i = 0; i < totalPhotonsForLight;)
			{
				Ray photonRay;
				currentLight->GenerateRandomPhotonRay(photonRay);

				IntersectionState state(0, 0);
				bool hit = storedScene->Trace(&photonRay, &state);
				if (!hit)
				{
					continue;
				}

				const MeshObject* hitMeshObject = state.intersectedPrimitive->GetParentMeshObject();
				const Material* hitMaterial = hitMeshObject->GetMaterial();
				const float transmittance = hitMaterial->GetTransmittance();
				const float reflectivity = hitMaterial->GetReflectivity();

				if (transmittance + reflectivity < LARGE_EPSILON)
				{
					continue;
				}

				++i;

				std::vector<char> path;
				path.push_back('L');
				TracePhoton(&photonRay, photonIntensity, path, 1.f, maxPhotonBounces, includeDirect);
			}
		}
	}
}

void PhotonMappingRenderer::TracePhoton(Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces, bool withDirect)
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

	Photon myPhoton;
	myPhoton.position = intersectionPoint + LARGE_EPSILON * norm; // Move intersection point above the surface
	myPhoton.normal = norm;
	myPhoton.intensity = lightIntensity;
	glm::vec3 newLigthIntensity = lightIntensity;
	myPhoton.toLightRay = Ray(intersectionPoint, -photonRay->GetRayDirection());

	const MeshObject* hitMeshObject = state.intersectedPrimitive->GetParentMeshObject();
	const Material* hitMaterial = hitMeshObject->GetMaterial();

//	const glm::vec3 transmittanceColor = hitMaterial->GetBaseTransmittance(); // Don't need at the moment
	const float transmittance = hitMaterial->GetTransmittance();
	const float reflectivity = hitMaterial->GetReflectivity();
	assert(transmittance + reflectivity <= 1.f);

	Ray outputRay;

	float n2 = hitMaterial->GetIOR();
	const float NdR = glm::dot(photonRay->GetRayDirection(), norm);

	float rnd = SampleRange(0.f, 1.f);
	if (transmittance + reflectivity > LARGE_EPSILON)
	{
		if (rnd <= transmittance)
		{
			const glm::vec3 refractionDir = photonRay->RefractRay(norm, state.currentIOR, n2);
			outputRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * refractionDir);
			outputRay.SetRayDirection(refractionDir);

			myPhoton.position = intersectionPoint + LARGE_EPSILON * refractionDir; // Is it correct?

			path.push_back('S');
		}
		else if (transmittance < rnd && rnd <= (reflectivity + transmittance))
		{
			const glm::vec3 normal = (NdR > SMALL_EPSILON) ? -1.f * norm : norm;
			const glm::vec3 reflectionDir = glm::reflect(photonRay->GetRayDirection(), normal);
			outputRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * norm);
			outputRay.SetRayDirection(reflectionDir);

			n2 = currentIOR;

			path.push_back('S');
		} 
		else
		{
			return;
		}
	}
	else
	{
		n2 = currentIOR;

		// Blinn-Phong
		const glm::vec3 diffuseColor = hitMaterial->GetBaseDiffuseReflection();
		const glm::vec3 specularColor = hitMaterial->GetBaseSpecularReflection();

		if (path[path.size() - 1] == 'S')
		{
			causticMap.insert(myPhoton);
	//		diffuseMap.insert(myPhoton); // debug
		}
		else
		{
			if (path.size() > 1 || withDirect)
				diffuseMap.insert(myPhoton);
		}
		path.push_back('D');

		float Pr = std::max(diffuseColor.x + specularColor.x, std::max(diffuseColor.y + specularColor.y, diffuseColor.z + specularColor.z));
		float total = diffuseColor.x + diffuseColor.y + diffuseColor.z + specularColor.x + specularColor.y + specularColor.z;

		float Pd = (diffuseColor.x + diffuseColor.y + diffuseColor.z) * Pr / total;
		float Ps = (specularColor.x + specularColor.y + specularColor.z) * Pr / total;

		glm::vec3 newDiffuseLightIntensity = glm::vec3();
		if (Pd > SMALL_EPSILON)
			newDiffuseLightIntensity = lightIntensity * diffuseColor / Pd;

		glm::vec3 newSpecularLightIntensity = glm::vec3();
		if (Ps > SMALL_EPSILON)
			newSpecularLightIntensity = lightIntensity * specularColor / Ps;

		if (rnd <= Pd)
		{
			newLigthIntensity = newDiffuseLightIntensity;
		}
		else if (Pd < rnd && rnd <= Pd + Ps)
		{
			newLigthIntensity = newSpecularLightIntensity;
			return;
		}
		else
		{
			return;
		}

		glm::vec3 rayDirection = SampleHemisphereRayDirectionGlobalSpace(norm);

		outputRay.SetRayPosition(intersectionPoint);
		outputRay.SetRayDirection(rayDirection);
	}

	--remainingBounces;
	TracePhoton(&outputRay, newLigthIntensity, path, n2, remainingBounces);
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}

void PhotonMappingRenderer::SetNumberOfCausticPhotons(int caustic)
{
	causticPhotonNumber = caustic;
}

void PhotonMappingRenderer::SetNumberOfGatherSamples(int samplesNumber)
{
	gatherSamplesNumber = samplesNumber;
}

glm::vec3 PhotonMappingRenderer::CalculateColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay, 
												const PhotonKdtree& photonMap, float radius, int n, bool useConeFilter) const
{
	glm::vec3 addColor = glm::vec3();

#if VISUALIZE_PHOTON_MAPPING
	Photon intersectionVirtualPhoton;
	intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

	std::vector<Photon> foundPhotons;
	diffuseMap.find_within_range(intersectionVirtualPhoton, radius, std::back_inserter(foundPhotons)); // 0.003f
	if (!foundPhotons.empty())
	{
		addColor += glm::vec3(0.f, 0.0f, 0.55f); // glm::vec3(1.f, 0.f, 0.f);
	}

#else 
	if (intersection.hasIntersection)
	{
		glm::vec3 intersectionPoint = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
		Photon intersectionVirtualPhoton;
		intersectionVirtualPhoton.position = intersectionPoint;

		std::vector<Photon> foundPhotons;
		photonMap.find_within_range(intersectionVirtualPhoton, radius, std::back_inserter(foundPhotons));
		if (foundPhotons.size() == 0)
			return addColor;
#if 0
		if (foundPhotons.size() > 3 * n)
		{
			std::sort(foundPhotons.begin(), foundPhotons.end(), [intersectionPoint](Photon a, Photon b)
			{
				return glm::dot(a.position - intersectionPoint, a.position - intersectionPoint) < dot(b.position - intersectionPoint, b.position - intersectionPoint);
			});

			//	std::cout << "Radius = " << radius << " Photons found = " << foundPhotons.size() << std::endl;
//			radius *= 0.5f;
//			photonMap.find_within_range(intersectionVirtualPhoton, radius, std::back_inserter(foundPhotons));
		}
#endif
		const MeshObject* hitMeshObject = intersection.intersectedPrimitive->GetParentMeshObject();
		const Material* hitMaterial = hitMeshObject->GetMaterial();

		float area = PI * radius * radius;

		float r = radius;				// temp
		int count = 0;
		float minArea = 0.f;
		for (int i = 0; i < foundPhotons.size(); ++i)
		{
			Photon photon = foundPhotons[i];

			const glm::vec3 N = intersection.ComputeNormal();
			if (std::abs(std::abs(glm::dot(photon.normal, N)) - 1.f) > 1000.f * LARGE_EPSILON)
				continue;

			glm::vec3 brdfColor = hitMaterial->ComputeBRDF(intersection, photon.intensity, photon.toLightRay, fromCameraRay, 1.f, true, true);

			float modV = glm::length(brdfColor);
			float maxC = std::max(std::abs(brdfColor.x), std::max(std::abs(brdfColor.y), std::abs(brdfColor.z)));

			if (modV > SMALL_EPSILON)
			{
				float scale = 1.5f * maxC / modV; // 3.f
				scale = 1.f; // Testing
				brdfColor = brdfColor * scale;
			}

			float distSqr = glm::dot(intersectionPoint - photon.position, intersectionPoint - photon.position);
			float dist = sqrt(distSqr); // square to sphere
			if (dist > radius)
				continue;

			minArea = PI * distSqr > minArea ? PI * distSqr : minArea;

			count++;
			float noweight = 1.f;

			// Cone filter
			float k = 1;
			float norm = 1.f - 2.f / (3.f * k);
			float weight = 1.f / norm * (1.f - k * dist / r);

			// Gauss filter
			float alpha = 0.918;
			float beta = 1.953;
			//	float gaussW = alpha * (1.f - (1.f - pow(e())/());

			if (!useConeFilter)
			{
				weight = noweight;
			}
			addColor += weight * brdfColor;
#if 0
			if (count > n)
			{
				if (minArea > SMALL_EPSILON && minArea < area)
					area = minArea;
				break;
			}
#endif
		}

		addColor /= area;
	}
#endif

	return addColor;
}

glm::vec3 PhotonMappingRenderer::ComputeGatherColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay, const float diffuseRadius, const float specularRadius) const
{
	glm::vec3 diffuseColor = CalculateColor(intersection, fromCameraRay, diffuseMap, diffuseRadius, 200);
	glm::vec3 causticColor = CalculateColor(intersection, fromCameraRay, causticMap, specularRadius, 100);

	return diffuseColor + causticColor;
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
	if (!intersection.hasIntersection)
	{
		return glm::vec3();
	}

	glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);
	// Debug
//	glm::vec3 finalRenderColor = glm::vec3(); 

	float diffuseRadius = 0.03;
	float causticRadius = 0.02;

#if VISUALIZE_PHOTON_MAPPING
	return finalRenderColor + CalculateColor(intersection, fromCameraRay, diffuseMap, 0.007, 100);
#endif

#if DIRECT_VISUALIZATION
	return ComputeGatherColor(intersection, fromCameraRay, diffuseRadius, causticRadius);
#endif

#if !WITHOUT_CAUSTICS
	glm::vec3 causticColor = CalculateColor(intersection, fromCameraRay, causticMap, causticRadius, 100, true);
	finalRenderColor += 0.18f * causticColor;
#endif

	// Debug
//	return finalRenderColor;

	glm::vec3 normal = intersection.ComputeNormal();
	glm::vec3 hitPoint = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
	for (int i = 0; i < gatherSamplesNumber; ++i)
	{
		glm::vec3 sampleDir = SampleHemisphereRayDirectionGlobalSpace(normal);

		Ray sampleRay;
		sampleRay.SetRayDirection(sampleDir);
		sampleRay.SetRayPosition(hitPoint + LARGE_EPSILON * normal);

		// Fix! storedApplication->GetMaxReflectionBounces(), storedApplication->GetMaxRefractionBounces()
		IntersectionState sampleIntersection(2, 4); // 2, 4
		bool didHitScene = storedScene->Trace(&sampleRay, &sampleIntersection);

		// Use the intersection data to compute the BRDF response.
		if (!didHitScene)
		{
			continue;
		}

		glm::vec3 intersectionPoint = sampleIntersection.intersectionRay.GetRayPosition(intersection.intersectionT);
		glm::vec3 sampleColor = CalculateColor(intersection, fromCameraRay, diffuseMap, diffuseRadius, 200);

		const MeshObject* hitMeshObject = sampleIntersection.intersectedPrimitive->GetParentMeshObject();
		const Material* hitMaterial = hitMeshObject->GetMaterial();

		glm::vec3 brdfColor = hitMaterial->ComputeBRDF(intersection, sampleColor, sampleRay, fromCameraRay, 1.f, true, true);

		finalRenderColor += brdfColor / static_cast<float>(gatherSamplesNumber);
	}

	return finalRenderColor;
}