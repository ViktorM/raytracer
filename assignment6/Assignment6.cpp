#include "assignment6/Assignment6.h"
#include "common/core.h"

std::shared_ptr<Camera> Assignment6::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    return camera;
}

std::shared_ptr<Scene> Assignment6::CreateScene() const
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
    cubeMaterial->SetReflectivity(0.3f);

    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment6-Alt.obj", &loadedMaterials);
    for (size_t i = 0; i < cubeObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        cubeObjects[i]->SetMaterial(materialCopy);

        std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
        cubeSceneObject->AddMeshObject(cubeObjects[i]);
        cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
        cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);

        cubeSceneObject->ConfigureAccelerationStructure([](AccelerationStructure* genericAccelerator) {
            BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
            accelerator->SetMaximumChildren(2);
            accelerator->SetNodesOnLeaves(2);
        });

        cubeSceneObject->ConfigureChildMeshAccelerationStructure([](AccelerationStructure* genericAccelerator) {
            BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
            accelerator->SetMaximumChildren(2);
            accelerator->SetNodesOnLeaves(2);
        });


        newScene->AddSceneObject(cubeSceneObject);
    }

    // Lights
    std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
    pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));

/*
// Assignment 6 Part 1 TODO: Change the '1' here.
#define ACCELERATION_TYPE 2
#if ACCELERATION_TYPE == 0
    newScene->GenerateAccelerationData(AccelerationTypes::NONE);
#elif ACCELERATION_TYPE == 1
    newScene->GenerateAccelerationData(AccelerationTypes::BVH);
#else
    UniformGridAcceleration* accelerator = dynamic_cast<UniformGridAcceleration*>(newScene->GenerateAccelerationData(AccelerationTypes::UNIFORM_GRID));
    assert(accelerator);
    // Assignment 6 Part 2 TODO: Change the glm::ivec3(10, 10, 10) here.
    accelerator->SetSuggestedGridSize(glm::ivec3(3, 3, 3));
#endif
*/
	AccelerationTypes accType = GetAcceleratingStructureType();
	AccelerationStructure* accStructure = newScene->GenerateAccelerationData(accType);
	if (accType == AccelerationTypes::UNIFORM_GRID)
	{
		UniformGridAcceleration* accelerator = dynamic_cast<UniformGridAcceleration*>(accStructure);
		assert(accelerator);
		// Assignment 6 Part 2 TODO: Change the glm::ivec3(10, 10, 10) here.
		accelerator->SetSuggestedGridSize(glm::ivec3(7, 7, 7));
	}

    newScene->AddLight(pointLight);

    return newScene;

}
std::shared_ptr<ColorSampler> Assignment6::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    jitter->SetGridSize(GetGridSize());
    return jitter;
}

std::shared_ptr<class Renderer> Assignment6::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
    return std::make_shared<BackwardRenderer>(scene, sampler);
}

bool Assignment6::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}
/*
int Assignment6::GetMaxReflectionBounces() const
{
    return 2;
}

int Assignment6::GetMaxRefractionBounces() const
{
    return 4;
}
*/