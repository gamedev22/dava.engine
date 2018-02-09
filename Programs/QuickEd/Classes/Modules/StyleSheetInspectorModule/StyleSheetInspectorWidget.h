#pragma once

#include "Model/PackageHierarchy/PackageListener.h"

#include "Utils/PackageListenerProxy.h"

#include <TArc/Core/FieldBinder.h>

#include <QtTools/Updaters/ContinuousUpdater.h>

#include <Base/RefPtr.h>

#include <QListWidget>

namespace DAVA
{
class UIControl;
}

class StyleSheetInspectorWidget : public QListWidget, public PackageListener
{
public:
    StyleSheetInspectorWidget(DAVA::ContextAccessor* accessor);
    ~StyleSheetInspectorWidget() override;

private:
    void InitFieldBinder();

    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property) override;
    void StyleSheetsWereRebuilt() override;

    void OnSelectionChanged(const DAVA::Any& selectionValue);

    void Update();

    DAVA::ContextAccessor* accessor = nullptr;
    DAVA::RefPtr<DAVA::UIControl> currentControl;
    ContinuousUpdater updater;
    std::unique_ptr<DAVA::FieldBinder> selectionFieldBinder;
    PackageListenerProxy packageListenerProxy;
};
