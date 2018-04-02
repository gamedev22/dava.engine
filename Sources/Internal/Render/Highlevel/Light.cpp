#include "Render/Highlevel/Light.h"
#include "Render/RenderHelper.h"
#include "Particles/ParticlePropertyLine.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Utils/StringFormat.h"

#include "Engine/Engine.h"
#include "Base/GlobalEnum.h"

ENUM_DECLARE(DAVA::Light::eFlags)
{
    ENUM_ADD_DESCR(DAVA::Light::DYNAMIC_LIGHT, "Dynamic");
    ENUM_ADD_DESCR(DAVA::Light::CASTS_SHADOW, "Casts Shadow");
}

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(Light)
{
    ReflectionRegistrator<Light>::Begin()
    .End();
}

Light::Light()
{
    bbox = AABBox3(Vector3(0.0f, 0.0f, 0.0f), 1.0f); // GFX_COMPLETE - temporary
    RenderObject::type = TYPE_LIGHT;
}

void Light::SetAmbientColor(const Color& _color)
{
    ambientColorDeprecated = _color;
}

void Light::SetColor(const Color& _color)
{
    baseColor = _color;
    lastSetColorTemperature = 0.0f;
}

void Light::SetIntensity(float32 _intensity)
{
    intensityDeprecated = _intensity;
}

RenderObject* Light::Clone(RenderObject* dstNode)
{
    if (dstNode == nullptr)
    {
        DVASSERT(IsPointerToExactClass<Light>(this), "Can clone only LightNode");
        dstNode = new Light();
    }

    Light* lightNode = static_cast<Light*>(dstNode);

    lightNode->lightType = lightType;
    lightNode->baseColor = baseColor;
    lightNode->ambientColorDeprecated = ambientColorDeprecated;
    lightNode->intensityDeprecated = intensityDeprecated;
    lightNode->radius = radius;
    lightNode->lightFlags = lightFlags;

    return dstNode;
}

void Light::SetPositionDirectionFromMatrix(const Matrix4& worldTransform)
{
    position = Vector3(0.0f, 0.0f, 0.0f) * worldTransform;
    direction = MultiplyVectorMat3x3(Vector3(0.0, -1.0f, 0.0f), worldTransform);
    direction.Normalize();
}

void Light::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    BaseObject::SaveObject(archive);

    archive->SetInt32("type", GetLightType());
    archive->SetFloat("ambColor.r", ambientColorDeprecated.r);
    archive->SetFloat("ambColor.g", ambientColorDeprecated.g);
    archive->SetFloat("ambColor.b", ambientColorDeprecated.b);
    archive->SetFloat("ambColor.a", ambientColorDeprecated.a);
    archive->SetFloat("color.r", baseColor.r);
    archive->SetFloat("color.g", baseColor.g);
    archive->SetFloat("color.b", baseColor.b);
    archive->SetFloat("color.a", baseColor.a);
    archive->SetFloat("intensity", intensityDeprecated);
    archive->SetFloat("radius", radius);
    archive->SetFloat("ao.radius", aoRadius);
    archive->SetUInt32("flags", lightFlags);
    archive->SetBool("autoColor", autoColor);

    archive->SetFloat("shadow.cascades1.0", shadowCascadesIntervals1);

    archive->SetFloat("shadow.cascades2.0", shadowCascadesIntervals2.x);
    archive->SetFloat("shadow.cascades2.1", shadowCascadesIntervals2.y);

    archive->SetFloat("shadow.cascades3.0", shadowCascadesIntervals3.x);
    archive->SetFloat("shadow.cascades3.1", shadowCascadesIntervals3.y);
    archive->SetFloat("shadow.cascades3.2", shadowCascadesIntervals3.z);

    archive->SetFloat("shadow.cascades4.0", shadowCascadesIntervals4.x);
    archive->SetFloat("shadow.cascades4.1", shadowCascadesIntervals4.y);
    archive->SetFloat("shadow.cascades4.2", shadowCascadesIntervals4.z);
    archive->SetFloat("shadow.cascades4.3", shadowCascadesIntervals4.w);

    archive->SetFloat("shadow.write.bias", shadowWriteBias.x);
    archive->SetFloat("shadow.write.bias_scale", shadowWriteBias.y);
    archive->SetFloat("shadow.filter.radius", shadowFilterRadius.x);
    archive->SetFloat("shadow.filter.elongation", shadowFilterRadius.y);

    archive->SetString("env.map", environmentMap.GetRelativePathname(serializationContext->GetScenePath()));
}

