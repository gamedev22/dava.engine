/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Notification/LocalNotificationController.h"
#include "Notification/LocalNotificationProgress.h"
#include "Notification/LocalNotificationText.h"

#include "Base/BaseTypes.h"
#include "Thread/LockGuard.h"

namespace DAVA
{

LocalNotificationController::~LocalNotificationController()
{
	LockGuard<Mutex> guard(notificationsListMutex);
    if (!notificationsList.empty())
    {
        for (typename List<LocalNotification *>::iterator it = notificationsList.begin(); it != notificationsList.end();)
        {
            LocalNotification *notification = (*it);
            it = notificationsList.erase(it);
            SafeRelease(notification);
        }
    }
}

LocalNotificationProgress *const LocalNotificationController::CreateNotificationProgress(const WideString &title, const WideString &text, const uint32 maximum, const uint32 current)
{
	LocalNotificationProgress *note = new LocalNotificationProgress();
    
    if (NULL != note)
    {
        note->SetText(text);
        note->SetTitle(title);
        note->SetProgressCurrent(current);
        note->SetProgressTotal(maximum);
        note->SetAction(Message());

        LockGuard<Mutex> guard(notificationsListMutex);
        notificationsList.push_back(note);
    }

    return note;
}

LocalNotificationText *const LocalNotificationController::CreateNotificationText(const WideString &title, const WideString &text)
{
	LocalNotificationText *note = new LocalNotificationText();

    if (NULL != note)
    {
        note->SetText(text);
        note->SetTitle(title);
        note->SetAction(Message());

        LockGuard<Mutex> guard(notificationsListMutex);
        notificationsList.push_back(note);
    }

    return note;
}

bool LocalNotificationController::Remove(LocalNotification *notification)
{
	LockGuard<Mutex> guard(notificationsListMutex);
    if (!notificationsList.empty())
    {
        for (typename List<LocalNotification *>::iterator it = notificationsList.begin(); it != notificationsList.end();)
        {
        	if (notification == (*it))
        	{
        		it = notificationsList.erase(it);
        		notification->Release();
        		return true;
        	}
        }
    }

    return false;
}

void LocalNotificationController::Clear()
{
    LockGuard<Mutex> guard(notificationsListMutex);
    if (!notificationsList.empty())
    {
        for (typename List<LocalNotification *>::iterator it = notificationsList.begin(); it != notificationsList.end(); ++it)
        {
            (*it)->Release();
            it = notificationsList.erase(it);
        }
    }
}
    
void LocalNotificationController::Update()
{
	LockGuard<Mutex> guard(notificationsListMutex);

    if (!notificationsList.empty())
    {
    	typename List<LocalNotification *>::const_iterator end = notificationsList.end();
        for (List<LocalNotification *>::const_iterator it = notificationsList.begin(); it != end; ++it)
        {
			(*it)->Update();
        }
    }
}

LocalNotification *const LocalNotificationController::GetNotificationById(const uint32 id)
{
	LockGuard<Mutex> guard(notificationsListMutex);
    if (!notificationsList.empty())
    {
    	typename List<LocalNotification *>::const_iterator end = notificationsList.end();
        for (List<LocalNotification *>::const_iterator it = notificationsList.begin(); it != end; ++it)
        {
        	DVASSERT(NULL != (*it));
        	if (id == (*it)->GetId())
        	{
        		return (*it);
        	}
        }
    }

    return NULL;
}

void LocalNotificationController::OnNotificationPressed(const uint32 id)
{
	LocalNotification *const notification = GetNotificationById(id);
	if (NULL != notification)
	{
		notification->RunAction();
	}
}

}

