#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/MockModules/MockProjectManagerModule.h"
#include "Classes/SceneManager/SceneManagerModule.h"
#include "Classes/Selection/SelectionModule.h"

#include <REPlatform/Commands/TransformCommand.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/SelectableGroup.h>
#include <REPlatform/DataNodes/SelectionData.h>
#include <REPlatform/DataNodes/Settings/GlobalSceneSettings.h>
#include <REPlatform/Global/GlobalOperations.h>
#include <REPlatform/Scene/SceneEditor2.h>
#include <REPlatform/Scene/Systems/SelectionSystem.h>

#include <TArc/Core/ContextAccessor.h>
#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include <Engine/Qt/RenderWidget.h>
#include <Math/Transform.h>
#include <Math/TransformUtils.h>
#include <Render/Highlevel/Camera.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <UI/UIControlSystem.h>

#include <QApplication> 
#include <QSpinBox>
#include <QtTest>

using namespace DAVA;

DAVA_TARC_TESTCLASS(SelectionModuleTest)
{
    void SetFocus(QWidget * widget, QDoubleSpinBox * box)
    {
        box->clearFocus();
        if (box->isActiveWindow() == false)
            qApp->setActiveWindow(widget);

        box->setFocus(Qt::MouseFocusReason);
    };

    DAVA_TEST (SelectEntityTest)
    {
        ContextAccessor* accessor = GetAccessor();
        accessor->GetGlobalContext()->GetData<GlobalSceneSettings>()->autoSelectNewEntity = false;
        InvokeOperation(CreateFirstSceneOperation.ID);
        SceneEditor2* scene = accessor->GetActiveContext()->GetData<SceneData>()->GetScene().Get();
        ScopedPtr<Entity> node(new Entity());
        scene->AddNode(node);

        Vector3 newPosition(100, 100, 0);
        Transform newTransform(newPosition);
        TransformComponent* tc = node->GetComponent<TransformComponent>();
        scene->Exec(std::unique_ptr<Command>(new TransformCommand(Selectable(node.get()), tc->GetLocalTransform(), newTransform)));

        RenderWidget* renderWidget = GetContextManager()->GetRenderWidget();
        QDoubleSpinBox* box = new QDoubleSpinBox(renderWidget);

        auto stepMouseClick = [this, scene, renderWidget, node]()
        {
            Rect r(renderWidget->childrenRect().x(), renderWidget->childrenRect().y(), renderWidget->childrenRect().width(), renderWidget->childrenRect().height());
            Camera* camera = scene->GetCurrentCamera();
            Vector2 nodePos = camera->GetOnScreenPosition(node->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation(), r);
            QTest::mouseClick(GetRenderWidgetTestTarget(), Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(nodePos.x, nodePos.y));
        };

        auto stepCheckSelection = [this, scene, node]()
        {
            SelectableGroup selectionGroup = scene->GetSystem<SelectionSystem>()->GetSelection();
            TEST_VERIFY(selectionGroup.GetSize() == 1);
            TEST_VERIFY(selectionGroup.ContainsObject(node.get()));
            InvokeOperation(DAVA::CloseAllScenesOperation.ID, false);
        };

        auto& expectation = EXPECT_CALL(*this, AfterWrappersSync());
        for (int32 i = 0; i < FRAME_SKIP; ++i)
        {
            expectation.WillOnce(::testing::Return());
        }
        expectation.WillOnce(::testing::Invoke([this, renderWidget, box]() {
            SetFocus(renderWidget, box);
        }));
        expectation.WillOnce(::testing::Invoke(stepMouseClick));
        expectation.WillOnce(::testing::Invoke(stepCheckSelection));
        expectation.WillOnce(::testing::Return());
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ReflectionExtensionsModule)
    DECLARE_TESTED_MODULE(Mock::ProjectManagerModule)
    DECLARE_TESTED_MODULE(SceneManagerModule)
    DECLARE_TESTED_MODULE(SelectionModule)
    END_TESTED_MODULES()
};
