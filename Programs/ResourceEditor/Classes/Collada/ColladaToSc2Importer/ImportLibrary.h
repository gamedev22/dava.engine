#pragma once

#include "Asset/Asset.h"
#include "Base/FastName.h"

namespace DAVA
{
class NMaterial;
class PolygonGroup;
class ColladaPolygonGroupInstance;
class AnimationData;
class ColladaMaterial;
class SceneNodeAnimation;
class Texture;

class ImportLibrary
{
public:
    ~ImportLibrary();

    PolygonGroup* GetOrCreatePolygon(ColladaPolygonGroupInstance* colladaPGI);
    NMaterial* GetOrCreateMaterialParent(ColladaMaterial* colladaMaterial, bool isShadow, uint32 maxInfluenceCount);
    AnimationData* GetOrCreateAnimation(SceneNodeAnimation* colladaSceneNode);

private:
    Asset<Texture> GetTextureForPath(const FilePath& imagePath) const;
    void InitPolygon(PolygonGroup* davaPolygon, uint32 vertexFormat, Vector<ColladaVertex>& vertices) const;
    bool GetTextureTypeAndPathFromCollada(ColladaMaterial* material, FastName& type, FilePath& path) const;
    FilePath GetNormalMapTexturePath(const FilePath& originalTexturePath) const;
    inline void FlipTexCoords(Vector2& v) const;

    Map<ColladaPolygonGroupInstance*, PolygonGroup*> polygons;
    Map<FastName, NMaterial*> materialParents;
    Map<SceneNodeAnimation*, AnimationData*> animations;

    DAVA::uint32 materialInstanceNumber = 0;
};

inline void ImportLibrary::FlipTexCoords(Vector2& v) const
{
    v.y = 1.0f - v.y;
}
}
