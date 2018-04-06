#include "QualitySettingsDialog.h"
#include "QualityPreferences.h"
#include "UIControls/TriggerBox.h"

#include <Base/Message.h>
#include <UI/Layouts/UISizePolicyComponent.h>
#include <UI/Layouts/UIAnchorComponent.h>
#include <UI/UIStaticText.h>
#include <UI/UIControlSystem.h>
#include <Render/2D/FTFont.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/QualitySettingsSystem.h>
#include <Scene3D/Systems/FoliageSystem.h>
#include <Scene3D/Systems/ParticleEffectSystem.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Utils/UTF8Utils.h>
#include <FileSystem/FileSystem.h>

namespace QualitySettingsDialogDetails
{
using namespace DAVA;

float32 OPACITY = 0.45f;

class QualitySettingsCell : public DAVA::UIListCell
{
public:
    QualitySettingsCell(DAVA::Font* font)
    {
        UISizePolicyComponent* sizePolicy = GetOrCreateComponent<UISizePolicyComponent>();
        sizePolicy->SetHorizontalPolicy(UISizePolicyComponent::PERCENT_OF_PARENT);
        sizePolicy->SetHorizontalValue(97.0f);

        {
            UIAnchorComponent* anchor = GetOrCreateComponent<UIAnchorComponent>();
            anchor->SetHCenterAnchorEnabled(true);
        }

        GetOrCreateComponent<DAVA::UIControlBackground>();
        GetBackground()->SetColor(Color(0.65f, 0.65f, 0.65f, OPACITY));
        GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);

        float32 leftColumnWidth = 85.f;

        leftColumnText = new UIStaticText(Rect(0.f, 0.f, leftColumnWidth, 0.f));
        leftColumnText->SetFont(font);
        leftColumnText->SetTextAlign(ALIGN_LEFT);
        leftColumnText->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
        {
            UIAnchorComponent* anchor = leftColumnText->GetOrCreateComponent<UIAnchorComponent>();
            anchor->SetLeftAnchorEnabled(true);
            anchor->SetLeftAnchor(10.f);
            anchor->SetTopAnchorEnabled(true);
            anchor->SetBottomAnchorEnabled(true);
        }
        AddControl(leftColumnText);

        rightColumn = new UIControl();
        {
            UIAnchorComponent* anchor = rightColumn->GetOrCreateComponent<UIAnchorComponent>();
            anchor->SetLeftAnchorEnabled(true);
            anchor->SetLeftAnchor(leftColumnWidth);
            anchor->SetRightAnchorEnabled(true);
            anchor->SetTopAnchorEnabled(true);
            anchor->SetBottomAnchorEnabled(true);
        }
        AddControl(rightColumn);
    }

    void SetLeftColumnText(const WideString& text)
    {
        DVASSERT(leftColumnText);
        leftColumnText->SetText(text);
    }

    void SetRightColumnControl(UIControl* control)
    {
        DVASSERT(rightColumn);
        DVASSERT(control != nullptr);
        rightColumn->AddControl(control);

        UIAnchorComponent* anchor = control->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetLeftAnchorEnabled(true);
        anchor->SetRightAnchorEnabled(true);
        anchor->SetTopAnchorEnabled(true);
        anchor->SetBottomAnchorEnabled(true);
    }

private:
    ScopedPtr<UIStaticText> leftColumnText = nullptr;
    ScopedPtr<UIControl> rightColumn = nullptr;
};

class CaptionCell : public UIListCell
{
public:
    CaptionCell(Font* font, const WideString& text)
    {
        using namespace DAVA;

        UISizePolicyComponent* sizePolicy = GetOrCreateComponent<UISizePolicyComponent>();
        sizePolicy->SetHorizontalPolicy(UISizePolicyComponent::PERCENT_OF_PARENT);
        sizePolicy->SetHorizontalValue(100.0f);

        ScopedPtr<UIStaticText> captionText(new UIStaticText(Rect(0.f, 0.f, 150.f, 0.f)));
        captionText->SetFont(font);
        captionText->SetTextAlign(ALIGN_VCENTER);
        captionText->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
        captionText->SetText(text);

        UIAnchorComponent* anchor = captionText->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetLeftAnchorEnabled(true);
        anchor->SetLeftAnchor(20.f);
        anchor->SetTopAnchorEnabled(true);
        anchor->SetBottomAnchorEnabled(true);

        AddControl(captionText);
    }
};
}

