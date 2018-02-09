#include "UI/RichContent/UIRichContentSystem.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Logger/Logger.h"
#include "Render/RenderOptions.h"
#include "Render/Renderer.h"
#include "UI/RichContent/Private/RichLink.h"
#include "UI/RichContent/Private/XMLRichContentBuilder.h"
#include "UI/RichContent/UIRichAliasMap.h"
#include "UI/RichContent/UIRichContentAliasesComponent.h"
#include "UI/RichContent/UIRichContentComponent.h"
#include "UI/RichContent/UIRichContentObjectComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
UIRichContentSystem::UIRichContentSystem()
{
    Engine* engine = Engine::Instance();
    engine->windowCreated.Connect([&](Window*) {
        RenderOptions* options = Renderer::IsInitialized() ? Renderer::GetOptions() : nullptr;
        if (options)
        {
            isDebugDraw = options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_RICH_ITEMS);
            options->AddObserver(this);
        }
    });
    engine->windowDestroyed.Connect([&](Window*) {
        RenderOptions* options = Renderer::IsInitialized() ? Renderer::GetOptions() : nullptr;
        if (options)
        {
            options->RemoveObserver(this);
        }
    });
}

UIRichContentSystem::~UIRichContentSystem()
{
    RenderOptions* options = Renderer::IsInitialized() ? Renderer::GetOptions() : nullptr;
    if (options)
    {
        options->RemoveObserver(this);
    }
}

void UIRichContentSystem::RegisterControl(UIControl* control)
{
    UISystem::RegisterControl(control);
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        AddLink(component);
    }
}

void UIRichContentSystem::UnregisterControl(UIControl* control)
{
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        RemoveLink(component);
    }

    UISystem::UnregisterControl(control);
}

void UIRichContentSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    UISystem::RegisterComponent(control, component);

    UIRichContentComponent* richContent = CastIfEqual<UIRichContentComponent*>(component);
    if (richContent != nullptr)
    {
        AddLink(richContent);
    }

    UIRichContentAliasesComponent* richContentAliases = CastIfEqual<UIRichContentAliasesComponent*>(component);
    if (richContentAliases != nullptr)
    {
        AddAliases(control, richContentAliases);
    }
}

void UIRichContentSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    UIRichContentComponent* richContent = CastIfEqual<UIRichContentComponent*>(component);
    if (richContent != nullptr)
    {
        RemoveLink(richContent);
    }

    UIRichContentAliasesComponent* richContentAliases = CastIfEqual<UIRichContentAliasesComponent*>(component);
    if (richContentAliases != nullptr)
    {
        RemoveAliases(control, richContentAliases);
    }

    UISystem::UnregisterComponent(control, component);
}

void UIRichContentSystem::Process(float32 elapsedTime)
{
    // Add new links
    if (!appendLinks.empty())
    {
        links.insert(links.end(), appendLinks.begin(), appendLinks.end());
        appendLinks.clear();
    }
    // Remove empty links
    if (!links.empty())
    {
        links.erase(std::remove_if(links.begin(), links.end(), [](std::shared_ptr<RichLink>& l) {
                        return l->component == nullptr;
                    }),
                    links.end());
    }

    // Process links
    for (std::shared_ptr<RichLink>& l : links)
    {
        if (l->component)
        {
            bool update = l->component->IsModified();
            l->component->SetModified(false);
            for (UIRichContentAliasesComponent* c : l->aliasesComponents)
            {
                update |= c->IsModified();
                c->SetModified(false);
            }

            if (update)
            {
                UIControl* root = l->component->GetControl();
                l->RemoveItems();

                XMLRichContentBuilder builder(l.get(), isEditorMode, isDebugDraw);
                if (builder.Build("<span>" + l->component->GetText() + "</span>"))
                {
                    for (const RefPtr<UIControl>& ctrl : builder.GetControls())
                    {
                        root->AddControl(ctrl.Get());
                        l->AddItem(ctrl);
                    }
                }
                else
                {
                    const XMLParserStatus& st = builder.GetParserStatus();
                    Logger::Error("Syntax error in rich content text: %s (%d:%d)", st.errorMessage.c_str(), st.errorLine, st.errorPosition);
                }
            }
        }
    }
}

void UIRichContentSystem::SetEditorMode(bool editorMode)
{
    isEditorMode = editorMode;
}

void UIRichContentSystem::AddLink(UIRichContentComponent* component)
{
    DVASSERT(component);
    component->SetModified(true);

    std::shared_ptr<RichLink> link = std::make_shared<RichLink>();
    link->component = component;
    link->control = component->GetControl();

    uint32 count = link->control->GetComponentCount<UIRichContentAliasesComponent>();
    for (uint32 i = 0; i < count; ++i)
    {
        UIRichContentAliasesComponent* c = link->control->GetComponent<UIRichContentAliasesComponent>(i);
        link->AddAliases(c);
    }

    appendLinks.push_back(std::move(link));
}

void UIRichContentSystem::RemoveLink(UIRichContentComponent* component)
{
    DVASSERT(component);
    auto findIt = std::find_if(links.begin(), links.end(), [&component](std::shared_ptr<RichLink>& l) {
        return l->component == component;
    });
    if (findIt != links.end())
    {
        (*findIt)->RemoveItems();
        (*findIt)->component = nullptr; // mark link for delete
    }

    appendLinks.erase(std::remove_if(appendLinks.begin(), appendLinks.end(), [&component](std::shared_ptr<RichLink>& l) {
                          return l->component == component;
                      }),
                      appendLinks.end());
}

void UIRichContentSystem::AddAliases(UIControl* control, UIRichContentAliasesComponent* component)
{
    auto findIt = std::find_if(links.begin(), links.end(), [&control](std::shared_ptr<RichLink>& l) {
        return l->control == control;
    });
    if (findIt != links.end())
    {
        (*findIt)->AddAliases(component);
    }
}

void UIRichContentSystem::RemoveAliases(UIControl* control, UIRichContentAliasesComponent* component)
{
    auto findIt = std::find_if(links.begin(), links.end(), [&control](std::shared_ptr<RichLink>& l) {
        return l->control == control;
    });
    if (findIt != links.end())
    {
        (*findIt)->RemoveAliases(component);
    }
}

void UIRichContentSystem::HandleEvent(Observable* observable)
{
    RenderOptions* options = static_cast<RenderOptions*>(observable);
    bool newVal = options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_RICH_ITEMS);
    if (newVal != isDebugDraw)
    {
        isDebugDraw = newVal;
        for (std::shared_ptr<RichLink>& l : links)
        {
            if (l->component)
            {
                l->component->SetModified(true);
            }
        }
    }
}
}
