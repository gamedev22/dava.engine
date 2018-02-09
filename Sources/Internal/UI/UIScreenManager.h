#pragma once

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

namespace DAVA
{
class UIScreen;

class UIScreenManager : public Singleton<UIScreenManager>
{
public:
    UIScreenManager();
    virtual ~UIScreenManager();

    void RegisterScreen(int screenId, UIScreen* screen);

    void SetFirst(int screenId);
    void SetScreen(int screenId);
    void ResetScreen();

    UIScreen* GetScreen(int screenId);
    UIScreen* GetScreen();
    int32 GetScreenId();

private:
    void ActivateGLController();

    struct Screen
    {
        enum eType
        {
            TYPE_NULL = 0,
            TYPE_SCREEN,
        };
        Screen(eType _type = TYPE_NULL, void* _value = 0)
        {
            type = _type;
            value = _value;
        }
        eType type;
        void* value;
    };

    Map<int, Screen> screens;
    int glControllerId;
    int activeControllerId;
    int activeScreenId;
};
};
