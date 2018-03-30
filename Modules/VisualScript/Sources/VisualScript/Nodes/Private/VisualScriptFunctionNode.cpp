#include "VisualScript/Nodes/VisualScriptFunctionNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Utils/StringFormat.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptFunctionNode)
{
    ReflectionRegistrator<VisualScriptFunctionNode>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<const FastName&, const FastName&>()
    .End();
}

VisualScriptFunctionNode::VisualScriptFunctionNode()
{
    SetType(FUNCTION);
}

VisualScriptFunctionNode::VisualScriptFunctionNode(const FastName& className_, const FastName& functionName_)
    : VisualScriptFunctionNode()
{
    SetClassName(className_);
    SetFunctionName(functionName_);
    InitPins();
}

void VisualScriptFunctionNode::SetClassName(const FastName& className_)
{
    className = className_;
}
void VisualScriptFunctionNode::SetFunctionName(const FastName& functionName_)
{
    functionName = functionName_;
}

const FastName& VisualScriptFunctionNode::GetClassName() const
{
    return className;
}

const FastName& VisualScriptFunctionNode::GetFunctionName() const
{
    return functionName;
}

const AnyFn& VisualScriptFunctionNode::GetFunction() const
{
    return function;
}

void VisualScriptFunctionNode::InitPins()
{
    if (className.IsValid() && functionName.IsValid())
    {
        String nodeName = Format("%s::%s", className.c_str(), functionName.c_str());
        SetName(FastName(nodeName));

        const ReflectedType* type = ReflectedTypeDB::GetByPermanentName(className.c_str());
        const ReflectedStructure::Method* method = type->GetStructure()->GetMethod(functionName);
        if (method)
        {
            function = method->fn;

            if (!function.IsConst() && !function.IsStatic())
            {
                RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_IN, FastName("exec"), nullptr));
                RegisterPin(new VisualScriptPin(this, VisualScriptPin::EXEC_OUT, FastName("exit"), nullptr));
            }

            const M::Params* paramsMeta = method->meta ? method->meta->GetMeta<M::Params>() : nullptr;
            int32 index = function.IsStatic() ? 0 : -1; // First argument is pointer to object
            for (auto type : function.GetInvokeParams().argsType)
            {
                FastName paramName;
                if (index == -1) // Give name to self-object first argument
                {
                    paramName = FastName("self");
                }
                else if (paramsMeta && paramsMeta->paramsNames.size() > index) // Give name to argument from meta
                {
                    paramName = FastName(paramsMeta->paramsNames[index]);
                }
                else // Give name to argument from argument' number
                {
                    FastName(Format("arg%d", index));
                }
                index++;

                RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_IN, paramName, type, VisualScriptPin::DEFAULT_PARAM));
            }

            const Type* returnType = function.GetInvokeParams().retType;
            if (returnType != Type::Instance<void>())
            {
                RegisterPin(new VisualScriptPin(this, VisualScriptPin::ATTR_OUT, FastName("result"), returnType));
            }
        }
        else
        {
            Logger::Error("Failed to find function %s in class %s", functionName.c_str(), className.c_str());
        }
    }
}

void VisualScriptFunctionNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);

    node->Add("className", className.c_str());
    node->Add("functionName", functionName.c_str());

    SaveDefaults(node);
}

void VisualScriptFunctionNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);

    className = FastName(node->Get("className")->AsString());
    functionName = FastName(node->Get("functionName")->AsString());
    InitPins();

    LoadDefaults(node);
}
}
