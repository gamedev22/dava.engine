#include "Physics/CharacterControllerComponent.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/Private/PhysicsMath.h"

#include <Scene3D/Scene.h>
#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

ENUM_DECLARE(DAVA::CharacterControllerComponent::MovementMode)
{
    ENUM_ADD_DESCR(DAVA::CharacterControllerComponent::MovementMode::Flying, "Flying");
    ENUM_ADD_DESCR(DAVA::CharacterControllerComponent::MovementMode::Walking, "Walking");
}

namespace DAVA
{
void CharacterControllerComponent::SetContactOffset(float32 value)
{
    contactOffset = value;
}

float32 CharacterControllerComponent::GetContactOffset() const
{
    return contactOffset;
}

void CharacterControllerComponent::SetScaleCoeff(float32 value)
{
    DVASSERT(value < 1.0f);
    scaleCoeff = value;
}

float32 CharacterControllerComponent::GetScaleCoeff() const
{
    return scaleCoeff;
}

void CharacterControllerComponent::SetMovementMode(MovementMode newMode)
{
    mode = newMode;
}

CharacterControllerComponent::MovementMode CharacterControllerComponent::GetMovementMode() const
{
    return mode;
}

void CharacterControllerComponent::Move(const Vector3& displacement)
{
    totalDisplacement += displacement;
}

void CharacterControllerComponent::Teleport(const Vector3& worldPosition)
{
    teleported = true;
    teleportDestination = worldPosition;
    ScheduleUpdate();
}

bool CharacterControllerComponent::IsGrounded() const
{
    return grounded;
}

void CharacterControllerComponent::SetTypeMask(uint32 value)
{
    if (value != typeMask)
    {
        typeMask = value;
        ScheduleUpdate();
    }
}

uint32 CharacterControllerComponent::GetTypeMask() const
{
    return typeMask;
}

void CharacterControllerComponent::SetTypeMaskToCollideWith(uint32 value)
{
    if (typeMaskToCollideWith != value)
    {
        typeMaskToCollideWith = value;
        ScheduleUpdate();
    }
}

uint32 CharacterControllerComponent::GetTypeMaskToCollideWith() const
{
    return typeMaskToCollideWith;
}

void CharacterControllerComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetUInt32("mode", static_cast<uint32>(mode));
}

void CharacterControllerComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    mode = static_cast<MovementMode>(archive->GetUInt32("mode", static_cast<uint32>(mode)));
}

void CharacterControllerComponent::ScheduleUpdate()
{
    if (controller != nullptr)
    {
        Entity* entity = GetEntity();
        DVASSERT(entity != nullptr);

        Scene* scene = entity->GetScene();
        DVASSERT(scene != nullptr);

        scene->physicsSystem->ScheduleUpdate(this);
    }
}

void CharacterControllerComponent::CopyFieldsToComponent(CharacterControllerComponent* dest)
{
    dest->mode = this->mode;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CharacterControllerComponent)
{
    ReflectionRegistrator<CharacterControllerComponent>::Begin()
    .Field("Contact offset", &CharacterControllerComponent::GetContactOffset, &CharacterControllerComponent::SetContactOffset)
    .Field("Scale coefficient", &CharacterControllerComponent::GetScaleCoeff, &CharacterControllerComponent::SetScaleCoeff)
    .Field("Movement mode", &CharacterControllerComponent::GetMovementMode, &CharacterControllerComponent::SetMovementMode)[M::EnumT<MovementMode>()]
    .End();
}

} // namespace DAVA
