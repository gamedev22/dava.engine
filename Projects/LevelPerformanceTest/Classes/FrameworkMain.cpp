/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "DAVAEngine.h"
#include "GameCore.h"
 
using namespace DAVA;

#define IOS_WIDTH (960) // 960 2048 
#define IOS_HEIGHT (640) // 640 1536

void FrameworkDidLaunched()
{
    KeyedArchive * appOptions = new KeyedArchive();

#if defined(__DAVAENGINE_IPHONE__)
	appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);
    appOptions->SetInt32("renderer", Core::RENDERER_OPENGL_ES_2_0);
	
	DAVA::Core::Instance()->SetVirtualScreenSize(1024, 768);
	DAVA::Core::Instance()->RegisterAvailableResourceSize(1024, 768, "Gfx");

#elif defined(__DAVAENGINE_ANDROID__)

    appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);

    appOptions->SetInt32("width",  IOS_WIDTH);
    appOptions->SetInt32("height", IOS_HEIGHT);

    DAVA::Core::Instance()->SetVirtualScreenSize(IOS_WIDTH, IOS_HEIGHT);
    DAVA::Core::Instance()->SetProportionsIsFixed(false);
    DAVA::Core::Instance()->RegisterAvailableResourceSize(IOS_WIDTH, IOS_HEIGHT, "Gfx");

    appOptions->SetBool("Android_autodetectScreenScaleFactor", true);
    appOptions->SetFloat("phisicalToNativeControlsScaleFactor", 1.0f);
    appOptions->SetInt32("renderer", Core::RENDERER_OPENGL_ES_2_0);
	
#else
	appOptions->SetInt32("width",	1024);
	appOptions->SetInt32("height", 768);
	
	// 	appOptions->SetInt("fullscreen.width",	1280);
	// 	appOptions->SetInt("fullscreen.height", 800);
	
	appOptions->SetInt32("fullscreen", 0);
	appOptions->SetInt32("bpp", 32); 
	
	DAVA::Core::Instance()->SetVirtualScreenSize(1024, 768);
	DAVA::Core::Instance()->RegisterAvailableResourceSize(1024, 768, "Gfx");
#endif 
	
	GameCore * core = new GameCore();
	DAVA::Core::SetApplicationCore(core);
	DAVA::Core::Instance()->SetOptions(appOptions);
}


void FrameworkWillTerminate() 
{

}