void Light::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    BaseObject::LoadObject(archive);

    lightType = eType(archive->GetInt32("type"));

    ambientColorDeprecated.r = archive->GetFloat("ambColor.r", ambientColorDeprecated.r);
    ambientColorDeprecated.g = archive->GetFloat("ambColor.g", ambientColorDeprecated.g);
    ambientColorDeprecated.b = archive->GetFloat("ambColor.b", ambientColorDeprecated.b);
    ambientColorDeprecated.a = archive->GetFloat("ambColor.a", ambientColorDeprecated.a);

    baseColor.r = archive->GetFloat("color.r", baseColor.r);
    baseColor.g = archive->GetFloat("color.g", baseColor.g);
    baseColor.b = archive->GetFloat("color.b", baseColor.b);
    baseColor.a = archive->GetFloat("color.a", baseColor.a);
    autoColor = archive->GetBool("autoColor", autoColor);

    intensityDeprecated = archive->GetFloat("intensity", intensityDeprecated);

    lightFlags = archive->GetUInt32("flags", lightFlags);

    String envMap = archive->GetString("env.map");
    if (!envMap.empty())
    {
        environmentMap = serializationContext->GetScenePath() + envMap;
    }

    shadowCascadesIntervals1 = archive->GetFloat("shadow.cascades1.0", shadowCascadesIntervals1);

    shadowCascadesIntervals2.x = archive->GetFloat("shadow.cascades2.0", shadowCascadesIntervals2.x);
    shadowCascadesIntervals2.y = archive->GetFloat("shadow.cascades2.1", shadowCascadesIntervals2.y);

    shadowCascadesIntervals3.x = archive->GetFloat("shadow.cascades3.0", shadowCascadesIntervals3.x);
    shadowCascadesIntervals3.y = archive->GetFloat("shadow.cascades3.1", shadowCascadesIntervals3.y);
    shadowCascadesIntervals3.z = archive->GetFloat("shadow.cascades3.2", shadowCascadesIntervals3.z);

    shadowCascadesIntervals4.x = archive->GetFloat("shadow.cascades4.0", shadowCascadesIntervals4.x);
    shadowCascadesIntervals4.y = archive->GetFloat("shadow.cascades4.1", shadowCascadesIntervals4.y);
    shadowCascadesIntervals4.z = archive->GetFloat("shadow.cascades4.2", shadowCascadesIntervals4.z);
    shadowCascadesIntervals4.w = archive->GetFloat("shadow.cascades4.3", shadowCascadesIntervals4.w);

    shadowFilterRadius.x = archive->GetFloat("shadow.filter.radius", shadowFilterRadius.x);
    shadowFilterRadius.y = archive->GetFloat("shadow.filter.elongation", shadowFilterRadius.y);
    shadowWriteBias.x = archive->GetFloat("shadow.write.bias", shadowWriteBias.x);
    shadowWriteBias.y = archive->GetFloat("shadow.write.bias_scale", shadowWriteBias.y);
    radius = archive->GetFloat("radius", 10.0f);
    aoRadius = archive->GetFloat("ao.radius", aoRadius);
}

