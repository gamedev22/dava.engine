#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/WindowSubSystem/QtTArcEvents.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include <QWidget>
#include <QObject>
#include <QCoreApplication>

#define RETURN_IF_MODEL_LOST(x) \
    if (this->wrapper.HasData() == false) \
    {\
        return (x);\
    }

#define DECLARE_CONTROL_PARAMS(Fields) \
    struct Params : BaseParams\
    { \
        Params(DAVA::ContextAccessor* accessor, DAVA::UI* ui, const DAVA::WindowKey& wndKey) \
            : BaseParams(accessor, ui, wndKey){} \
        DAVA::ControlDescriptorBuilder<Fields> fields; \
    };

namespace DAVA
{
class ControlProxy
{
public:
    virtual ~ControlProxy() = default;

    virtual void ForceUpdate() = 0;
    virtual void TearDown() = 0;
    virtual QWidget* ToWidgetCast() = 0;
};

template <typename TBase>
class ControlProxyImpl : protected TBase, public ControlProxy, protected DataListener
{
public:
    struct BaseParams
    {
        BaseParams(ContextAccessor* accessor_, UI* ui_, WindowKey wndKey_)
            : accessor(accessor_)
            , ui(ui_)
            , wndKey(wndKey_)
        {
        }

        ContextAccessor* accessor = nullptr;
        UI* ui = nullptr;
        WindowKey wndKey = WindowKey("");
    };
    ControlProxyImpl(const BaseParams& params, const ControlDescriptor& descriptor_, DataWrappersProcessor* wrappersProcessor, Reflection model_, QWidget* parent)
        : TBase(parent)
        , controlParams(params)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(wrappersProcessor);
    }

    ControlProxyImpl(const BaseParams& params, const ControlDescriptor& descriptor_, ContextAccessor* accessor, Reflection model_, QWidget* parent)
        : TBase(parent)
        , controlParams(params)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(accessor);
    }

    ControlProxyImpl(const BaseParams& params, const ControlDescriptor& descriptor_, Reflection model_, QWidget* parent)
        : TBase(parent)
        , controlParams(params)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(params.accessor);
    }

    ~ControlProxyImpl() override
    {
        TearDown();
    }

    void SetObjectName(const QString& objName)
    {
        TBase::setObjectName(objName);
    }

    QWidget* ToWidgetCast() override
    {
        return this;
    }

    void ForceUpdate() override
    {
        wrapper.Sync(false);
        OnDataChanged(wrapper, Vector<Any>());
    }

    void TearDown() override
    {
        wrapper.SetListener(nullptr);
        wrapper = DataWrapper();
    }

protected:
    template <typename TPrivate>
    ControlProxyImpl(const BaseParams& params, const ControlDescriptor& descriptor_, DataWrappersProcessor* wrappersProcessor, Reflection model_, TPrivate&& d, QWidget* parent)
        : TBase(std::move(d), parent)
        , controlParams(params)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(wrappersProcessor);
    }

    template <typename TPrivate>
    ControlProxyImpl(const BaseParams& params, const ControlDescriptor& descriptor_, ContextAccessor* accessor, Reflection model_, TPrivate&& d, QWidget* parent)
        : TBase(std::move(d), parent)
        , controlParams(params)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(accessor);
    }

    void SetupControl(DataWrappersProcessor* wrappersProcessor)
    {
        wrapper = wrappersProcessor->CreateWrapper(MakeFunction(this, &ControlProxyImpl<TBase>::GetModel), nullptr);
        wrapper.SetListener(this);
    }

    void SetupControl(ContextAccessor* accessor)
    {
        wrapper = accessor->CreateWrapper(MakeFunction(this, &ControlProxyImpl<TBase>::GetModel));
        wrapper.SetListener(this);
    }

    bool event(QEvent* e) override
    {
        switch (e->type())
        {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        case QEvent::MouseButtonDblClick:
            RETURN_IF_MODEL_LOST(false);
            break;
        default:
            break;
        }

        return TBase::event(e);
    }

