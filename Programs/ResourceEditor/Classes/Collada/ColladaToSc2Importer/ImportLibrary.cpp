#include "Classes/Collada/ColladaPolygonGroup.h"
#include "Classes/Collada/ColladaMeshInstance.h"
#include "Classes/Collada/ColladaSceneNode.h"
#include "Classes/Collada/ColladaScene.h"
#include "Classes/Collada/ColladaToSc2Importer/ImportSettings.h"
#include "Classes/Collada/ColladaToSc2Importer/ImportLibrary.h"

#include <REPlatform/Scene/Utils/RETextureDescriptorUtils.h>

#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FileSystem.h>
#include <Render/3D/MeshUtils.h>
#include <Render/3D/PolygonGroup.h>
#include <Render/Highlevel/Mesh.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Material/FXAsset.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialNames.h>
#include <Render/Texture.h>
#include <Render/TextureDescriptor.h>
#include <Scene3D/AnimationData.h>
#include <Scene3D/Components/AnimationComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Lod/LodComponent.h>
#include <Utils/StringFormat.h>

namespace DAVA
{
ImportLibrary::~ImportLibrary()
{
    for (auto& pair : polygons)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    polygons.clear();

    for (auto& pair : materialParents)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    materialParents.clear();

    for (auto& pair : animations)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    animations.clear();
}

PolygonGroup* ImportLibrary::GetOrCreatePolygon(ColladaPolygonGroupInstance* colladaPGI)
{
    // Try to take polygon from library
    PolygonGroup* davaPolygon = polygons[colladaPGI];

    // there is no polygon, so create new one
    if (nullptr == davaPolygon)
    {
        davaPolygon = new PolygonGroup();

        ColladaPolygonGroup* colladaPolygon = colladaPGI->polyGroup;
        DVASSERT(nullptr != colladaPolygon && "Empty collada polyton group instance.");

        auto& vertices = colladaPolygon->GetVertices();
        uint32 vertexCount = static_cast<uint32>(vertices.size());
        auto vertexFormat = colladaPolygon->GetVertexFormat();
        auto& indecies = colladaPolygon->GetIndices();
        int32 indexCount = static_cast<int32>(indecies.size());

        // Allocate data buffers before fill them
        davaPolygon->AllocateData(vertexFormat, vertexCount, indexCount);

        // Fill index array
        for (int32 indexNo = 0; indexNo < indexCount; ++indexNo)
        {
            davaPolygon->SetIndex(indexNo, indecies[indexNo]);
        }

        // Take collada vertices and set to polygon group
        InitPolygon(davaPolygon, vertexFormat, vertices);

        const int32 prerequiredFormat = EVF_TANGENT | EVF_BINORMAL | EVF_NORMAL;
        if ((davaPolygon->GetFormat() & prerequiredFormat) == prerequiredFormat)
        {
            MeshUtils::RebuildMeshTangentSpace(davaPolygon, true);
        }
        else
        {
            davaPolygon->BuildBuffers();
        }

        // Put polygon to the library
        polygons[colladaPGI] = davaPolygon;
    }

    // TO VERIFY: polygon

    return davaPolygon;
}

void ImportLibrary::InitPolygon(PolygonGroup* davaPolygon, uint32 vertexFormat, Vector<ColladaVertex>& vertices) const
{
    uint32 vertexCount = static_cast<uint32>(vertices.size());
    for (uint32 vertexNo = 0; vertexNo < vertexCount; ++vertexNo)
    {
        const auto& vertex = vertices[vertexNo];

        if (vertexFormat & EVF_VERTEX)
        {
            davaPolygon->SetCoord(vertexNo, vertex.position);
        }
        if (vertexFormat & EVF_NORMAL)
        {
            davaPolygon->SetNormal(vertexNo, vertex.normal);
        }
        if (vertexFormat & EVF_TANGENT)
        {
            davaPolygon->SetTangent(vertexNo, vertex.tangent);
        }
        if (vertexFormat & EVF_BINORMAL)
        {
            davaPolygon->SetBinormal(vertexNo, vertex.binormal);
        }
        if (vertexFormat & EVF_COLOR)
        {
            uint32 col = static_cast<uint32>(vertex.color.x * 255) | static_cast<uint32>(vertex.color.y * 255) << 8 | static_cast<uint32>(vertex.color.z * 255) << 16 | static_cast<uint32>(vertex.color.w * 255) << 24;
            davaPolygon->SetColor(vertexNo, col);
        }
        if (vertexFormat & EVF_TEXCOORD0)
        {
            Vector2 coord = vertex.texCoords[0];
            FlipTexCoords(coord);
            davaPolygon->SetTexcoord(0, vertexNo, coord);
        }
        if (vertexFormat & EVF_TEXCOORD1)
        {
            Vector2 coord = vertex.texCoords[1];
            FlipTexCoords(coord);
            davaPolygon->SetTexcoord(1, vertexNo, coord);
        }
        if (vertexFormat & EVF_TEXCOORD2)
        {
            Vector2 coord = vertex.texCoords[2];
            FlipTexCoords(coord);
            davaPolygon->SetTexcoord(2, vertexNo, coord);
        }
        if (vertexFormat & EVF_TEXCOORD3)
        {
            Vector2 coord = vertex.texCoords[3];
            FlipTexCoords(coord);
            davaPolygon->SetTexcoord(3, vertexNo, coord);
        }
        if (vertexFormat & EVF_HARD_JOINTINDEX)
        {
            DVASSERT(vertex.jointCount == 1);
            davaPolygon->SetHardJointIndex(vertexNo, vertex.joint[0]);
        }
        uint32 requiredSoftSkinFormat = EVF_JOINTINDEX | EVF_JOINTWEIGHT;
        if ((vertexFormat & requiredSoftSkinFormat) == requiredSoftSkinFormat)
        {
            DVASSERT(vertex.jointCount <= 4);
            for (int idx = 0; idx < 4; ++idx)
            {
                davaPolygon->SetJointIndex(vertexNo, idx, vertex.joint[idx]);
                davaPolygon->SetJointWeight(vertexNo, idx, vertex.weight[idx]);
            }
        }
    }
}

AnimationData* ImportLibrary::GetOrCreateAnimation(SceneNodeAnimation* colladaAnimation)
{
    AnimationData* animation = animations[colladaAnimation];
    if (nullptr == animation)
    {
        animation = new AnimationData();

        animation->SetInvPose(colladaAnimation->invPose);
        animation->SetDuration(colladaAnimation->duration);
        if (nullptr != colladaAnimation->keys)
        {
            for (uint32 keyNo = 0; keyNo < colladaAnimation->keyCount; ++keyNo)
            {
                SceneNodeAnimationKey key = colladaAnimation->keys[keyNo];
                animation->AddKey(key);
            }
        }

        animations[colladaAnimation] = animation;
    }

    return animation;
}

Asset<Texture> ImportLibrary::GetTextureForPath(const FilePath& imagePath) const
{
    RETextureDescriptorUtils::CreateOrUpdateDescriptor(imagePath);

    Texture::PathKey key(imagePath);
    return GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
}

NMaterial* ImportLibrary::GetOrCreateMaterialParent(ColladaMaterial* colladaMaterial, bool isShadow, uint32 maxInfluenceCount)
{
    FastName parentMaterialTemplate;
    FastName parentMaterialName;

    if (isShadow)
    {
        parentMaterialName = ImportSettings::shadowMaterialName;
        parentMaterialTemplate = NMaterialName::SHADOW_VOLUME;
    }
    else
    {
        parentMaterialName = FastName(colladaMaterial->material->GetDaeId().c_str());
        parentMaterialTemplate = NMaterialName::FORWARD_PBS;
    }
    FastName materialUniqueKey = FastName(Format("%s_%u", parentMaterialName.c_str(), maxInfluenceCount));

    NMaterial* davaMaterialParent = materialParents[materialUniqueKey];
    if (nullptr == davaMaterialParent)
    {
        davaMaterialParent = new NMaterial(FXDescriptor::TYPE_COMMON);
        davaMaterialParent->SetMaterialName(parentMaterialName);
        davaMaterialParent->SetFXName(parentMaterialTemplate);

        if (maxInfluenceCount == 1u)
        {
            davaMaterialParent->AddFlag(NMaterialFlagName::FLAG_HARD_SKINNING, 1);
        }
        else if (maxInfluenceCount != 0u)
        {
            davaMaterialParent->AddFlag(NMaterialFlagName::FLAG_SOFT_SKINNING, maxInfluenceCount);
        }

        materialParents[materialUniqueKey] = davaMaterialParent;

        FastName textureType;
        FilePath texturePath;
        bool hasTexture = GetTextureTypeAndPathFromCollada(colladaMaterial, textureType, texturePath);
        if (!isShadow && hasTexture && !texturePath.IsEmpty())
        {
            Asset<Texture> texture(GetTextureForPath(texturePath));
            davaMaterialParent->AddTexture(textureType, texture);

            FilePath normalMap = GetNormalMapTexturePath(texturePath);
            if (FileSystem::Instance()->IsFile(normalMap))
            {
                Asset<Texture> nmTexture(GetTextureForPath(normalMap));
                davaMaterialParent->AddTexture(NMaterialTextureName::TEXTURE_NORMAL, nmTexture);
            }
        }
    }

    return davaMaterialParent;
}

bool ImportLibrary::GetTextureTypeAndPathFromCollada(ColladaMaterial* material, FastName& type, FilePath& path) const
{
    ColladaTexture* diffuse = material->diffuseTexture;
    bool useDiffuseTexture = (nullptr != diffuse && material->hasDiffuseTexture);
    if (useDiffuseTexture)
    {
        type = NMaterialTextureName::TEXTURE_ALBEDO;
        path = diffuse->texturePathName.c_str();
        return true;
    }
    return false;
}

FilePath ImportLibrary::GetNormalMapTexturePath(const FilePath& originalTexturePath) const
{
    FilePath path = originalTexturePath;
    path.ReplaceBasename(path.GetBasename() + ImportSettings::normalMapPattern);
    return path;
}

} // DAVA namespace
