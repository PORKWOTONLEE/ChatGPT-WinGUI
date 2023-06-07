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

void Common::SaveAPIKey(CDuiString api_key)
{
	api_key_ = api_key;

	::WritePrivateProfileString(_T("ChatGUI"), _T("API Key"), api_key, GetApplicationDir() + _T(".\\ChatGUI.ini"));
}

CDuiString Common::LoadAPIKey(void)
{
	TCHAR api_key[100];
	::GetPrivateProfileString(_T("ChatGUI"), _T("API Key"), _T(""), api_key, 100, Common::GetInstance()->GetApplicationDir() + _T(".\\ChatGUI.ini"));

	api_key_ = api_key;

	return api_key;
}

void Common::SaveProxy(CDuiString proxy)
{
	proxy_ = proxy;

	::WritePrivateProfileString(_T("ChatGUI"), _T("Proxy"), proxy, GetApplicationDir() + _T(".\\ChatGUI.ini"));
}

CDuiString Common::LoadProxy(void)
{
	TCHAR proxy[100];
	::GetPrivateProfileString(_T("ChatGUI"), _T("Proxy"), _T(""), proxy, 100, Common::GetInstance()->GetApplicationDir() + _T(".\\ChatGUI.ini"));

	proxy_ = proxy;

	return proxy_;
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

CDuiString Common::GetCurrentConversationUserText(void)
{
	return current_conversation_.user_bubble->GetMsgText();
}

BOOL Common::SetConversationBotMsg(wchar_t *bot_msg)
{
	if (current_conversation_.user_bubble->GetMsgText())
	{
		CDuiString exist_text = current_conversation_.bot_bubble->GetMsgText();
		exist_text.Append(bot_msg);
		current_conversation_.bot_bubble->SetMsgText(exist_text);
	}
	else
	{
		current_conversation_.bot_bubble->SetMsgText(bot_msg);
	}

	return TRUE;
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
			current_conversation_.user_bubble->SetMetaMsgText(_T("Send Failed"), Bubble::kErrorMetaMsg);
		}
		break;
	case kSendSuccess:
		if (current_conversation_.status == kSending)
		{
			current_conversation_.status = status;
			current_conversation_.user_bubble->SetMetaMsgText(_T("Send Successed"));
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
			current_conversation_.bot_bubble->SetMetaMsgText(_T("Recieve Successed"));
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

