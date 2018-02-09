#include "ControlPropertiesSection.h"

#include "PropertyVisitor.h"
#include "ValueProperty.h"
#include "IntrospectionProperty.h"

#include <Reflection/ReflectedMeta.h>
#include <UI/UIControl.h>
#include <Reflection/ReflectedMeta.h>

#include <Reflection/ReflectedTypeDB.h>

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(const DAVA::String& name, DAVA::UIControl* control_, const DAVA::Type* type, const Vector<Reflection::Field>& fields, const ControlPropertiesSection* sourceSection, eCloneType cloneType)
    : SectionProperty(name)
    , control(SafeRetain(control_))
{
    for (const Reflection::Field& field : fields)
    {
        if (field.ref.GetMeta<M::HiddenField>() != nullptr)
        {
            continue;
        }

        if (field.inheritFrom->GetType() == type)
        {
            String name = field.key.Cast<String>();
            IntrospectionProperty* sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindChildPropertyByName(name);
            IntrospectionProperty* prop = IntrospectionProperty::Create(control, nullptr, name, field.ref, sourceProperty, cloneType);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

ControlPropertiesSection::~ControlPropertiesSection()
{
    SafeRelease(control);
}

void ControlPropertiesSection::Accept(PropertyVisitor* visitor)
{
    visitor->VisitControlSection(this);
}
