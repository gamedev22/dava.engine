#pragma once

#include "Asset/AbstractAssetLoader.h"
#include "Asset/Asset.h"
#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"
#include "Base/Type.h"
#include "Concurrency/Mutex.h"
#include "FileSystem/File.h"
#include "Asset/AssetListener.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class ShaderDescriptor;
class ShaderAssetLoader final : public AbstractAssetLoader
{
public:
    ShaderAssetLoader();

    AssetFileInfo GetAssetFileInfo(const Any& assetKey) const override;
    bool ExistsOnDisk(const Any& assetKey) const override;

    AssetBase* CreateAsset(const Any& assetKey) const override;
    void DeleteAsset(AssetBase* asset) const override;
    void LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const override;
    bool SaveAsset(Asset<AssetBase>, File* file, eSaveMode requestedMode) const override;
    bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const override;
    Vector<String> GetDependsOnFiles(const AssetBase* asset) const override;
    Vector<const Type*> GetAssetKeyTypes() const override;

    static Vector<size_t> BuildFlagsKey(const FastName& name, const UnorderedMap<FastName, int32>& defines);
    static size_t GetUniqueFlagKey(FastName flagName);
    static String BuildResourceName(const ShaderDescriptor::Key& key, Vector<String>& progDefines);

private:
    struct ShaderSourceCode
    {
        Vector<char> vertexProgText;
        Vector<char> fragmentProgText;

        FilePath vertexProgSourcePath;
        FilePath fragmentProgSourcePath;

        uint32 vSrcHash = 0;
        uint32 fSrcHash = 0;
    };

    const ShaderSourceCode& GetSourceCode(const FastName& name, bool reloading, String& error) const;
    const ShaderSourceCode& GetSourceCodeImpl(const FastName& name, bool reloading, String& error);

    void LoadFromSource(const String& source, ShaderAssetLoader::ShaderSourceCode& sourceCode, String& error);
    bool LoadShaderSource(const String& source, Vector<char>& shaderText, String& error);

    Mutex mutex;
    Map<FastName, ShaderSourceCode> shaderSourceCodes;

    DAVA_VIRTUAL_REFLECTION(ShaderAssetLoader, AbstractAssetLoader);
};

class ShaderAssetListener : public StaticSingleton<ShaderAssetListener>
{
public:
    void Init();
    void Shoutdown();

    void ClearDynamicBindigs();
    void SetShadersCleanupEnabled(bool enabled);
    void SetLoadingNotifyEnabled(bool enable);

private:
    void OnAssetLoaded(const Asset<AssetBase>& asset);
    void OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset);
    void OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const String& msg);

    void Cleanup();

private:
    SimpleAssetListener listener;
    UnorderedSet<Asset<ShaderDescriptor>> shaders;
    bool cleanupEnabled = false;
    bool loadingNotify = false;
};
} // namespace DAVA