protected:
    void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields) override
    {
        DVASSERT(wrapper.HasData());

        if (fields.empty())
        {
            for (ControlDescriptor::Field& f : descriptor.fieldNames)
            {
                if (f.name.IsValid())
                {
                    f.isChanged = true;
                }
            }
        }
        else
        {
            for (const Any& anyFieldName : fields)
            {
                FastName name = anyFieldName.Cast<FastName>();
                auto iter = std::find_if(descriptor.fieldNames.begin(), descriptor.fieldNames.end(), [&name](const ControlDescriptor::Field& f)
                                         {
                                             return f.name == name;
                                         });

                if (iter != descriptor.fieldNames.end())
                {
                    iter->isChanged = true;
                }
            }
        }

        UpdateControl(descriptor);
        for (ControlDescriptor::Field& f : descriptor.fieldNames)
        {
            f.isChanged = false;
        }
    }

    Reflection GetModel(const DataContext* ctx) const
    {
        return model;
    }

    template <typename Enum>
    FastName GetFieldName(Enum fieldMark) const
    {
        return descriptor.GetName(fieldMark);
    }

    virtual void UpdateControl(const ControlDescriptor& descriptor) = 0;

    template <typename Enum>
    bool IsReadOnlyConstOfMeta(Enum valueRole) const
    {
        DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(valueRole));
        DVASSERT(fieldValue.IsValid());

        return fieldValue.IsReadonly() == true || fieldValue.GetMeta<DAVA::M::ReadOnly>() != nullptr;
    }

    template <typename Enum>
    bool IsValueReadOnly(const ControlDescriptor& descriptor, Enum valueRole, Enum readOnlyRole) const
    {
        Any constValue = descriptor.GetConstValue(valueRole);
        if (constValue.IsEmpty() == false)
        {
            return true;
        }

        Reflection fieldValue = model.GetField(descriptor.GetName(valueRole));
        DVASSERT(fieldValue.IsValid());

        bool readOnlyFieldValue = false;
        FastName readOnlyFieldName = descriptor.GetName(readOnlyRole);
        if (readOnlyFieldName.IsValid())
        {
            Reflection fieldReadOnly = model.GetField(readOnlyFieldName);
            if (fieldReadOnly.IsValid())
            {
                readOnlyFieldValue = fieldReadOnly.GetValue().Cast<bool>();
            }
        }

        return fieldValue.IsReadonly() == true ||
        fieldValue.GetMeta<M::ReadOnly>() != nullptr ||
        readOnlyFieldValue == true;
    }

    template <typename Enum>
    bool IsValueEnabled(const ControlDescriptor& descriptor, Enum valueRole, Enum enabledRole) const
    {
        DAVA::Any constValue = descriptor.GetConstValue(valueRole);
        if (constValue.IsEmpty() == false)
        {
            return false;
        }

        DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(valueRole));
        DVASSERT(fieldValue.IsValid());

        bool enabledFieldValue = true;
        FastName enabledFieldName = descriptor.GetName(enabledRole);
        if (enabledFieldName.IsValid())
        {
            DAVA::Reflection fieldEnabled = model.GetField(enabledFieldName);
            if (fieldEnabled.IsValid())
            {
                enabledFieldValue = fieldEnabled.GetValue().Cast<bool>();
            }
        }

        bool isDisabled = fieldValue.IsReadonly() || fieldValue.GetMeta<DAVA::M::ReadOnly>() != nullptr || enabledFieldValue == false;
        return !isDisabled;
    }

    template <typename CastType, typename Enum>
    CastType GetFieldValue(Enum role, const CastType& defaultValue) const
    {
        DAVA::Any constValue = descriptor.GetConstValue(role);
        if (constValue.IsEmpty() == false)
        {
            return constValue.Cast<CastType>(defaultValue);
        }

        const FastName& fieldName = GetFieldName(role);
        if (fieldName.IsValid() == true)
        {
            Reflection field = model.GetField(fieldName);
            if (field.IsValid())
            {
                return field.GetValue().Cast<CastType>(defaultValue);
            }
        }

        return defaultValue;
    }

    template <typename Enum>
    void SetFieldValue(Enum role, const Any& value)
    {
        const FastName& fieldName = GetFieldName(role);
        DVASSERT(fieldName.IsValid());
        wrapper.SetFieldValue(fieldName, value);
    }

protected:
    Reflection model;
    DataWrapper wrapper;
    BaseParams controlParams;

private:
    ControlDescriptor descriptor;
};
} // namespace DAVA
