#include "Settings.h"

#include <FileSystem/KeyedArchive.h>
#include <DocDirSetup/DocDirSetup.h>

namespace SceneViewerSettingsDetails
{
const char* SETTINGS_CONFIG_FILE = "~doc:/SceneViewerOptions.archive";
DAVA::String SETTING_LAST_OPENED_SCENE = "Settings/LastScenePath";
DAVA::String SETTING_QUALITY = "Quality/";
}

Settings::Settings()
    : settings(new DAVA::KeyedArchive())
{
}

void Settings::Load()
{
    settings.reset(new DAVA::KeyedArchive());
    settings->Load(SceneViewerSettingsDetails::SETTINGS_CONFIG_FILE);
}

void Settings::Save()
{
    settings->Save(SceneViewerSettingsDetails::SETTINGS_CONFIG_FILE);
}

DAVA::VariantType* Settings::GetValue(const DAVA::String& path) const
{
    DVASSERT(settings);
    return settings->GetVariant(path);
}

void Settings::SetValue(const DAVA::String& path, const DAVA::VariantType& value)
{
    DVASSERT(settings);
    settings->SetVariant(path, value);
    Save();
}

const DAVA::KeyedArchive* Settings::GetQualitySettings() const
{
    DAVA::VariantType* value = GetValue(SceneViewerSettingsDetails::SETTING_QUALITY);
    if (value != nullptr && value->GetType() == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
    {
        return value->AsKeyedArchive();
    }
    else
    {
        return nullptr;
    }
}

void Settings::SetQualitySettings(DAVA::KeyedArchive* archive)
{
    DVASSERT(archive != nullptr);
    SetValue(SceneViewerSettingsDetails::SETTING_QUALITY, DAVA::VariantType(archive));
}

DAVA::FilePath Settings::GetLastOpenedScenePath() const
{
    DAVA::FilePath result;
    DAVA::VariantType* value = GetValue(SceneViewerSettingsDetails::SETTING_LAST_OPENED_SCENE);
    if (value != nullptr && value->GetType() == DAVA::VariantType::TYPE_STRING)
    {
        DAVA::FilePath::AddResourcesFolder(DAVA::DocumentsDirectorySetup::GetEngineDocumentsPath());
        result = DAVA::FilePath(value->AsString()).GetAbsolutePathname();
        DAVA::FilePath::RemoveResourcesFolder(DAVA::DocumentsDirectorySetup::GetEngineDocumentsPath());
    }
    return result;
}

void Settings::SetLastOpenedScenePath(const DAVA::FilePath& path)
{
    DAVA::FilePath::AddResourcesFolder(DAVA::DocumentsDirectorySetup::GetEngineDocumentsPath());
    SetValue(SceneViewerSettingsDetails::SETTING_LAST_OPENED_SCENE, DAVA::VariantType(path.GetFrameworkPath()));
    DAVA::FilePath::RemoveResourcesFolder(DAVA::DocumentsDirectorySetup::GetEngineDocumentsPath());
}
