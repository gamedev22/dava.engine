#pragma once

#include "Base/FastName.h"
#include "Scene3D/Systems/ParticleEffectDebugDrawSystem.h"

#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Render/Highlevel/RenderPass.h"

namespace DAVA
{
class RenderBatch;
class RenderSystem;
class NMaterial;
class Texture;

class ParticleDebugRenderPass : public RenderPass
{
public:
    struct ParticleDebugRenderPassConfig
    {
        const FastName& name;
        RenderSystem* renderSystem;
        NMaterial* wireframeMaterial;
        NMaterial* overdrawMaterial;
        NMaterial* showAlphaMaterial;
        const eParticleDebugDrawMode& drawMode;
        const bool& drawOnlySelected;
        UnorderedSet<RenderObject*>* selectedParticles;
    };

    ParticleDebugRenderPass(ParticleDebugRenderPassConfig config);
    ~ParticleDebugRenderPass();

    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;
    Asset<Texture> GetTexture() const;

    static const FastName PASS_DEBUG_DRAW_PARTICLES;

private:
    void DrawBatches(Camera* camera);
    void PrepareParticlesVisibilityArray(Camera* camera, RenderSystem* renderSystem);
    void PrepareParticlesBatchesArray(const Vector<RenderObject*> objectsArray, Camera* camera);
    void MakePacket(Camera* camera);
    NMaterial* SelectMaterial(RenderBatch* batch);

    Asset<Texture> debugTexture;
    NMaterial* wireframeMaterial;
    NMaterial* overdrawMaterial;
    NMaterial* showAlphaMaterial;
    RenderBatchArray particleBatches;
    UnorderedSet<RenderObject*>* selectedParticles;
    const eParticleDebugDrawMode& drawMode;
    const bool& drawOnlySelected;
};
}