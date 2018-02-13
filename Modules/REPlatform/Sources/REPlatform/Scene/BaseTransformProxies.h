#pragma once

#include "REPlatform/DataNodes/Selectable.h"

namespace DAVA
{
class ParticleEmitterInstance;
class ParticleForce;
class EntityTransformProxy : public Selectable::TransformProxy
{
public:
    const Matrix4& GetWorldTransform(const Any& object) override;
    Matrix4 GetLocalTransform(const Any& object) override;
    void SetLocalTransform(Any& object, const Matrix4& matrix) override;
    bool SupportsTransformType(const Any& object, Selectable::TransformType) const override;
    bool TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const override;
};

class EmitterTransformProxy : public Selectable::TransformProxy
{
public:
    const Matrix4& GetWorldTransform(const Any& object) override;
    Matrix4 GetLocalTransform(const Any& object) override;
    void SetLocalTransform(Any& object, const Matrix4& matrix) override;
    bool SupportsTransformType(const Any& object, Selectable::TransformType) const override;
    bool TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const override;
};

class ParticleForceTransformProxy : public Selectable::TransformProxy
{
public:
    const Matrix4& GetWorldTransform(const Any& object) override;
    Matrix4 GetLocalTransform(const Any& object) override;
    void SetLocalTransform(Any& object, const Matrix4& matrix) override;
    bool SupportsTransformType(const Any& object, Selectable::TransformType) const override;
    bool TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const override;

private:
    ParticleEmitterInstance* GetEmitterInstance(ParticleForce* force) const;
};
} // namespace DAVA
