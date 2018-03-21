#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Scene3D/Prefab.h"
#include "Asset/AssetListener.h"

namespace DAVA
{
class PrefabComponent : public Component, private AssetListener
{
protected:
    virtual ~PrefabComponent();

public:
    PrefabComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetFilepath(const FilePath& filepath);
    const FilePath& GetFilepath() const;

    const Asset<Prefab>& GetPrefab() const;

    void OnAssetReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded) override;

private:
    Asset<Prefab> prefab = nullptr;
    FilePath filepath;
    DAVA_VIRTUAL_REFLECTION(PrefabComponent, Component);
};
}
