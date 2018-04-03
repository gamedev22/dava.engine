#include "../Common/rhi_Private.h"
#include "../Common/rhi_Pool.h"
#include "rhi_GLES2.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
using DAVA::Logger;

#include "_gl.h"
#include "rhi_OpenGLState.h"

namespace rhi
{
//==============================================================================

struct DepthStencilStateGLES2_t
{
    struct StencilState
    {
        GLenum failOp;
        GLenum depthFailOp;
        GLenum depthStencilPassOp;
        GLenum func;
        uint32 writeMask;
        uint32 readMask;
        uint32 refValue;
    };

    uint32 depthTestEnabled : 1;
    GLuint depthMask;
    GLenum depthFunc;

    uint32 stencilEnabled : 1;
    uint32 stencilSeparate : 1;
    StencilState stencilFront, stencilBack;
};

typedef ResourcePool<DepthStencilStateGLES2_t, RESOURCE_DEPTHSTENCIL_STATE, DepthStencilState::Descriptor, false> DepthStencilStateGLES2Pool;
RHI_IMPL_POOL(DepthStencilStateGLES2_t, RESOURCE_DEPTHSTENCIL_STATE, DepthStencilState::Descriptor, false);

//------------------------------------------------------------------------------

static GLenum
_StencilOpGLES2(StencilOperation op)
{
    GLenum s = GL_KEEP;

    switch (op)
    {
    case STENCILOP_KEEP:
        s = GL_KEEP;
        break;
    case STENCILOP_ZERO:
        s = GL_ZERO;
        break;
    case STENCILOP_REPLACE:
        s = GL_REPLACE;
        break;
    case STENCILOP_INVERT:
        s = GL_INVERT;
        break;
    case STENCILOP_INCREMENT_CLAMP:
        s = GL_INCR;
        break;
    case STENCILOP_DECREMENT_CLAMP:
        s = GL_DECR;
        break;
    case STENCILOP_INCREMENT_WRAP:
        s = GL_INCR_WRAP;
        break;
    case STENCILOP_DECREMENT_WRAP:
        s = GL_DECR_WRAP;
        break;
    }

    return s;
}

//==============================================================================

static Handle gles2_DepthStencilState_Create(const DepthStencilState::Descriptor& desc)
{
    Handle handle = DepthStencilStateGLES2Pool::Alloc();
    DepthStencilStateGLES2_t* state = DepthStencilStateGLES2Pool::Get(handle);

    state->depthTestEnabled = desc.depthTestEnabled;
    state->depthMask = (desc.depthWriteEnabled) ? GL_TRUE : GL_FALSE;
    state->depthFunc = GetComparisonFunctionValue(CmpFunc(desc.depthFunc));

    state->stencilEnabled = desc.stencilEnabled;
    state->stencilSeparate = desc.stencilTwoSided;

    state->stencilFront.failOp = _StencilOpGLES2(StencilOperation(desc.stencilFront.failOperation));
    state->stencilFront.depthFailOp = _StencilOpGLES2(StencilOperation(desc.stencilFront.depthFailOperation));
    state->stencilFront.depthStencilPassOp = _StencilOpGLES2(StencilOperation(desc.stencilFront.depthStencilPassOperation));
    state->stencilFront.func = GetComparisonFunctionValue(CmpFunc(desc.stencilFront.func));
    state->stencilFront.readMask = desc.stencilFront.readMask;
    state->stencilFront.writeMask = desc.stencilFront.writeMask;
    state->stencilFront.refValue = desc.stencilFront.refValue;

    state->stencilBack.failOp = _StencilOpGLES2(StencilOperation(desc.stencilBack.failOperation));
    state->stencilBack.depthFailOp = _StencilOpGLES2(StencilOperation(desc.stencilBack.depthFailOperation));
    state->stencilBack.depthStencilPassOp = _StencilOpGLES2(StencilOperation(desc.stencilBack.depthStencilPassOperation));
    state->stencilBack.func = GetComparisonFunctionValue(CmpFunc(desc.stencilBack.func));
    state->stencilBack.readMask = desc.stencilBack.readMask;
    state->stencilBack.writeMask = desc.stencilBack.writeMask;
    state->stencilBack.refValue = desc.stencilBack.refValue;

    return handle;
}

//------------------------------------------------------------------------------

void gles2_DepthStencilState_Delete(Handle state)
{
    DepthStencilStateGLES2Pool::Free(state);
}

//==============================================================================

namespace DepthStencilStateGLES2
{
DepthStencilStateGLES2_t::StencilState _GLES_StencilFront;
DepthStencilStateGLES2_t::StencilState _GLES_StencilBack;
bool stencilStateValid = false;

void Init(uint32 maxCount)
{
    DepthStencilStateGLES2Pool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_DepthStencilState_Create = &gles2_DepthStencilState_Create;
    dispatch->impl_DepthStencilState_Delete = &gles2_DepthStencilState_Delete;
}

void InvalidateCache()
{
    stencilStateValid = false;
}

void SetToRHI(Handle hstate)
{
    DepthStencilStateGLES2_t* state = DepthStencilStateGLES2Pool::Get(hstate);

    glState.SetEnabled(OpenGLState::StateDepthTest, state->depthTestEnabled ? OpenGLState::Enabled : OpenGLState::Disabled);
    glState.SetEnabled(OpenGLState::StateDepthWrite, state->depthMask ? OpenGLState::Enabled : OpenGLState::Disabled);
    glState.SetEnabled(OpenGLState::StateStencilTest, state->stencilEnabled ? rhi::OpenGLState::Enabled : rhi::OpenGLState::Disabled);
    glState.SetDepthFunction(state->depthFunc);

    if (state->stencilEnabled && (stencilStateValid == false))
    {
        if (state->stencilSeparate)
        {
            if (memcmp(&_GLES_StencilFront, &(state->stencilFront), sizeof(DepthStencilStateGLES2_t::StencilState)))
            {
                GL_CALL(glStencilOpSeparate(GL_FRONT, state->stencilFront.failOp, state->stencilFront.depthFailOp, state->stencilFront.depthStencilPassOp));
                GL_CALL(glStencilFuncSeparate(GL_FRONT, state->stencilFront.func, state->stencilFront.refValue, state->stencilFront.readMask));
                GL_CALL(glStencilMaskSeparate(GL_FRONT, state->stencilFront.writeMask));
                memcpy(&_GLES_StencilFront, &(state->stencilFront), sizeof(DepthStencilStateGLES2_t::StencilState));
            }

            if (memcmp(&_GLES_StencilBack, &(state->stencilBack), sizeof(DepthStencilStateGLES2_t::StencilState)))
            {
                GL_CALL(glStencilOpSeparate(GL_BACK, state->stencilBack.failOp, state->stencilBack.depthFailOp, state->stencilBack.depthStencilPassOp));
                GL_CALL(glStencilFuncSeparate(GL_BACK, state->stencilBack.func, state->stencilBack.refValue, state->stencilBack.readMask));
                GL_CALL(glStencilMaskSeparate(GL_BACK, state->stencilBack.writeMask));
                memcpy(&_GLES_StencilBack, &(state->stencilBack), sizeof(DepthStencilStateGLES2_t::StencilState));
            }
        }
        else
        {
            if (memcmp(&_GLES_StencilFront, &(state->stencilFront), sizeof(DepthStencilStateGLES2_t::StencilState)) ||
                memcmp(&_GLES_StencilBack, &(state->stencilFront), sizeof(DepthStencilStateGLES2_t::StencilState)))
            {
                GL_CALL(glStencilOp(state->stencilFront.failOp, state->stencilFront.depthFailOp, state->stencilFront.depthStencilPassOp));
                GL_CALL(glStencilFunc(state->stencilFront.func, state->stencilFront.refValue, state->stencilFront.readMask));
                GL_CALL(glStencilMask(state->stencilFront.writeMask));
                memcpy(&_GLES_StencilFront, &(state->stencilFront), sizeof(DepthStencilStateGLES2_t::StencilState));
                memcpy(&_GLES_StencilBack, &(state->stencilFront), sizeof(DepthStencilStateGLES2_t::StencilState));
            }
        }

        stencilStateValid = true;
    }
}
}

//==============================================================================
} // namespace rhi
