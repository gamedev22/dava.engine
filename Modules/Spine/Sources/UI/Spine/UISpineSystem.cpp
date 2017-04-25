#include "UI/Spine/UISpineSystem.h"
#include "UI/Spine/UISpineComponent.h"
#include "UI/Spine/SpineSkeleton.h"

#include <UI/UIControl.h>
#include <UI/Components/UIComponent.h>

#include <spine/spine.h>

namespace DAVA
{

UISpineSystem::UISpineSystem() = default;

UISpineSystem::~UISpineSystem() = default;

void UISpineSystem::RegisterControl(UIControl * control)
{
    UISpineComponent* component = control->GetComponent<UISpineComponent>();
    if (component)
    {
        AddNode(component);
    }
}

void UISpineSystem::UnregisterControl(UIControl * control)
{
    UISpineComponent* component = control->GetComponent<UISpineComponent>();
    if (component)
    {
        RemoveNode(component);
    }
}

void UISpineSystem::RegisterComponent(UIControl * control, UIComponent * component)
{
    if (component->GetType() == Type::Instance<UISpineComponent>())
    {
        AddNode(static_cast<UISpineComponent*>(component));
    }
}

void UISpineSystem::UnregisterComponent(UIControl * control, UIComponent * component)
{
    if (component->GetType() == Type::Instance<UISpineComponent>())
    {
        RemoveNode(static_cast<UISpineComponent*>(component));
    }
}

void UISpineSystem::OnControlVisible(UIControl * control)
{
}

void UISpineSystem::OnControlInvisible(UIControl * control)
{
}

void UISpineSystem::Process(DAVA::float32 elapsedTime)
{
    for(SpineNode& node : nodes)
    {
        UISpineComponent* component = node.component;
        const RefPtr<SpineSkeleton>& skeleton = node.skeleton;

        if (component->IsNeedReload())
        {
            skeleton->Load(component->GetSkeletonPath(), component->GetAtlasPath());

            bool modified = component->IsModified();
            component->SetAnimationsNames(skeleton->GetAvailableAnimationsNames());
            component->SetModified(modified);

            component->SetNeedReload(false);
        }

        if (component->IsModified())
        {
            switch(component->GetAnimationState())
            {
            case UISpineComponent::PLAYED:
                {
                    const String& name = component->GetAnimationName();
                    if (!name.empty())
                    {
                        skeleton->SetAnimation(0, name, component->IsLoopedPlayback());
                    }
                }
                break;
            case UISpineComponent::STOPPED:
                skeleton->ClearTracks();
                break;
            }
            component->SetModified(false);
        }

        skeleton->Update(elapsedTime);

        UIControl* control = component->GetControl();
        if (control->IsVisible())
        {
            UIControlBackground* bg = control->GetComponent<UIControlBackground>();
            if (bg)
            {
                bg->SetDrawType(UIControlBackground::DRAW_BATCH);
                bg->SetRenderBatch(skeleton->GetRenderBatch());
            }
        }
    }
}

void UISpineSystem::AddNode(UISpineComponent* component)
{
    SpineNode node;
    node.component = component;
    node.skeleton.Set(new SpineSkeleton());
    node.skeleton->onEvent.Connect([this, component](const String& event) {
        // System event observer
        onAnimationEvent.Emit(component, event);
    });
    node.skeleton->onEvent.Connect([this, component](const String& event) {
        // Compoent event observer
        component->onAnimationEvent.Emit(component, event);
    });
    nodes.push_back(node);
}

void UISpineSystem::RemoveNode(UISpineComponent* component)
{
    auto it = std::find_if(nodes.begin(), nodes.end(), [component](const SpineNode& node) {
        return node.component == component;
    });

    if (it != nodes.end())
    {
        UIControl* ctrl = it->component->GetControl();
        UIControlBackground* bg = ctrl->GetComponent<UIControlBackground>();
        if (bg)
        {
            bg->SetRenderBatch(nullptr);
        }
        nodes.erase(it, nodes.end());
    }
}

}