void Light::SaveToYaml(const FilePath& presetPath, YamlNode* parentNode, const FilePath& realEnvMapPath)
{
    YamlNode* lightNode = new YamlNode(YamlNode::TYPE_MAP);
    parentNode->AddNodeToMap(Format("light%d", GetLightType()), lightNode);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(lightNode, "type", static_cast<int32>(GetLightType()));
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ambColor.r", ambientColorDeprecated.r);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ambColor.g", ambientColorDeprecated.g);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ambColor.b", ambientColorDeprecated.b);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ambColor.a", ambientColorDeprecated.a);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "color.r", baseColor.r);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "color.g", baseColor.g);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "color.b", baseColor.b);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "color.a", baseColor.a);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "intensity", intensityDeprecated);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "radius", radius);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "ao.radius", aoRadius);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(lightNode, "flags", lightFlags);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades1.0", shadowCascadesIntervals1);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades2.0", shadowCascadesIntervals2.x);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades2.1", shadowCascadesIntervals2.y);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades3.0", shadowCascadesIntervals3.x);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades3.1", shadowCascadesIntervals3.y);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades3.2", shadowCascadesIntervals3.z);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades4.0", shadowCascadesIntervals4.x);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades4.1", shadowCascadesIntervals4.y);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades4.2", shadowCascadesIntervals4.z);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.cascades4.3", shadowCascadesIntervals4.w);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.filter.radius", shadowFilterRadius.x);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.filter.elongation", shadowFilterRadius.y);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.write.bias", shadowWriteBias.x);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(lightNode, "shadow.write.bias_scale", shadowWriteBias.y);

    if (!realEnvMapPath.IsEmpty())
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<DAVA::String>(lightNode, "environmentMap", realEnvMapPath.GetFilename());
}

void Light::LoadFromYaml(const FilePath& presetPath, const YamlNode* parentNode)
{
    lightType = eType::TYPE_DIRECTIONAL;
    const YamlNode* typeNode = parentNode->Get("type");
    if (typeNode != nullptr)
        lightType = static_cast<eType>(typeNode->AsInt32());

    const YamlNode* abmColorDeprNodeR = parentNode->Get("ambColor.r");
    if (abmColorDeprNodeR != nullptr)
        ambientColorDeprecated.r = abmColorDeprNodeR->AsFloat();

    const YamlNode* abmColorDeprNodeG = parentNode->Get("ambColor.g");
    if (abmColorDeprNodeG != nullptr)
        ambientColorDeprecated.g = abmColorDeprNodeG->AsFloat();

    const YamlNode* abmColorDeprNodeB = parentNode->Get("ambColor.b");
    if (abmColorDeprNodeB != nullptr)
        ambientColorDeprecated.b = abmColorDeprNodeB->AsFloat();

    const YamlNode* abmColorDeprNodeA = parentNode->Get("ambColor.a");
    if (abmColorDeprNodeA != nullptr)
        ambientColorDeprecated.a = abmColorDeprNodeA->AsFloat();

    const YamlNode* baseColorNodeR = parentNode->Get("color.r");
    if (baseColorNodeR != nullptr)
        baseColor.r = baseColorNodeR->AsFloat();

    const YamlNode* baseColorNodeG = parentNode->Get("color.g");
    if (baseColorNodeG != nullptr)
        baseColor.g = baseColorNodeG->AsFloat();

    const YamlNode* baseColorNodeB = parentNode->Get("color.b");
    if (baseColorNodeB != nullptr)
        baseColor.b = baseColorNodeB->AsFloat();

    const YamlNode* baseColorNodeA = parentNode->Get("color.a");
    if (baseColorNodeA != nullptr)
        baseColor.a = baseColorNodeA->AsFloat();

    const YamlNode* intensityDeprecatedNode = parentNode->Get("intensity");
    if (intensityDeprecatedNode != nullptr)
        intensityDeprecated = intensityDeprecatedNode->AsFloat();

    const YamlNode* flagsNode = parentNode->Get("intensity");
    if (flagsNode != nullptr)
        lightFlags = static_cast<uint32>(flagsNode->AsInt32());

    auto GetCascade = [parentNode](const char* keyId, float& output) {
        const YamlNode* node = parentNode->Get(keyId);
        if (node != nullptr)
            output = node->AsFloat();
    };

    GetCascade("shadow.cascades1.0", shadowCascadesIntervals1);
    GetCascade("shadow.cascades2.0", shadowCascadesIntervals2.x);
    GetCascade("shadow.cascades2.1", shadowCascadesIntervals2.y);
    GetCascade("shadow.cascades3.0", shadowCascadesIntervals3.x);
    GetCascade("shadow.cascades3.1", shadowCascadesIntervals3.y);
    GetCascade("shadow.cascades3.2", shadowCascadesIntervals3.z);
    GetCascade("shadow.cascades4.0", shadowCascadesIntervals4.x);
    GetCascade("shadow.cascades4.1", shadowCascadesIntervals4.y);
    GetCascade("shadow.cascades4.2", shadowCascadesIntervals4.z);
    GetCascade("shadow.cascades4.2", shadowCascadesIntervals4.w);

    const YamlNode* shadowFilterRadiusNode = parentNode->Get("shadow.filter.radius");
    if (shadowFilterRadiusNode != nullptr)
        shadowFilterRadius.x = shadowFilterRadiusNode->AsFloat();

    const YamlNode* shadowFilterElongationNode = parentNode->Get("shadow.filter.elongation");
    if (shadowFilterElongationNode != nullptr)
        shadowFilterRadius.y = shadowFilterElongationNode->AsFloat();

    const YamlNode* shadowWriteBiasNode = parentNode->Get("shadow.write.bias");
    if (shadowWriteBiasNode != nullptr)
        shadowWriteBias.x = shadowWriteBiasNode->AsFloat();

    const YamlNode* shadowWriteBiasScaleNode = parentNode->Get("shadow.write.bias_scale");
    if (shadowWriteBiasScaleNode != nullptr)
        shadowWriteBias.y = shadowWriteBiasScaleNode->AsFloat();

    const YamlNode* radiusNode = parentNode->Get("radius");
    if (radiusNode != nullptr)
        radius = radiusNode->AsFloat();

    const YamlNode* aoRadiusNode = parentNode->Get("ao.radius");
    if (aoRadiusNode != nullptr)
        aoRadius = aoRadiusNode->AsFloat();

    const YamlNode* envMapNode = parentNode->Get("environmentMap");
    if (envMapNode != nullptr)
        environmentMap = presetPath.GetDirectory() + envMapNode->AsString();
}

