#pragma once

#include "bubble.h"

typedef enum
{
	kHistory,
	kIdle,
	kSending,
	kSendSuccess,
	kSendFail,
	kRecieving,
	kRecieveSuccess,

}ConversationStatus;

typedef struct
{
	Bubble *user_bubble;
	Bubble *bot_bubble;
	ConversationStatus status;

}Conversation;

class Common
{
public:
	static Common *GetInstance(void);
	~Common();

	void SetMainWindowHWND(HWND mainwin_hwnd);
	HWND GetMainWindowHWND(void);
	CDuiString LoadConfig(LPCWSTR key);
	void SaveConfig(LPCWSTR key, LPCWSTR value);
	void SaveAPIKey(CDuiString api_key);
	CDuiString LoadAPIKey(void);
	void SaveProxy(CDuiString proxy);
	CDuiString LoadProxy(void);
	void SaveContext(BOOL checked);
	BOOL LoadContext(void);
	CDuiString GetApplicationDir(void);

	BOOL StartConversation(CListUI *owner, CDuiString user_msg);
	BOOL EndConversation(BOOL force_end = false);
	BOOL StopConversation(void);
	Bubble *GetCurrentConversationUserBubble(void);
	Bubble *GetCurrentConversationBotBubble(void);
	void SetCurrentConversationStatus(ConversationStatus status);
	ConversationStatus GetCurrentConversationStatus(void);
	Bubble *GetPreviousConversationUserBubble(void);
	Bubble *GetPreviousConversationBotBubble(void);
	ConversationStatus GetPreviousConversationStatus(void);

	static int ConvertWcharToAnsi(const wchar_t *src, char * dst = NULL, int dst_size = 0);
	static int ConvertWcharToUtf8(const wchar_t *src, char * dst = NULL, int dst_size = 0);
	static int ConvertAnisToWchar(const char * src, wchar_t * dst, int dst_size);
	static int ConvertUtf8ToWchar(const char * src, wchar_t * dst, int dst_size);

private:
	Common();

private:
	static HANDLE mutex_;
	static Common *instance_;

	HWND   mainwin_hwnd_{ NULL };

	Conversation current_conversation_{ NULL, NULL, kIdle };
	Conversation previous_conversation_{NULL, NULL, kIdle};
};