QualitySettingsDialog::QualitySettingsDialog(Settings& settings)
    : settings(settings)
{
    using namespace DAVA;

    const Size2i screenSize = DAVA::GetEngineContext()->uiControlSystem->vcs->GetVirtualScreenSize();
    float32 w = screenSize.dx * 2.f / 3.f;
    float32 h = screenSize.dy * 4.f / 5.f;
    SetSize(DAVA::Vector2(w, h));

    cellHeight = screenSize.dy / 20.0f;

    font = FTFont::Create("~res:/SceneViewer/Fonts/korinna.ttf");
    font->SetSize(cellHeight / 2.5f);

    GetOrCreateComponent<DAVA::UIControlBackground>();
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0.6f, 0.6f, 0.6f, QualitySettingsDialogDetails::OPACITY));

    {
        UIAnchorComponent* anchor = GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetRightAnchorEnabled(true);
        anchor->SetRightAnchor(30.f);
        anchor->SetVCenterAnchorEnabled(true);
    }

    captionText = new UIStaticText(Rect(0.f, 0.f, 0.f, cellHeight));
    captionText->SetText(L"Quality settings");
    captionText->SetFont(font);
    captionText->SetTextAlign(ALIGN_VCENTER | ALIGN_HCENTER);
    captionText->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    {
        UIAnchorComponent* anchor = captionText->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetTopAnchorEnabled(true);
        anchor->SetHCenterAnchorEnabled(true);
    }
    AddControl(captionText);

    float32 margin = 5.f;

    scrollableOptionsList = new UIList(Rect(), UIList::ORIENTATION_VERTICAL);
    scrollableOptionsList->SetDelegate(this);
    {
        UIAnchorComponent* anchor = scrollableOptionsList->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetTopAnchorEnabled(true);
        anchor->SetTopAnchor(cellHeight);
        anchor->SetLeftAnchorEnabled(true);
        anchor->SetLeftAnchor(margin);
        anchor->SetRightAnchorEnabled(true);
        anchor->SetRightAnchor(margin);
        anchor->SetBottomAnchorEnabled(true);
        anchor->SetBottomAnchor(cellHeight);
    }
    scrollableOptionsList->GetOrCreateComponent<DAVA::UIControlBackground>();
    scrollableOptionsList->GetBackground()->SetColor(Color(0.75f, 0.75f, 0.75f, QualitySettingsDialogDetails::OPACITY));
    scrollableOptionsList->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    AddControl(scrollableOptionsList);

    float32 buttonWidth = cellHeight * 3.0f;

    ScopedPtr<UIButton> okButton(new UIButton(Rect(0.f, 0.f, buttonWidth, cellHeight - 2 * margin)));
    okButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    okButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.5f, 0.6f, 0.5f, 0.5f));
    okButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    okButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.75f, 0.85f, 0.75f, 0.5f));
    okButton->SetStateFont(UIControl::STATE_NORMAL, font);
    okButton->SetStateText(UIControl::STATE_NORMAL, L"OK");
    okButton->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);
    okButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &QualitySettingsDialog::OnButtonOk));
    {
        UIAnchorComponent* anchor = okButton->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetRightAnchorEnabled(true);
        anchor->SetRightAnchor(margin + buttonWidth + margin + buttonWidth + margin);
        anchor->SetBottomAnchorEnabled(true);
        anchor->SetBottomAnchor(margin);
    }
    AddControl(okButton);

    ScopedPtr<UIButton> cancelButton(new UIButton(Rect(0.f, 0.f, buttonWidth, cellHeight - 2 * margin)));
    cancelButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    cancelButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.6f, 0.5f, 0.5f, 0.5f));
    cancelButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    cancelButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.85f, 0.75f, 0.75f, 0.5f));
    cancelButton->SetStateFont(UIControl::STATE_NORMAL, font);
    cancelButton->SetStateText(UIControl::STATE_NORMAL, L"Cancel");
    cancelButton->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);
    cancelButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &QualitySettingsDialog::OnButtonCancel));
    {
        UIAnchorComponent* anchor = cancelButton->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetRightAnchorEnabled(true);
        anchor->SetRightAnchor(margin + margin + buttonWidth);
        anchor->SetBottomAnchorEnabled(true);
        anchor->SetBottomAnchor(margin);
    }
    AddControl(cancelButton);

    applyButton = new UIButton(Rect(0.f, 0.f, buttonWidth, cellHeight - 2 * margin));
    applyButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    applyButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.6f, 0.5f, 0.5f, 0.5f));
    applyButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    applyButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.85f, 0.75f, 0.75f, 0.5f));
    applyButton->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    applyButton->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.75f, 0.65f, 0.65f, 0.5f));
    applyButton->SetStateFont(UIControl::STATE_NORMAL, font);
    applyButton->SetStateText(UIControl::STATE_NORMAL, L"Apply");
    applyButton->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);
    applyButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &QualitySettingsDialog::OnButtonApply));
    {
        UIAnchorComponent* anchor = applyButton->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetRightAnchorEnabled(true);
        anchor->SetRightAnchor(margin);
        anchor->SetBottomAnchorEnabled(true);
        anchor->SetBottomAnchor(margin);
    }
    AddControl(applyButton);

    BuildQualityControls();
}

