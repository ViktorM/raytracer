#pragma once

#include "common/Application.h"

class Assignment8 : public Application
{
public:
    virtual std::shared_ptr<class Camera> CreateCamera() const override;
    virtual std::shared_ptr<class Scene> CreateSphereScene() const;
	virtual std::shared_ptr<class Scene> CreateScene() const override;
	virtual std::shared_ptr<class Scene> CreateScene2() const;
	virtual std::shared_ptr<class Scene> CreateScene3() const;
	virtual std::shared_ptr<class Scene> CreateScene4() const;
	virtual std::shared_ptr<class Scene> CreateScene5() const;
    virtual std::shared_ptr<class ColorSampler> CreateSampler() const override;
    virtual std::shared_ptr<class Renderer> CreateRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler) const override;
    virtual bool NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex) override;
};