void Light::SetIsDynamic(const bool& isDynamic)
{
    if (isDynamic)
    {
        AddFlag(DYNAMIC_LIGHT);
    }
    else
    {
        RemoveFlag(DYNAMIC_LIGHT);
    }
}

bool Light::GetIsDynamic()
{
    return (lightFlags & DYNAMIC_LIGHT) == DYNAMIC_LIGHT;
}

void Light::SetCastsShadow(const bool& castsShadow)
{
    if (castsShadow)
    {
        AddFlag(CASTS_SHADOW);
    }
    else
    {
        RemoveFlag(CASTS_SHADOW);
    }
}

bool Light::GetCastsShadow()
{
    return (lightFlags & CASTS_SHADOW) == CASTS_SHADOW;
}

void Light::AddFlag(uint32 flag)
{
    lightFlags |= flag;
}

void Light::RemoveFlag(uint32 flag)
{
    lightFlags &= ~flag;
}

uint32 Light::GetFlags()
{
    return lightFlags;
}

const Vector4& Light::CalculatePositionDirectionBindVector(Camera* inCamera)
{
    uint32 globalFrameIndex = Engine::Instance()->GetGlobalFrameIndex();
    if (inCamera != lastUsedCamera || lastUpdatedFrame != globalFrameIndex)
    {
        DVASSERT(inCamera);

        lastUsedCamera = inCamera;
        lastUpdatedFrame = globalFrameIndex;
        if (GetLightType() == TYPE_DIRECTIONAL)
        {
            // Here we prepare direction according to shader direction usage.
            // Shaders use it as ToLightDirection, so we have to invert it here
            resultPositionDirection = -direction;
            resultPositionDirection.w = 0.0f;
            resultPositionDirection.Normalize();
        }
        else
        {
            resultPositionDirection = position;
        }
    }
    return resultPositionDirection;
}

