#include "DAVAEngine.h"
#include "DAVAClassRegistrator.h"
#include "Render/Highlevel/ShadowVolume.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Input/UIModalInputComponent.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/Focus/UIFocusGroupComponent.h"
#include "UI/Focus/UINavigationComponent.h"
#include "UI/Focus/UITabOrderComponent.h"
#include "UI/Input/UIActionComponent.h"
#include "UI/Input/UIActionBindingComponent.h"
#include "UI/Scroll/UIScrollBarDelegateComponent.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"

using namespace DAVA;

void DAVA::RegisterDAVAClasses()
{
    //this code do nothing. Needed to compiler generate code from this cpp file
    Logger* log = Logger::Instance();
    if (log)
        log->Log(Logger::LEVEL__DISABLE, "");

    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIControlBackground, "Background");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UILinearLayoutComponent, "LinearLayout");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFlowLayoutComponent, "FlowLayout");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFlowLayoutHintComponent, "FlowLayoutHint");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIIgnoreLayoutComponent, "IgnoreLayout");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UISizePolicyComponent, "SizePolicy");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIAnchorComponent, "Anchor");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIModalInputComponent, "ModalInput");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFocusComponent, "Focus");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFocusGroupComponent, "FocusGroup");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UINavigationComponent, "Navigation");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UITabOrderComponent, "TabOrder");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIActionComponent, "Action");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIActionBindingComponent, "ActionBinding");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIScrollBarDelegateComponent, "ScrollBarDelegate");

    GetEngineContext()->componentManager->RegisterUIComponent<UIControlBackground>();
    GetEngineContext()->componentManager->RegisterUIComponent<UILinearLayoutComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIFlowLayoutComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIFlowLayoutHintComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIIgnoreLayoutComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UISizePolicyComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIAnchorComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIModalInputComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIFocusComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIFocusGroupComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UINavigationComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UITabOrderComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIActionComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIActionBindingComponent>();
    GetEngineContext()->componentManager->RegisterUIComponent<UIScrollBarDelegateComponent>();
    
#if !defined(__DAVAENGINE_ANDROID__)
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TheoraPlayer);
#endif

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BaseObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PolygonGroup);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticMesh);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Camera);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIScrollViewContainer);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UISlider);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UISpinner);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIStaticText);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UISwitch);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UITextField);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Landscape);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(AnimationData);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Light);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Mesh);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SkinnedMesh);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SpeedTreeObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderBatch);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShadowVolume);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NMaterial);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DataNode);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Entity);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Scene);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIButton);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIControl);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIList);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIListCell);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIScrollBar);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIScrollView);
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(PartilceEmitterLoadProxy, "ParticleEmitter3D");
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIWebView);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIMovieView);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIParticles);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIJoypad);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VegetationRenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BillboardRenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SpriteObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UI3DView);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(AnimationComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TransformComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UpdatableComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CustomPropertiesComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ActionComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DebugRenderComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SoundComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BulletComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LightComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SpeedTreeComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WindComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WaveComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(QualitySettingsComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UserComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SwitchComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleEffectComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CameraComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionDataComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PathComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WASDControllerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RotationControllerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapToLandscapeControllerComponent);
}

#if !defined(__DAVAENGINE_ANDROID__)
REGISTER_CLASS(TheoraPlayer);
#endif

REGISTER_CLASS(BaseObject);
REGISTER_CLASS(PolygonGroup);
REGISTER_CLASS(StaticMesh);
REGISTER_CLASS(Camera);
REGISTER_CLASS(UIScrollViewContainer);
REGISTER_CLASS(UISlider);
REGISTER_CLASS(UISpinner);
REGISTER_CLASS(UIStaticText);
REGISTER_CLASS(UISwitch);
REGISTER_CLASS(UITextField);
REGISTER_CLASS(Landscape);
REGISTER_CLASS(AnimationData);
REGISTER_CLASS(Light);
REGISTER_CLASS(Mesh);
REGISTER_CLASS(SkinnedMesh);
REGISTER_CLASS(SpeedTreeObject);
REGISTER_CLASS(RenderBatch);
REGISTER_CLASS(RenderObject);
REGISTER_CLASS(ShadowVolume);
REGISTER_CLASS(NMaterial);
REGISTER_CLASS(DataNode);
REGISTER_CLASS(Entity);
REGISTER_CLASS(Scene);
REGISTER_CLASS(UIButton);
REGISTER_CLASS(UIControl);
REGISTER_CLASS(UIList);
REGISTER_CLASS(UIListCell);
REGISTER_CLASS(UIScrollBar);
REGISTER_CLASS(UIScrollView);
REGISTER_CLASS_WITH_ALIAS(PartilceEmitterLoadProxy, "ParticleEmitter3D");
REGISTER_CLASS(UIWebView);
REGISTER_CLASS(UIMovieView);
REGISTER_CLASS(UIParticles);
REGISTER_CLASS(UIJoypad);
REGISTER_CLASS(VegetationRenderObject);
REGISTER_CLASS(BillboardRenderObject);
REGISTER_CLASS(SpriteObject);
REGISTER_CLASS(UI3DView);
REGISTER_CLASS(AnimationComponent);
REGISTER_CLASS(TransformComponent);
REGISTER_CLASS(UpdatableComponent);
REGISTER_CLASS(RenderComponent);
REGISTER_CLASS(CustomPropertiesComponent);
REGISTER_CLASS(ActionComponent);
REGISTER_CLASS(DebugRenderComponent);
REGISTER_CLASS(SoundComponent);
REGISTER_CLASS(BulletComponent);
REGISTER_CLASS(LightComponent);
REGISTER_CLASS(SpeedTreeComponent);
REGISTER_CLASS(WindComponent);
REGISTER_CLASS(WaveComponent);
REGISTER_CLASS(QualitySettingsComponent);
REGISTER_CLASS(UserComponent);
REGISTER_CLASS(SwitchComponent);
REGISTER_CLASS(ParticleEffectComponent);
REGISTER_CLASS(CameraComponent);
REGISTER_CLASS(StaticOcclusionComponent);
REGISTER_CLASS(StaticOcclusionDataComponent);
REGISTER_CLASS(PathComponent);
REGISTER_CLASS(WASDControllerComponent);
REGISTER_CLASS(RotationControllerComponent);
REGISTER_CLASS(SnapToLandscapeControllerComponent);