void QualitySettingsDialog::BuildQualityControls()
{
    using namespace DAVA;
    using namespace QualitySettingsDialogDetails;

    auto ToWideString = [](const FastName& fastName)
    {
        WideString fastNameWS;
        UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(fastName.c_str()), strlen(fastName.c_str()), fastNameWS);
        return fastNameWS;
    };

    auto AddCaptionCell = [this](const WideString& text)
    {
        CaptionCell* captionCell = new CaptionCell(font, text);
        cells.push_back(captionCell);
    };

    DAVA::QualitySettingsSystem* qs = DAVA::QualitySettingsSystem::Instance();

    for (uint32 groupIndex = 0; groupIndex < QualityGroup::Count; ++groupIndex)
    {
        QualityGroup group = static_cast<QualityGroup>(groupIndex);

        QualitySettingsCell* cell = new QualitySettingsCell(font);
        cell->SetLeftColumnText(ToWideString(qs->GetQualityGroupName(group)));

        qualityBoxes[groupIndex] = new TriggerBox(*this, font);
        cell->SetRightColumnControl(qualityBoxes[groupIndex]);

        cells.emplace_back(cell);

        uint32 qualityIndex = 0;
        FastName currentQuality = qs->GetCurrentQualityForGroup(group);
        for (const FastName& q : qs->GetAvailableQualitiesForGroup(group))
        {
            qualityBoxes[groupIndex]->AddOption(static_cast<TriggerBox::OptionID>(qualityIndex), ToWideString(q), (q == currentQuality));
            ++qualityIndex;
        }
    }

    // quality options
    {
        AddCaptionCell(L"Options");

        DAVA::int32 optionsCount = DAVA::QualitySettingsSystem::Instance()->GetOptionsCount();
        for (DAVA::int32 i = 0; i < optionsCount; ++i)
        {
            QualitySettingsCell* cell = new QualitySettingsCell(font);
            cells.emplace_back(cell);

            DAVA::FastName optionName = DAVA::QualitySettingsSystem::Instance()->GetOptionName(i);
            cell->SetLeftColumnText(ToWideString(optionName) + L":");

            ScopedPtr<BinaryTriggerBox> binaryBox(new BinaryTriggerBox(*this, font, L"Yes", L"No"));
            qualityOptionBoxes.push_back(binaryBox);
            cell->SetRightColumnControl(binaryBox);
            binaryBox->SetOn(DAVA::QualitySettingsSystem::Instance()->IsOptionEnabled(optionName));
        }
    }

    AddCaptionCell(L"");

    applyButton->SetState(UIControl::eControlState::STATE_DISABLED);
}

QualitySettingsDialog::~QualitySettingsDialog()
{
    for (CellData& cellData : cells)
    {
        if (!cellData.wasRequested)
        {
            SafeRelease(cellData.cell); // remaining cells (i.e. wasRequested == false) are owned (and will be deleted) by UIList mechanism
        }
    }
}

