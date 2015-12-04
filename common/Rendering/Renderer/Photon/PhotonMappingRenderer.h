#pragma once

#include "common/Rendering/Renderer.h"
#include "common/Rendering/Renderer/Photon/Photon.h"
#include <kdtree++/kdtree.hpp>
#include <functional>
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Renderer/Backward/BackwardRenderer.h"

class PhotonMappingRenderer : public BackwardRenderer
{
public:
    PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler);
    virtual void InitializeRenderer() override;
    glm::vec3 ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const override;

    void SetNumberOfDiffusePhotons(int diffuse);
	void SetNumberOfCausticPhotons(int caustic);
	void SetNumberOfGatherSamples(int samples);

	float SampleRangeLess(const float x, const float y) const;
	float SampleRange(const float x, const float y) const;

	glm::vec3 SampleHemisphereRayDirection() const;
	glm::vec3 SampleHemisphereRayDirectionGlobalSpace(const glm::vec3& normal) const;

private:
	using PhotonKdtree = KDTree::KDTree<3, Photon, PhotonAccessor>;
	glm::vec3 CalculateColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay, 
							const PhotonKdtree& photonMap, float radius, int n, bool useConeFilter = false) const;

	PhotonKdtree diffuseMap;
	PhotonKdtree causticMap;
	PhotonKdtree volumeMap;

    int diffusePhotonNumber;
	int causticPhotonNumber;

    int maxPhotonBounces;

	int gatherSamplesNumber;

	glm::vec3 ComputeGatherColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay, 
								const float diffuseRadius = 0.01, const float specularRadius = 0.002) const;

	void GenericPhotonMapGeneration(int totalPhotons, float lightingScale, bool includeDirect);
	void CausticPhotonMapGeneration(int totalPhotons, float lightingScale, bool includeDirect = false);

    void TracePhoton(Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces, bool withDirect = true);
};