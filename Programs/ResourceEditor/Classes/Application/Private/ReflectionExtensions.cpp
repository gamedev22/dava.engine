#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/PropertyPanel/RenderObjectExtensions.h"
#include "Classes/PropertyPanel/NMaterialExtensions.h"
#include "Classes/PropertyPanel/FilePathExtensions.h"
#include "Classes/PropertyPanel/ComponentExtensions.h"
#include "Classes/SlotSupportModule/Private/SlotComponentExtensions.h"

#include <REPlatform/Global/REMeta.h>
#include <REPlatform/DataNodes/SceneData.h>

#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/CommonFieldNames.h>
#include <TArc/Utils/ReflectionHelpers.h>

#include <Entity/Component.h>
#include <Scene3D/Components/ActionComponent.h>
#include <Scene3D/Components/GeoDecalComponent.h>
#include <Scene3D/Components/LightComponent.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/Components/PostEffectComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/ReflectionComponent.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Scene3D/Components/SoundComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/WaveComponent.h>
#include <Scene3D/Components/ReflectionComponent.h>
#include <Scene3D/Components/MeshComponent.h>
#include <Scene3D/Components/PrefabComponent.h>

#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/Vegetation/VegetationRenderObject.h>
#include <Render/Material/NMaterial.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedStructure.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>