void QualitySettingsDialog::ApplyQualitySettings()
{
    using namespace DAVA;

    bool qualityChanged = false;
    bool materialSettingsChanged = false;
    bool qualityGroupChanged[QualityGroup::Count] = {};

    DAVA::QualitySettingsSystem* qs = DAVA::QualitySettingsSystem::Instance();
    for (uint32 i = 0; i < QualityGroup::Count; ++i)
    {
        QualityGroup group = static_cast<QualityGroup>(i);
        size_t selectedQualityIndex = static_cast<size_t>(qualityBoxes[i]->GetSelectedOptionID());
        FastName selectedQuality = qs->GetAvailableQualitiesForGroup(group).at(selectedQualityIndex);
        if (selectedQuality != qs->GetCurrentQualityForGroup(group))
        {
            qs->SetCurrentQualityForGroup(group, selectedQuality);
            qualityGroupChanged[i] = true;
            qualityChanged = true;
        }
    }

    // quality options
    bool optionsChanged = false;
    int32 optionsCount = qs->GetOptionsCount();
    for (int32 i = 0; i < optionsCount; ++i)
    {
        DVASSERT(static_cast<size_t>(i) < qualityOptionBoxes.size());
        BinaryTriggerBox* optionBox = qualityOptionBoxes[i];
        bool checked = optionBox->IsOn();

        FastName optionName = qs->GetOptionName(i);

        if (qs->IsOptionEnabled(optionName) != checked)
        {
            qs->EnableOption(optionName, checked);
            optionsChanged = true;
        }
    }

    if (qualityGroupChanged[QualityGroup::RenderFlowType])
    {
        Renderer::SetRenderFlow(qs->GetCurrentQualityValue<QualityGroup::RenderFlowType>());
    }

    if (qualityGroupChanged[QualityGroup::Shadow])
    {
        ShadowQuality shadowQuality = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Shadow>();
        Renderer::GetRuntimeFlags().SetFlag(RuntimeFlags::Flag::SHADOW_CASCADES, std::min(uint32(MAX_SHADOW_CASCADES), shadowQuality.cascadesCount));
        Renderer::GetRuntimeFlags().SetFlag(RuntimeFlags::Flag::SHADOW_PCF, shadowQuality.PCFsamples);
        Renderer::GetRuntimeTextures().InvalidateTexture(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
        materialSettingsChanged = true;
    }

    if (qualityGroupChanged[QualityGroup::Scattering])
    {
        ScatteringQuality scatteringQuality = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Scattering>();
        Renderer::GetRuntimeFlags().SetFlag(RuntimeFlags::Flag::ATMOSPHERE_SCATTERING_SAMPLES, scatteringQuality.scatteringSamples);
        materialSettingsChanged = true;
    }

    if (materialSettingsChanged)
    {
        QualityPreferences::ReloadShaders();
    }

    if (qualityChanged || optionsChanged)
    {
        ApplyMaterialQuality();
        qs->UpdateEntityVisibility(scene);
        scene->foliageSystem->SyncFoliageWithLandscape();
    }

    QualityPreferences::SaveToSettings(settings);
    applyButton->SetState(UIControl::eControlState::STATE_DISABLED);
}

void QualitySettingsDialog::ResetQualitySettings()
{
    using namespace DAVA;

    QualitySettingsSystem* qs = QualitySettingsSystem::Instance();
    for (uint32 groupIndex = 0; groupIndex < QualityGroup::Count; ++groupIndex)
    {
        QualityGroup group = static_cast<QualityGroup>(groupIndex);
        FastName currentQuality = qs->GetCurrentQualityForGroup(group);
        uint32 i = 0;
        for (const FastName& q : qs->GetAvailableQualitiesForGroup(group))
        {
            if (currentQuality == q)
            {
                qualityBoxes[groupIndex]->SetOptionSelected(static_cast<TriggerBox::OptionID>(i));
                break;
            }
            ++i;
        }
    }

    /*/ materials quality
    for (size_t i = 0; i < QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount(); ++i)
    {
        TriggerBox* materialQualityBox = materialQualityBoxes[i];

        FastName groupName = QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(i);
        FastName curQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(groupName);

        for (size_t j = 0; j < QualitySettingsSystem::Instance()->GetMaterialQualityCount(groupName); ++j)
        {
            FastName qualityName = QualitySettingsSystem::Instance()->GetMaterialQualityName(groupName, j);
            if (qualityName == curQuality)
            {
                materialQualityBox->SetOptionSelected(static_cast<TriggerBox::OptionID>(j));
                break;
            }
        }
    }
    // */

    /*/ particles quality
    if (particleQualityBox)
    {
        const ParticlesQualitySettings& particlesSettings = QualitySettingsSystem::Instance()->GetParticlesQualitySettings();
        FastName curQuality = particlesSettings.GetCurrentQuality();

        for (size_t i = 0; i < particlesSettings.GetQualitiesCount(); ++i)
        {
            FastName qualityName = particlesSettings.GetQualityName(i);
            if (curQuality == qualityName)
            {
                particleQualityBox->SetOptionSelected(static_cast<TriggerBox::OptionID>(i));
                break;
            }
        }
    }
    // */

    // quality options
    int32 optionsCount = QualitySettingsSystem::Instance()->GetOptionsCount();
    for (int32 i = 0; i < optionsCount; ++i)
    {
        FastName optionName = QualitySettingsSystem::Instance()->GetOptionName(i);

        DVASSERT(static_cast<size_t>(i) < qualityOptionBoxes.size());
        BinaryTriggerBox* binaryBox = qualityOptionBoxes[i];
        binaryBox->SetOn(QualitySettingsSystem::Instance()->IsOptionEnabled(optionName));
    }
}

void QualitySettingsDialog::ApplyTextureQuality()
{
    DAVA::Set<DAVA::NMaterial*> materialList;

    DAVA::List<DAVA::NMaterial*> materials;
    scene->GetDataNodes(materials);

    DAVA::List<DAVA::Texture*> textures;

    for (auto& material : materials)
    {
        materialList.insert(material);
    }

    DAVA::TexturesMap textureMap;

    for (DAVA::NMaterial* material : materialList)
    {
        DAVA::String materialName = material->GetMaterialName().c_str();
        DAVA::String parentName = material->GetParent() ? material->GetParent()->GetMaterialName().c_str() : DAVA::String();

        if ((parentName.find("Particle") != DAVA::String::npos) || (materialName.find("Particle") != DAVA::String::npos))
        {
            // because particle materials has textures only after first start
            // so we have different result during scene life.
            continue;
        }

        DAVA::Set<DAVA::MaterialTextureInfo*> materialTextures;
        material->CollectLocalTextures(materialTextures);

        for (DAVA::MaterialTextureInfo* matTex : materialTextures)
        {
            if (DAVA::FileSystem::Instance()->Exists(matTex->path) && matTex->texture)
            {
                textureMap[FILEPATH_MAP_KEY(matTex->path)] = matTex->texture;
            }
        }
    }

    DAVA::eGPUFamily forGPU = DAVA::Texture::GetPrimaryGPUForLoading();

    for (auto& entry : textureMap)
    {
        DAVA::Texture* texture = entry.second;
        texture->ReloadAs(forGPU);
    }

    for (DAVA::NMaterial* material : materialList)
    {
        material->InvalidateTextureBindings();
    }

    DAVA::Sprite::ReloadSprites(forGPU);
}

void QualitySettingsDialog::ApplyMaterialQuality()
{
    DAVA::List<DAVA::NMaterial*> materials;
    scene->GetDataNodes(materials);

    for (auto material : materials)
    {
        material->InvalidateRenderVariants();
    }

    scene->renderSystem->SetForceUpdateLights();
}

void QualitySettingsDialog::ApplyParticlesQuality()
{
    ReloadEntityEmitters(scene);
}

void QualitySettingsDialog::ReloadEntityEmitters(DAVA::Entity* e)
{
    DAVA::ParticleEffectComponent* comp = GetEffectComponent(e);
    if (comp)
    {
        comp->ReloadEmitters();
    }

    for (DAVA::int32 i = 0, sz = e->GetChildrenCount(); i < sz; ++i)
    {
        ReloadEntityEmitters(e->GetChild(i));
    }
}

void QualitySettingsDialog::SetParentControl(UIControl* parent)
{
    DVASSERT(parent != nullptr);
    parentControl = parent;
}

void QualitySettingsDialog::SetDelegate(QualitySettingsDialogDelegate* _delegate)
{
    DVASSERT(_delegate != nullptr);
    delegate = _delegate;
}

void QualitySettingsDialog::SetCurrentScene(DAVA::Scene* s)
{
    DVASSERT(s != nullptr);
    scene = s;
}

void QualitySettingsDialog::Show()
{
    DVASSERT(parentControl != nullptr);
    DVASSERT(delegate != nullptr);

    parentControl->AddControl(this);
}

void QualitySettingsDialog::OnButtonOk(DAVA::BaseObject* caller, void* param, void* callerData)
{
    parentControl->RemoveControl(this);
    ApplyQualitySettings();
    delegate->OnQualitySettingsEditDone();
}

void QualitySettingsDialog::OnButtonCancel(DAVA::BaseObject* caller, void* param, void* callerData)
{
    parentControl->RemoveControl(this);
    ResetQualitySettings();
    delegate->OnQualitySettingsEditDone();
}

void QualitySettingsDialog::OnButtonApply(DAVA::BaseObject* caller, void* param, void* callerData)
{
    ApplyQualitySettings();
}

DAVA::int32 QualitySettingsDialog::ElementsCount(DAVA::UIList* list)
{
    return static_cast<DAVA::int32>(cells.size());
}

DAVA::float32 QualitySettingsDialog::CellHeight(DAVA::UIList* list, DAVA::int32 index)
{
    return cellHeight;
}

DAVA::UIListCell* QualitySettingsDialog::CellAtIndex(DAVA::UIList* list, DAVA::int32 index)
{
    DAVA::UIListCell* c = list->GetReusableCell(DAVA::Format("cell%d", index));
    if (!c)
    {
        cells[index].wasRequested = true;
        return cells[index].cell;
    }
    else
    {
        return c;
    }
}

void QualitySettingsDialog::OnOptionChanged(TriggerBox*)
{
    applyButton->SetState(UIControl::eControlState::STATE_NORMAL);
}
