#include "stdafx.h"
#include "common.h"
#include "openai.h"

Common *Common::instance_ = NULL;
HANDLE Common::mutex_ = ::CreateMutex(NULL, NULL, _T("CommonMutex"));

Common::Common()
{
}

Common * Common::GetInstance(void)
{
	if (instance_ == NULL)
	{
		// Lock mutex
		DWORD dwWaitResult = WaitForSingleObject(mutex_, INFINITE);
		if (dwWaitResult == WAIT_OBJECT_0)
		{
			if (instance_ == NULL)
			{
				Common *instance = new Common();
				instance_ = instance;
			}
			
			// Unlock mutex
			ReleaseMutex(mutex_);
		}
	}

	return instance_;
}

Common::~Common()
{
}

void Common::SetMainWindowHWND(HWND mainwin_hwnd)
{
	mainwin_hwnd_ = mainwin_hwnd;
}

HWND Common::GetMainWindowHWND(void)
{
	return mainwin_hwnd_;
}

CDuiString Common::LoadConfig(LPCWSTR key)
{
	TCHAR value[100];
	::GetPrivateProfileString(_T("ChatGUI"), key, _T("default"), value, 100, Common::GetInstance()->GetApplicationDir() + _T(".\\ChatGUI.ini"));

	return value;
}

void Common::SaveConfig(LPCWSTR key, LPCWSTR value)
{
	::WritePrivateProfileString(_T("ChatGUI"), key, value, GetApplicationDir() + _T(".\\ChatGUI.ini"));
}

void Common::SaveAPIKey(CDuiString api_key)
{
	SaveConfig(_T("API Key"), api_key);
}

CDuiString Common::LoadAPIKey(void)
{
	return	LoadConfig(_T("API Key"));
}

void Common::SaveProxy(CDuiString proxy)
{
	SaveConfig(_T("Proxy"), proxy);
}

CDuiString Common::LoadProxy(void)
{
	return LoadConfig(_T("Proxy"));
}

void Common::SaveContext(BOOL checked)
{
	if (checked)
	{
		SaveConfig(_T("Context"), _T("TRUE"));
	}
	else
	{
		SaveConfig(_T("Context"), _T("FALSE"));
	}
}

BOOL Common::LoadContext(void)
{
	return LoadConfig(_T("Context")) == _T("TRUE");
}

CDuiString Common::GetApplicationDir(void)
{
	// app location
	TCHAR app_path[MAX_PATH];
	::GetModuleFileName(NULL, app_path, MAX_PATH);
	::PathRemoveFileSpec(app_path);

	return app_path;
}

BOOL Common::StartConversation(CListUI * owner, CDuiString user_msg)
{
	if (current_conversation_.status == kIdle)
	{
		current_conversation_.user_bubble = new Bubble(owner, Bubble::BubbleType::kUserBubble);
		current_conversation_.bot_bubble = new Bubble(owner, Bubble::BubbleType::kBotBubble);

		// add bubble
		owner->Add(current_conversation_.user_bubble);
		owner->Add(current_conversation_.bot_bubble);

		// set user msg
		if (current_conversation_.user_bubble != NULL)
		{
			current_conversation_.user_bubble->SetMsgText(user_msg);
		}

		// send user msg throw curl
		OpenAI::StartTask(OpenAI::kCompletion);

		return TRUE;
	}

	return FALSE;
}

BOOL Common::EndConversation(BOOL force_end)
{
	if (current_conversation_.status==kSendFail || 
		current_conversation_.status==kRecieveSuccess || 
		force_end)
	{
		previous_conversation_.user_bubble = current_conversation_.user_bubble;
		previous_conversation_.bot_bubble  = current_conversation_.bot_bubble;
		previous_conversation_.status      = kHistory;

		current_conversation_.user_bubble = NULL;
		current_conversation_.bot_bubble  = NULL;
		current_conversation_.status      = kIdle;

		return TRUE;
	}

	return FALSE;
}

BOOL Common::StopConversation(void)
{
	if (current_conversation_.user_bubble != NULL)
	{
		((CListUI *)current_conversation_.user_bubble->GetOwner())->Remove(current_conversation_.user_bubble);

		delete current_conversation_.user_bubble;
		current_conversation_.user_bubble = NULL;
	}

	if (current_conversation_.bot_bubble!= NULL)
	{
		((CListUI *)current_conversation_.bot_bubble->GetOwner())->Remove(current_conversation_.bot_bubble);

		delete current_conversation_.bot_bubble;
		current_conversation_.bot_bubble= NULL;
	}

	return TRUE;
}

Bubble * Common::GetCurrentConversationUserBubble(void)
{
	return current_conversation_.user_bubble;
}

Bubble * Common::GetCurrentConversationBotBubble(void)
{
	return current_conversation_.bot_bubble;
}

void Common::SetCurrentConversationStatus(ConversationStatus status)
{
	switch (status)
	{
	case kSending:
		if (current_conversation_.status == kIdle)
		{
			current_conversation_.status = status;
			current_conversation_.user_bubble->SetMetaMsgText(_T("Sending"));
		}
		break;
	case kSendFail:
		if (current_conversation_.status == kSending)
		{
			current_conversation_.status = status;
			current_conversation_.user_bubble->SetMetaMsgText(_T("Sent Failed"), Bubble::kErrorMetaMsg);
		}
		break;
	case kSendSuccess:
		if (current_conversation_.status == kSending)
		{
			current_conversation_.status = status;
			current_conversation_.user_bubble->SetMetaMsgText(_T("Sent Successfully"));
		}
		break;
	case kRecieving:
		if (current_conversation_.status == kSendSuccess)
		{
			current_conversation_.status = status;
			current_conversation_.bot_bubble->SetMetaMsgText(_T("Recieving"));
		}
		break;
	case kRecieveSuccess:
		if (current_conversation_.status == kRecieving)
		{
			current_conversation_.status = status;
			current_conversation_.bot_bubble->SetMetaMsgText(_T("Recieved Successfully"));
		}
		break;
	default:
		break;
	}
}

ConversationStatus Common::GetCurrentConversationStatus(void)
{
	return current_conversation_.status;
}

Bubble * Common::GetPreviousConversationUserBubble(void)
{
	return previous_conversation_.user_bubble;
}

Bubble * Common::GetPreviousConversationBotBubble(void)
{
	return previous_conversation_.bot_bubble;
}

ConversationStatus Common::GetPreviousConversationStatus(void)
{
	return previous_conversation_.status;
}

int Common::ConvertWcharToAnsi(const wchar_t* src, char* dst, int dst_size)
{
	return WideCharToMultiByte(CP_ACP, 0, src, -1, dst, dst_size, NULL, NULL);
}

int Common::ConvertWcharToUtf8(const wchar_t* src, char* dst, int dst_size)
{
	return WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, dst_size, NULL, NULL);
}

int Common::ConvertAnisToWchar(const char* src, wchar_t* dst, int dst_size)
{
	return MultiByteToWideChar(CP_ACP, 0, src, -1, dst, dst_size);
}

int Common::ConvertUtf8ToWchar(const char* src, wchar_t* dst, int dst_size)
{
	return MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, dst_size);
}