void Light::SetColorTemperature(float k)
{
    /*
     * https://www.shadertoy.com/view/lsSXW1
     * http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
     */
    float luminance = Max(1.0f, Max(baseColor.r, Max(baseColor.g, baseColor.b)));
    double temperatureInKelvins = Clamp(static_cast<double>(k), 1000.0, 40000.0) / 100.0;
    double cr = 1.0f;
    double cg = 1.0f;
    double cb = 1.0f;

    if (temperatureInKelvins <= 66.0)
    {
        cg = Clamp(0.39008157876901960784 * log(temperatureInKelvins) - 0.63184144378862745098, 0.0, 1.0);
    }
    else
    {
        double t = temperatureInKelvins - 60.0;
        cr = Clamp(1.29293618606274509804 * pow(t, -0.1332047592), 0.0, 1.0);
        cg = Clamp(1.12989086089529411765 * pow(t, -0.0755148492), 0.0, 1.0);
    }

    if (temperatureInKelvins <= 19.0)
        cb = 0.0;
    else if (temperatureInKelvins < 66.0)
        cb = Clamp(0.54320678911019607843 * log(temperatureInKelvins - 10.0) - 1.19625408914, 0.0, 1.0);

    SetColor(Color(luminance * static_cast<float>(cr), luminance * static_cast<float>(cg), luminance * static_cast<float>(cb), 1.0));
    lastSetColorTemperature = static_cast<float>(100.0 * temperatureInKelvins);
}

float Light::GetColorTemperature() const
{
    return lastSetColorTemperature;
}

Vector4 Light::GetShadowCascadesIntervals() const
{
    Vector4 result;
    switch (Renderer::GetRuntimeFlags().GetFlagValue(RuntimeFlags::Flag::SHADOW_CASCADES))
    {
    case 0:
        break;
    case 1:
        result = Vector4(shadowCascadesIntervals1, 0.0f, 0.0f, 0.0f);
        break;
    case 2:
        result = Vector4(shadowCascadesIntervals2.x, shadowCascadesIntervals2.y, 0.0f, 0.0f);
        break;
    case 3:
        result = Vector4(shadowCascadesIntervals3.x, shadowCascadesIntervals3.y, shadowCascadesIntervals3.z, 0.0f);
        break;
    case 4:
        result = shadowCascadesIntervals4;
        break;
    default:
        DVASSERT(0, "Invalid cascades count");
    }
    return result;
}

void Light::SetShadowCascadesIntervals(const Vector4& values)
{
    shadowCascadesIntervals1 = values.x;
    shadowCascadesIntervals2.x = values.x;
    shadowCascadesIntervals3.x = values.x;
    shadowCascadesIntervals4.x = values.x;

    shadowCascadesIntervals2.y = values.y;
    shadowCascadesIntervals3.y = values.y;
    shadowCascadesIntervals4.y = values.y;

    shadowCascadesIntervals3.z = values.z;
    shadowCascadesIntervals4.z = values.z;

    shadowCascadesIntervals4.w = values.w;
}

void Light::RecalcBoundingBox()
{
    if ((GetLightType() == Light::eType::TYPE_POINT) || (GetLightType() == Light::eType::TYPE_SPOT))
    {
        RenderObject::RecalcBoundingBox();
    }
    else
    {
        bbox = AABBox3(Vector3::Zero, 1.0f);
        worldBBox = bbox;
    }
}

void Light::RecalculateWorldBoundingBox()
{
    if ((GetLightType() == Light::eType::TYPE_POINT) || (GetLightType() == Light::eType::TYPE_SPOT))
    {
        RenderObject::RecalculateWorldBoundingBox();
    }
    else
    {
        bbox = AABBox3(Vector3::Zero, 1.0f);
        worldBBox = bbox;
    }
}
};
