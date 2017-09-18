#include "Scene3D/SkeletonAnimation/JointTransform.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Renderer.h"

namespace DAVA
{
SkinnedMesh::SkinnedMesh()
    : jointTargetsDataMap(16)
{
    type = TYPE_SKINNED_MESH;
    flags |= RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER;
}

RenderObject* SkinnedMesh::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<SkinnedMesh>(this), "Can clone only SkinnedMesh");
        newObject = new SkinnedMesh();
    }
    RenderObject::Clone(newObject);

    SkinnedMesh* mesh = static_cast<SkinnedMesh*>(newObject);
    uint32 batchCount = mesh->GetRenderBatchCount();
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        RenderBatch* batch = mesh->GetRenderBatch(ri);
        RenderBatch* batch0 = GetRenderBatch(ri);

        mesh->SetJointTargets(batch, GetJointTargets(batch0));
    }

    return newObject;
}

void SkinnedMesh::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    uint32 rbCount = GetRenderBatchCount();
    for (uint32 ri = 0; ri < rbCount; ++ri)
    {
        RenderBatch* batch = GetRenderBatch(ri);

        auto found = jointTargetsDataMap.find(batch);
        if (found != jointTargetsDataMap.end())
        {
            uint32 dataIndex = found->second;
            const JointTargets& targets = jointTargetsData[dataIndex].first;

            uint32 targetsCount = uint32(targets.size());
            archive->SetUInt32(Format("skinnedObject.batch%d.targetsCount", ri), targetsCount);
            archive->SetByteArray(Format("skinnedObject.batch%d.targetsData", ri), reinterpret_cast<const uint8*>(targets.data()), targetsCount * sizeof(int32));
        }
    }

    RenderObject::Save(archive, serializationContext);
}

void SkinnedMesh::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    RenderObject::Load(archive, serializationContext);

    uint32 rbCount = GetRenderBatchCount();
    for (uint32 ri = 0; ri < rbCount; ++ri)
    {
        RenderBatch* batch = GetRenderBatch(ri);

        String targetsCountKey = Format("skinnedObject.batch%d.targetsCount", ri);
        if (archive->IsKeyExists(targetsCountKey))
        {
            uint32 targetsCount = archive->GetUInt32(targetsCountKey);
            JointTargets targets(targetsCount);
            const uint8* targetsData = archive->GetByteArray(Format("skinnedObject.batch%d.targetsData", ri));
            if (targetsData != nullptr)
                Memcpy(targets.data(), targetsData, targetsCount * sizeof(int32));

            SetJointTargets(batch, targets);
        }
    }
}

void SkinnedMesh::RecalcBoundingBox()
{
    AABBox3 geometryBBox;

    for (const RenderBatchWithOptions& i : renderBatchArray)
    {
        RenderBatch* batch = i.renderBatch;
        geometryBBox.AddAABBox(batch->GetBoundingBox());
    }

    float32 radius = geometryBBox.GetBoundingSphereRadius();
    bbox = AABBox3(geometryBBox.GetCenter(), 2.f * radius);
}

void SkinnedMesh::BindDynamicParameters(Camera* camera, RenderBatch* batch)
{
    auto found = jointTargetsDataMap.find(batch);
    if (found != jointTargetsDataMap.end())
    {
        const JointTargetsData& data = jointTargetsData[found->second].second;

        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINTS_COUNT, &data.jointsDataCount, reinterpret_cast<pointer_size>(&data.jointsDataCount));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_POSITIONS, data.positions.data(), reinterpret_cast<pointer_size>(data.positions.data()));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_QUATERNIONS, data.quaternions.data(), reinterpret_cast<pointer_size>(data.quaternions.data()));
    }

    RenderObject::BindDynamicParameters(camera, batch);
}

void SkinnedMesh::PrepareToRender(Camera* camera)
{
    if (!jointTargetsDataMap.empty() && skeletonFinalJointTransforms)
    {
        jointTargetsDataToPrepare.clear();
        for (RenderBatch* b : activeRenderBatchArray)
        {
            auto found = jointTargetsDataMap.find(b);
            if (found != jointTargetsDataMap.end())
                jointTargetsDataToPrepare.insert(found->second);
        }

        for (uint32 dataIndex : jointTargetsDataToPrepare)
            PrepareJointTargetsData(dataIndex);
    }
}

void SkinnedMesh::PrepareJointTargetsData(uint32 dataIndex)
{
    DVASSERT(dataIndex < uint32(jointTargetsData.size()));

    const JointTargets& targets = jointTargetsData[dataIndex].first;
    JointTargetsData& data = jointTargetsData[dataIndex].second;
    for (uint32 j = 0; j < data.jointsDataCount; ++j)
    {
        uint32 transformIndex = targets[j];
        DVASSERT(transformIndex < skeletonJointCount);

        const JointTransform& finalTransform = skeletonFinalJointTransforms[transformIndex];
        data.positions[j] = Vector4(Vector3(finalTransform.GetPosition().data), finalTransform.GetScale());
        data.quaternions[j] = Vector4(finalTransform.GetOrientation().data);
    }
}

void SkinnedMesh::SetJointTargets(RenderBatch* batch, const JointTargets& targets)
{
    DVASSERT(uint32(targets.size()) <= MAX_TARGET_JOINTS);

    auto found = std::find_if(jointTargetsData.begin(), jointTargetsData.end(), [&targets](const std::pair<JointTargets, JointTargetsData>& item) {
        return (item.first == targets);
    });

    if (found != jointTargetsData.end())
    {
        jointTargetsDataMap[batch] = uint32(std::distance(jointTargetsData.begin(), found));
    }
    else
    {
        uint32 dataIndex = uint32(jointTargetsData.size());
        jointTargetsData.emplace_back();

        uint32 targetsCount = uint32(targets.size());
        jointTargetsData.back().first = targets;
        jointTargetsData.back().second.positions.resize(targetsCount);
        jointTargetsData.back().second.quaternions.resize(targetsCount);
        jointTargetsData.back().second.jointsDataCount = targetsCount;

        jointTargetsDataMap[batch] = dataIndex;
    }
}

SkinnedMesh::JointTargets SkinnedMesh::GetJointTargets(RenderBatch* batch)
{
    auto found = jointTargetsDataMap.find(batch);
    if (found != jointTargetsDataMap.end())
        return jointTargetsData[found->second].first;

    return JointTargets();
}

SkinnedMesh::JointTargetsData SkinnedMesh::GetJointTargetsData(RenderBatch* batch)
{
    auto found = jointTargetsDataMap.find(batch);
    if (found != jointTargetsDataMap.end())
    {
        uint32 dataIndex = found->second;
        PrepareJointTargetsData(dataIndex);

        return jointTargetsData[dataIndex].second;
    }

    return SkinnedMesh::JointTargetsData();
}
}
