#include "Modules/ModernPropertiesModule/Editors/ModernPropertyBindingEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Modules/DataBindingInspectorModule/DataBindingInspectorModel.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>
#include <Engine/Engine.h>
#include <UI/DataBinding/UIDataBindingComponent.h>
#include <UI/DataBinding/UIDataBindingSystem.h>
#include <UI/DataBinding/Private/UIDataModel.h>
#include <UI/Formula/FormulaContext.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>

#include <TArc/Utils/Utils.h>

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QTreeView>

ModernPropertyBindingEditor::ModernPropertyBindingEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    updateModeComboBox = new QComboBox();
    updateModeComboBox->setProperty("property", true);

    updateModeComboBox->addItem(QIcon(":/Icons/link-r.png"), "", UIDataBindingComponent::MODE_READ);
    updateModeComboBox->addItem(QIcon(":/Icons/link-w.png"), "", UIDataBindingComponent::MODE_WRITE);
    updateModeComboBox->addItem(QIcon(":/Icons/link-rw.png"), "", UIDataBindingComponent::MODE_READ_WRITE);

    QObject::connect(updateModeComboBox, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged),
                     this, &ModernPropertyBindingEditor::OnCurrentIndexChanged);

    line = new QLineEdit();
    line->setProperty("property", true);
    QObject::connect(line, &QLineEdit::editingFinished, this, &ModernPropertyBindingEditor::OnEditingFinished);

    bindButton = new QToolButton();
    bindButton->setText("...");
    bindButton->setProperty("property", true);
    QObject::connect(bindButton, &QToolButton::clicked, this, &ModernPropertyBindingEditor::OnButtonClicked);

    layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(4);

    layout->addWidget(updateModeComboBox);
    layout->addWidget(line);
    layout->addWidget(bindButton);

    OnPropertyChanged();
}

ModernPropertyBindingEditor::~ModernPropertyBindingEditor()
{
    delete updateModeComboBox;
    delete line;
    delete bindButton;
    delete layout;
}

bool ModernPropertyBindingEditor::IsBindingEditor() const
{
    return true;
}

void ModernPropertyBindingEditor::AddToGrid(QGridLayout* grid, int row, int col, int colSpan)
{
    grid->addWidget(propertyName, row, col);
    grid->addLayout(layout, row, col + 1, 1, colSpan);
}

void ModernPropertyBindingEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    QSignalBlocker blockSignalsLine(line);
    QSignalBlocker blockSignalsUpdateModeComboBox(updateModeComboBox);

    QString stringValue = StringToQString(property->GetBindingExpression());
    line->setText(DAVA::UnescapeString(stringValue));
    line->setDisabled(property->IsReadOnly());

    for (int i = 0; i < updateModeComboBox->count(); i++)
    {
        if (updateModeComboBox->itemData(i).toInt() == property->GetBindingUpdateMode())
        {
            updateModeComboBox->setCurrentIndex(i);
            break;
        }
    }

    ApplyStyleToWidget(line);
}

void ModernPropertyBindingEditor::OnEditingFinished()
{
    if (line->isModified())
    {
        ApplyChanges();
    }
}

void ModernPropertyBindingEditor::OnCurrentIndexChanged(int index)
{
    ApplyChanges();
}

void ModernPropertyBindingEditor::OnButtonClicked()
{
    using namespace DAVA;

    DataBindingInspectorModel* model = new DataBindingInspectorModel(true, GetParentWidget());
    RootProperty* root = dynamic_cast<RootProperty*>(property->GetRootProperty());
    UIControl* control = nullptr;
    if (root)
    {
        ControlNode* controlNode = root->GetControlNode();
        control = controlNode->GetControl();
    }

    if (control)
    {
        FormulaContext* context = DAVA::GetEngineContext()->uiControlSystem->GetSystem<UIDataBindingSystem>()->GetFormulaContext(control, UIDataModel::PRIORITY_DATA_BINDING);
        model->UpdateModel(context);
    }

    QString expr = bindAction->property("bindingExpr").toString();
    int32 bindingMode = bindAction->property("bindingMode").toInt();
    QVariant modelVariant = bindAction->property("model");

    QDialog* dialog = new QDialog(GetParentWidget());
    dialog->setWindowTitle("Bind Data");
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    dialog->setLayout(layout);
    dialog->setModal(true);

    QTreeView* treeView = new QTreeView();
    treeView->setModel(model);
    treeView->expandAll();
    treeView->resizeColumnToContents(0);

    dialog->layout()->addWidget(treeView);

    QDialogButtonBox* buttons = new QDialogButtonBox(Qt::Horizontal);
    buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialog->layout()->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

    if (dialog->exec() == QDialog::Accepted)
    {
        QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
        QString exprStr = "";
        if (!indices.empty())
        {
            exprStr = model->data(indices.first(), DataBindingInspectorModel::PATH_DATA).toString();
        }

        DAVA::int32 mode = updateModeComboBox->currentData().toInt();
        ChangeBinding(QStringToString(exprStr), mode);
    }
}

void ModernPropertyBindingEditor::ApplyChanges()
{
    QString expr = DAVA::EscapeString(line->text());
    DAVA::int32 mode = updateModeComboBox->currentData().toInt();
    ChangeBinding(QStringToString(expr), mode);
}
