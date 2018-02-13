#include "REPlatform/Scene/Systems/EditorParticlesSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"

#include "REPlatform/Commands/ParticleEditorCommands.h"
#include "REPlatform/Commands/ParticleLayerCommands.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"

#include <Base/BaseTypes.h>
#include <Entity/Component.h>
#include <Entity/ComponentUtils.h>
#include <Particles/ParticleEmitter.h>
#include <Particles/ParticleEmitterInstance.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/RenderComponent.h>

namespace DAVA
{
namespace EditorParticlesSystemDetails
{
template <typename T>
inline const Vector<T*>& GetForceVector(T* force, ParticleLayer* layer);

template <>
inline const Vector<ParticleForce*>& GetForceVector(ParticleForce* force, ParticleLayer* layer)
{
    return layer->GetParticleForces();
}

template <>
inline const Vector<ParticleForceSimplified*>& GetForceVector(ParticleForceSimplified* force, ParticleLayer* layer)
{
    return layer->GetSimplifiedParticleForces();
}
} // namespace EditorParticlesSystemDetails

EditorParticlesSystem::EditorParticlesSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<ParticleEffectComponent>())
{
}

void EditorParticlesSystem::DrawDebugInfoForEffect(Entity* effectEntity)
{
    DVASSERT(effectEntity != nullptr);

    Scene* scene = GetScene();
    SceneCollisionSystem* collisionSystem = scene->GetSystem<SceneCollisionSystem>();

    AABBox3 worldBox;
    AABBox3 collBox = collisionSystem->GetBoundingBox(effectEntity);
    DVASSERT(!collBox.IsEmpty());
    collBox.GetTransformedBox(effectEntity->GetWorldTransform(), worldBox);
    float32 radius = (collBox.max - collBox.min).Length() / 3;
    scene->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(worldBox.GetCenter(), radius, Color(0.9f, 0.9f, 0.9f, 0.35f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawEmitter(ParticleEmitterInstance* emitter, Entity* owner, bool selected)
{
    DVASSERT((emitter != nullptr) && (owner != nullptr));

    Scene* scene = GetScene();
    SceneCollisionSystem* collisionSystem = scene->GetSystem<SceneCollisionSystem>();

    Vector3 center = emitter->GetSpawnPosition();
    TransformPerserveLength(center, Matrix3(owner->GetWorldTransform()));
    center += owner->GetWorldTransform().GetTranslationVector();

    AABBox3 boundingBox = collisionSystem->GetBoundingBox(owner);
    DVASSERT(!boundingBox.IsEmpty());
    float32 radius = (boundingBox.max - boundingBox.min).Length() / 3;
    scene->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(center, radius, Color(1.0f, 1.0f, 1.0f, 0.5f), RenderHelper::DRAW_SOLID_DEPTH);

    if (selected)
    {
        DrawVectorArrow(emitter, center);

        switch (emitter->GetEmitter()->emitterType)
        {
        case ParticleEmitter::EMITTER_ONCIRCLE_VOLUME:
        case ParticleEmitter::EMITTER_ONCIRCLE_EDGES:
        {
            DrawSizeCircle(owner, emitter);
            break;
        }
        case ParticleEmitter::EMITTER_SHOCKWAVE:
        {
            DrawSizeCircleShockWave(owner, emitter);
            break;
        }

        case ParticleEmitter::EMITTER_RECT:
        {
            DrawSizeBox(owner, emitter);
            break;
        }

        default:
            break;
        }
    }
}

void EditorParticlesSystem::Draw()
{
    const SelectableGroup& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection();
    Set<ParticleEmitterInstance*> selectedEmitterInstances;
    for (auto instance : selection.ObjectsOfType<ParticleEmitterInstance>())
    {
        selectedEmitterInstances.insert(instance);
    }

    for (ParticleForce* force : selection.ObjectsOfType<ParticleForce>())
        DrawParticleForces(force);

    for (auto entity : entities)
    {
        auto effect = entity->GetComponent<ParticleEffectComponent>();
        if (effect != nullptr)
        {
            for (uint32 i = 0, e = effect->GetEmittersCount(); i < e; ++i)
            {
                auto instance = effect->GetEmitterInstance(i);
                DrawEmitter(instance, entity, selectedEmitterInstances.count(instance) > 0);
            }
            DrawDebugInfoForEffect(entity);
        }
    }
}

void EditorParticlesSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandTypes<CommandChangeLayerMaterialProperties,
                                              CommandChangeFlowProperties,
                                              CommandChangeNoiseProperties,
                                              CommandChangeFresnelToAlphaProperties,
                                              CommandChangeParticlesStripeProperties,
                                              CommandChangeAlphaRemapProperties,
                                              CommandUpdateParticleLayerBase>())
    {
        RestartParticleEffects();
    }
}

void EditorParticlesSystem::DrawSizeCircleShockWave(Entity* effectEntity, ParticleEmitterInstance* emitter)
{
    float32 time = GetEffectComponent(effectEntity)->GetCurrTime();
    float32 emitterRadius = (emitter->GetEmitter()->radius) ? emitter->GetEmitter()->radius->GetValue(time) : 0.0f;

    Vector3 emissionVector(0.0f, 0.0f, 1.0f);

    if (emitter->GetEmitter()->emissionVector)
    {
        Matrix4 wMat = effectEntity->GetWorldTransform();
        wMat.SetTranslationVector(Vector3(0.0f, 0.0f, 0.0f));
        emissionVector = emitter->GetEmitter()->emissionVector->GetValue(time) * wMat;
    }

    auto center = Selectable(emitter).GetWorldTransform().GetTranslationVector();

    auto drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawCircle(center, emissionVector, emitterRadius, 12, Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeCircle(Entity* effectEntity, ParticleEmitterInstance* emitter)
{
    float32 emitterRadius = 0.0f;
    Vector3 emitterVector;
    float32 time = GetEffectComponent(effectEntity)->GetCurrTime();

    if (emitter->GetEmitter()->radius)
    {
        emitterRadius = emitter->GetEmitter()->radius->GetValue(time);
    }

    if (emitter->GetEmitter()->emissionVector)
    {
        Matrix4 wMat = effectEntity->GetWorldTransform();
        wMat.SetTranslationVector(Vector3(0.0f, 0.0f, 0.0f));
        emitterVector = emitter->GetEmitter()->emissionVector->GetValue(time) * wMat;
    }

    auto center = Selectable(emitter).GetWorldTransform().GetTranslationVector();

    auto drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawCircle(center, emitterVector, emitterRadius, 12,
                       Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeBox(Entity* effectEntity, ParticleEmitterInstance* emitter)
{
    // Default value of emitter size
    Vector3 emitterSize;

    float32 time = GetEffectComponent(effectEntity)->GetCurrTime();

    if (emitter->GetEmitter()->size)
    {
        emitterSize = emitter->GetEmitter()->size->GetValue(time);
    }

    Matrix4 wMat = effectEntity->GetWorldTransform();

    wMat.SetTranslationVector(Selectable(emitter).GetWorldTransform().GetTranslationVector());

    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawAABoxTransformed(AABBox3(-0.5f * emitterSize, 0.5f * emitterSize), wMat,
                                 Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawVectorArrow(ParticleEmitterInstance* emitter, Vector3 center)
{
    ParticleEffectComponent* effect = emitter->GetOwner();
    if (effect == nullptr)
        return;

    Vector3 emitterVector(0.0f, 0.0f, 1.0f);
    if (emitter->GetEmitter()->emissionVector)
    {
        emitterVector = emitter->GetEmitter()->emissionVector->GetValue(effect->GetCurrTime());
        emitterVector.Normalize();
    }
    Vector3 emissionVelocityVector(0.0f, 0.0f, 1.0f);
    if (emitter->GetEmitter()->emissionVelocityVector)
    {
        emissionVelocityVector = emitter->GetEmitter()->emissionVelocityVector->GetValue(effect->GetCurrTime());
        float32 sqrLen = emissionVelocityVector.SquareLength();
        if (sqrLen > EPSILON)
            emissionVelocityVector /= std::sqrt(sqrLen);
    }

    float32 scale = 1.0f;
    HoodSystem* hoodSystem = GetScene()->GetSystem<HoodSystem>();
    if (hoodSystem != nullptr)
    {
        scale = hoodSystem->GetScale();
    }

    float32 arrowSize = scale;
    float32 arrowBaseSize = 5.0f;
    emitterVector = (emitterVector * arrowBaseSize * scale);

    Matrix4 wMat = effect->GetEntity()->GetWorldTransform();
    wMat.SetTranslationVector(Vector3(0, 0, 0));
    TransformPerserveLength(emitterVector, wMat);

    emissionVelocityVector *= arrowBaseSize * scale;
    TransformPerserveLength(emissionVelocityVector, wMat);

    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(center, center + emitterVector, arrowSize,
                                                               Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);

    if (emitter->GetEmitter()->emissionVelocityVector)
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(center, center + emissionVelocityVector, arrowSize,
                                                                   Color(0.0f, 0.0f, 0.7f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawParticleForces(ParticleForce* force)
{
    using ForceType = ParticleForce::eType;

    if (force->type == ForceType::GRAVITY)
        return;

    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    ParticleLayer* layer = GetForceOwner(force);
    if (layer == nullptr)
        return;

    ParticleEmitterInstance* emitterInstance = GetRootEmitterLayerOwner(layer);
    ParticleEffectComponent* effectComponent = emitterInstance->GetOwner();
    Entity* entity = effectComponent->GetEntity();
    if (force->type == ForceType::VORTEX || force->type == ForceType::WIND || force->type == ForceType::PLANE_COLLISION)
    {
        float32 scale = 1.0f;
        HoodSystem* hoodSystem = GetScene()->GetSystem<HoodSystem>();
        if (hoodSystem != nullptr)
            scale = hoodSystem->GetScale();

        float32 arrowSize = scale;
        float32 arrowBaseSize = 5.0f;
        Vector3 emitterVector = force->direction;
        Vector3 center;
        if (force->worldAlign)
        {
            center = entity->GetWorldTransform().GetTranslationVector() + force->position;
        }
        else
        {
            Matrix4 wMat = entity->GetWorldTransform();
            emitterVector = emitterVector * Matrix3(wMat);
            center = force->position * wMat;
        }
        emitterVector.Normalize();
        emitterVector *= arrowBaseSize * scale;

        drawer->DrawArrow(center, center + emitterVector, arrowSize, Color(0.7f, 0.7f, 0.0f, 0.35f), RenderHelper::DRAW_SOLID_DEPTH);
    }

    if (force->type == ForceType::PLANE_COLLISION)
    {
        Matrix4 wMat = entity->GetWorldTransform();
        Vector3 forcePosition;
        Vector3 wNormal;
        if (force->worldAlign)
        {
            forcePosition = wMat.GetTranslationVector() + force->position;
            wNormal = force->direction;
        }
        else
        {
            wNormal = force->direction * Matrix3(wMat);
            forcePosition = force->position;
            forcePosition = force->position * wMat;
        }
        wNormal.Normalize();
        Vector3 cV(0.0f, 0.0f, 1.0f);
        if (1.0f - Abs(cV.DotProduct(wNormal)) < EPSILON)
            cV = Vector3(1.0f, 0.0f, 0.0f);
        Matrix4 transform;
        transform.BuildLookAtMatrix(forcePosition, forcePosition + wNormal, cV);
        transform.Inverse();

        float32 bbsize = force->planeScale * 0.5f;
        drawer->DrawAABoxTransformed(AABBox3(Vector3(-bbsize, -bbsize, -0.1f), Vector3(bbsize, bbsize, 0.01f)), transform,
                                     Color(0.0f, 0.7f, 0.7f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
        drawer->DrawAABoxTransformed(AABBox3(Vector3(-bbsize, -bbsize, -0.1f), Vector3(bbsize, bbsize, 0.01f)), transform,
                                     Color(0.0f, 0.35f, 0.35f, 0.35f), RenderHelper::DRAW_WIRE_DEPTH);
    }
    else if (force->type == ForceType::POINT_GRAVITY)
    {
        float32 radius = force->pointGravityRadius;
        Vector3 translation;
        if (force->worldAlign)
            translation = entity->GetWorldTransform().GetTranslationVector() + force->position;
        else
            translation = Selectable(force).GetWorldTransform().GetTranslationVector();

        drawer->DrawIcosahedron(translation, radius, Color(0.0f, 0.3f, 0.7f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
        drawer->DrawIcosahedron(translation, radius, Color(0.0f, 0.15f, 0.35f, 0.35f), RenderHelper::DRAW_WIRE_DEPTH);
    }

    if (force->isInfinityRange)
        return;
    if (force->GetShape() == ParticleForce::eShape::BOX)
    {
        Matrix4 wMat = entity->GetWorldTransform();
        if (force->worldAlign)
        {
            Vector3 translation = wMat.GetTranslationVector();
            wMat = Matrix4::IDENTITY;
            wMat.SetTranslationVector(translation + force->position);
        }
        else
            wMat.SetTranslationVector(Selectable(force).GetWorldTransform().GetTranslationVector());

        drawer->DrawAABoxTransformed(AABBox3(-force->GetHalfBoxSize(), force->GetHalfBoxSize()), wMat,
                                     Color(0.0f, 0.7f, 0.3f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);

        drawer->DrawAABoxTransformed(AABBox3(-force->GetHalfBoxSize(), force->GetHalfBoxSize()), wMat,
                                     Color(0.0f, 0.35f, 0.15f, 0.35f), RenderHelper::DRAW_WIRE_DEPTH);
    }
    else if (force->GetShape() == ParticleForce::eShape::SPHERE)
    {
        Matrix4 wMat = Selectable(force).GetWorldTransform();
        if (force->worldAlign)
        {
            Vector3 translation = entity->GetWorldTransform().GetTranslationVector();
            wMat = Matrix4::IDENTITY;
            wMat.SetTranslationVector(translation + force->position);
        }
        float32 radius = force->GetRadius();
        drawer->DrawIcosahedron(wMat.GetTranslationVector(), radius, Color(0.0f, 0.7f, 0.3f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
        drawer->DrawIcosahedron(wMat.GetTranslationVector(), radius, Color(0.0f, 0.35f, 0.15f, 0.35f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void EditorParticlesSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);
}

void EditorParticlesSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(entities, entity);
}

void EditorParticlesSystem::PrepareForRemove()
{
    entities.clear();
}

void EditorParticlesSystem::RestartParticleEffects()
{
    for (Entity* entity : entities)
    {
        ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        DVASSERT(effectComponent);
        if (!effectComponent->IsStopped())
        {
            effectComponent->Restart();
        }
    }
}

ParticleEffectComponent* EditorParticlesSystem::GetEmitterOwner(ParticleEmitterInstance* wantedEmitter) const
{
    ParticleEffectComponent* component = wantedEmitter->GetOwner();
    if (component != nullptr)
    {
        return component;
    }

    using TFunctor = Function<bool(ParticleEmitterInstance*)>;
    TFunctor lookUpEmitter = [&lookUpEmitter, &wantedEmitter](ParticleEmitterInstance* emitter) {
        if (emitter == wantedEmitter)
        {
            return true;
        }

        for (ParticleLayer* layer : emitter->GetEmitter()->layers)
        {
            if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                DVASSERT(layer->innerEmitter != nullptr);
                if (lookUpEmitter(layer->innerEmitter) == true)
                {
                    return true;
                }
            }
        }

        return false;
    };

    for (Entity* entity : entities)
    {
        ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        uint32 emittersCount = effectComponent->GetEmittersCount();
        for (uint32 id = 0; id < emittersCount; ++id)
        {
            ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            bool found = lookUpEmitter(emitterInstance);
            if (found == true)
            {
                return effectComponent;
            }
        }
    }

    return nullptr;
}

template <typename T>
ParticleLayer* EditorParticlesSystem::GetForceOwner(T* force) const
{
    Function<ParticleLayer*(ParticleEmitterInstance*, T*)> getForceOwner = [&getForceOwner](ParticleEmitterInstance* emitter, T* force) -> ParticleLayer*
    {
        for (ParticleLayer* layer : emitter->GetEmitter()->layers)
        {
            if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                DVASSERT(layer->innerEmitter != nullptr);
                ParticleLayer* foundLayer = getForceOwner(layer->innerEmitter, force);
                if (foundLayer != nullptr)
                {
                    return foundLayer;
                }
            }
            const Vector<T*>& forces = EditorParticlesSystemDetails::GetForceVector(force, layer);
            if (std::find(forces.begin(), forces.end(), force) != forces.end())
            {
                return layer;
            }
        }

        return nullptr;
    };

    for (Entity* entity : entities)
    {
        ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        uint32 emittersCount = effectComponent->GetEmittersCount();
        for (uint32 id = 0; id < emittersCount; ++id)
        {
            ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            ParticleLayer* owner = getForceOwner(emitterInstance, force);
            if (owner != nullptr)
            {
                return owner;
            }
        }
    }

    return nullptr;
}

ParticleEmitterInstance* EditorParticlesSystem::GetRootEmitterLayerOwner(ParticleLayer* layer) const
{
    Function<bool(ParticleEmitterInstance*, ParticleLayer*)> hasLayerOwner = [&hasLayerOwner](ParticleEmitterInstance* emitter, ParticleLayer* layer) -> bool
    {
        for (ParticleLayer* l : emitter->GetEmitter()->layers)
        {
            if (l == layer)
            {
                return true;
            }

            if (l->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                DVASSERT(l->innerEmitter != nullptr);
                bool found = hasLayerOwner(l->innerEmitter, layer);
                if (found)
                {
                    return true;
                }
            }
        }

        return false;
    };

    for (Entity* entity : entities)
    {
        ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        uint32 emittersCount = effectComponent->GetEmittersCount();
        for (uint32 id = 0; id < emittersCount; ++id)
        {
            ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            if (hasLayerOwner(emitterInstance, layer))
            {
                return emitterInstance;
            }
        }
    }

    return nullptr;
}

ParticleEmitterInstance* EditorParticlesSystem::GetDirectEmitterLayerOwner(ParticleLayer* layer) const
{
    using TFunctor = Function<ParticleEmitterInstance*(ParticleEmitterInstance*, ParticleLayer*)>;
    TFunctor lookUpEmitter = [&lookUpEmitter](ParticleEmitterInstance* emitter, ParticleLayer* layer) -> ParticleEmitterInstance* {
        for (ParticleLayer* l : emitter->GetEmitter()->layers)
        {
            if (l == layer)
            {
                return emitter;
            }

            if (l->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                DVASSERT(l->innerEmitter != nullptr);
                ParticleEmitterInstance* result = lookUpEmitter(l->innerEmitter, layer);
                if (result != nullptr)
                {
                    return result;
                }
            }
        }

        return nullptr;
    };

    for (Entity* entity : entities)
    {
        ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        uint32 emittersCount = effectComponent->GetEmittersCount();
        for (uint32 id = 0; id < emittersCount; ++id)
        {
            ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            ParticleEmitterInstance* wantedEmitter = lookUpEmitter(emitterInstance, layer);

            if (wantedEmitter != nullptr)
            {
                return wantedEmitter;
            }
        }
    }

    return nullptr;
}

template ParticleLayer* EditorParticlesSystem::GetForceOwner<ParticleForce>(ParticleForce* force) const;
template ParticleLayer* EditorParticlesSystem::GetForceOwner<ParticleForceSimplified>(ParticleForceSimplified* force) const;
} // namespace DAVA