namespace ReflectoinExtensionsDetail
{
using namespace DAVA;

void RegisterRenderComponentExtensions()
{
    EmplaceTypeMeta<RenderObject>(CreateRenderObjectCommandProducer());
    EmplaceTypeMeta<RenderBatch>(CreateRenderBatchCommandProducer());
    EmplaceTypeMeta<NMaterial>(CreateNMaterialCommandProducer());
    EmplaceTypeMeta<MeshComponent>(CreateMeshComponentCommandProducer());
}

void RegisterFilePathExtensions(DAVA::ContextAccessor* accessor)
{
    // HeightMap
    InitFilePathExtensions(accessor);
    EmplaceFieldMeta<Landscape>(DAVA::FastName("heightmapPath"), CreateHeightMapValidator());
    EmplaceFieldMeta<Landscape>(DAVA::FastName("heightmapPath"), CreateHeightMapFileMeta());
    EmplaceFieldMeta<VegetationRenderObject>(DAVA::FastName("lightmap"), CreateTextureValidator());
    EmplaceFieldMeta<VegetationRenderObject>(DAVA::FastName("lightmap"), CreateTextureFileMeta());
    EmplaceFieldMeta<VegetationRenderObject>(DAVA::FastName("customGeometry"), CreateSceneValidator());
    EmplaceFieldMeta<VegetationRenderObject>(DAVA::FastName("customGeometry"), CreateSceneFileMeta());
    EmplaceFieldMeta<SlotComponent>(DAVA::SlotComponent::SlotNameFieldName, PropertyPanel::CreateSlotNameCommandProvider());
    EmplaceFieldMeta<SlotComponent>(DAVA::SlotComponent::ConfigPathFieldName, GenericFileMeta<REFileMeta>("All supported formats (*.yaml *.xml);;Yaml (*.yaml);;XML(*.xml)", "Open items list"));
    EmplaceFieldMeta<SlotComponent>(DAVA::SlotComponent::ConfigPathFieldName, CreateExistsFile());
    EmplaceFieldMeta<SlotComponent>(DAVA::SlotComponent::ConfigPathFieldName, PropertyPanel::CreateSlotConfigCommandProvider());
    EmplaceTypeMeta<SlotComponent>(DAVA::M::DisableEntityReparent());
    EmplaceFieldMeta<MotionComponent>(DAVA::FastName("motionPath"), GenericFileMeta<REFileMeta>("All supported formats (*.yaml *.anim);;Yaml (*.yaml);;Animation (*.anim)", "Open Motion file"));
    EmplaceFieldMeta<LightComponent>(DAVA::FastName("EnvironmentMap"), CreateTextureValidator());
    EmplaceFieldMeta<GeoDecalComponent>(DAVA::FastName("decalAlbedo"), CreateTextureValidator());
    EmplaceFieldMeta<GeoDecalComponent>(DAVA::FastName("decalNormal"), CreateTextureValidator());
    EmplaceFieldMeta<GeoDecalComponent>(DAVA::FastName("decalSpecular"), CreateTextureValidator(false));
    EmplaceFieldMeta<PostEffectComponent>(DAVA::FastName("colorGradingTable"), CreateTextureValidator(false));
    EmplaceFieldMeta<PostEffectComponent>(DAVA::FastName("heatmapTable"), CreateTextureValidator(false));
    EmplaceFieldMeta<PostEffectComponent>(DAVA::FastName("lightMeterTable"), CreateTextureValidator(false));
    EmplaceFieldMeta<ReflectionComponent>(DAVA::FastName("reflectionsMap"), CreateTextureValidator(true));

    EmplaceFieldMeta<MeshBatchDescriptor>(DAVA::FastName("material"), GenericFileMeta<REFileMeta>("Material (*.mat)", "Open material"));
    EmplaceFieldMeta<MeshBatchDescriptor>(DAVA::FastName("material"), CreateExistsFile());

    EmplaceFieldMeta<MeshLODDescriptor>(DAVA::FastName("geometry"), GenericFileMeta<REFileMeta>("Geometry (*.geo)", "Open geometry"));
    EmplaceFieldMeta<MeshLODDescriptor>(DAVA::FastName("geometry"), CreateExistsFile());
    EmplaceFieldMeta<PrefabComponent>(DAVA::FastName("filepath"), GenericFileMeta<REFileMeta>("Geometry (*.prefab)", "Open prefab"));
}

void RegisterComponentExtensions(const TypeInheritance::Info& type)
{
    const Type* transformComponent = Type::Instance<TransformComponent>();
    const Type* actionComponent = Type::Instance<ActionComponent>();
    const Type* soundComponent = Type::Instance<SoundComponent>();
    const Type* waveComponent = Type::Instance<WaveComponent>();
    const Type* componentType = Type::Instance<Component>();

    const Vector<TypeInheritance::Info> derivedTypes = type.type->GetInheritance()->GetDerivedTypes();
    for (const TypeInheritance::Info& derived : derivedTypes)
    {
        RegisterComponentExtensions(derived);
    }

    if (type.type == transformComponent)
    {
        return;
    }

    if (type.type->IsAbstract())
    {
        return;
    }

    ReflectedType* refType = const_cast<ReflectedType*>(ReflectedTypeDB::GetByType(type.type));
    if (refType == nullptr)
    {
        DVASSERT(false, "We have a component that derived from DAVA::Component, but without created ReflectedType");
    }

    const ReflectedStructure* structure = refType->GetStructure();
    DVASSERT(structure != nullptr, "Somebody has forgotten to declare reflected structure for component");

    M::CommandProducerHolder holder;
    if (structure->meta == nullptr || structure->meta->GetMeta<M::CantBeDeletedManualyComponent>() == nullptr)
    {
        holder.AddCommandProducer(CreateRemoveComponentProducer());
    }

    if (type.type == actionComponent)
    {
        holder.AddCommandProducer(CreateActionsEditProducer());
    }
    else if (type.type == soundComponent)
    {
        holder.AddCommandProducer(CreateSoundsEditProducer());
    }
    else if (type.type == waveComponent)
    {
        holder.AddCommandProducer(CreateWaveTriggerProducer());
    }
    else if (type.type == Type::Instance<SlotComponent>())
    {
        holder.AddCommandProducer(PropertyPanel::CreateCopySlotProducer());
    }

    EmplaceTypeMeta(refType, std::move(holder));
}

void RegisterComponentsExtensions()
{
    const Type* componentType = Type::Instance<Component>();
    const Vector<TypeInheritance::Info> derivedTypes = componentType->GetInheritance()->GetDerivedTypes();
    for (const TypeInheritance::Info& derived : derivedTypes)
    {
        RegisterComponentExtensions(derived);
    }
}

String GetSceneName(DAVA::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScenePath().GetBasename();
}

String GetScenePath(DAVA::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScenePath().GetAbsolutePathname();
}

bool IsSceneChanged(DAVA::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScene()->IsChanged();
}

void RegisterDataContextExtensions()
{
    using namespace DAVA;
    using namespace DAVA;
    ReflectionRegistrator<DataContext>::Begin()
    .Field(ContextNameFieldName, &GetSceneName, nullptr)
    .Field(ContextToolTipFieldName, &GetScenePath, nullptr)
    .Field(IsContextModifiedFieldName, &IsSceneChanged, nullptr)
    .End();

    ReflectionRegistrator<ContextAccessor>::Begin()
    .Field(MainObjectName, [](ContextAccessor*) { return "Scene"; }, nullptr)
    .End();
}

void RegisterReflectionExtensions(DAVA::ContextAccessor* accessor)
{
    RegisterRenderComponentExtensions();
    RegisterFilePathExtensions(accessor);
    RegisterComponentsExtensions();
    RegisterDataContextExtensions();
}
} // namespace ReflectoinExtensionsDetail

void ReflectionExtensionsModule::PostInit()
{
    using namespace ReflectoinExtensionsDetail;
    RegisterReflectionExtensions(GetAccessor());
}

DAVA_VIRTUAL_REFLECTION_IMPL(ReflectionExtensionsModule)
{
    DAVA::ReflectionRegistrator<ReflectionExtensionsModule>::Begin()
    .ConstructorByPointer()
    .End();
}
