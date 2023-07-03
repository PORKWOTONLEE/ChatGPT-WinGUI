#include "stdafx.h"
#include "openai.h"
#include "common.h"

BOOL   OpenAI::webinterfaceloop_start_ = TRUE;
HANDLE OpenAI::webinterface_thread_    = NULL;
OpenAI::WebinterfaceBuffer OpenAI::request_buffer_  = {NULL, 0, NULL, 0};
OpenAI::WebinterfaceBuffer OpenAI::response_buffer_ = {NULL, 0, NULL, 0};;
CURL     *OpenAI::curl_       = NULL;
CURLcode OpenAI::curl_result_ = CURLE_OK;

const OpenAI::WebinterfaceTask OpenAI::webinterface_tasklist_[OpenAI::kTaskAll] =
{
	{ kNoTask,     "",                                           NULL,                      NULL},
	{ kCompletion, "https://api.openai.com/v1/chat/completions", CompletionJsonConstructor, CompletionJsonParsor }
};
OpenAI::WebinterfaceTask OpenAI::current_task_ = webinterface_tasklist_[kNoTask];

OpenAI::OpenAI()
{
}

OpenAI::~OpenAI()
{
}

void OpenAI::InitOpenAI(void)
{
	curl_global_init(CURL_GLOBAL_ALL);

	webinterfaceloop_start_ = TRUE;

	webinterface_thread_ = ::CreateThread(NULL, NULL, WebinterfaceLoop, NULL, NULL, NULL);

	InitWebinterfaceBuffer(&request_buffer_);
	InitWebinterfaceBuffer(&response_buffer_);
}

void OpenAI::DeinitOpenAI(void)
{
	curl_global_cleanup();

	webinterfaceloop_start_ = FALSE;

	CloseHandle(webinterface_thread_);

	DeinitWebinterfaceBuffer(&request_buffer_);
	DeinitWebinterfaceBuffer(&response_buffer_);
}

void OpenAI::InitWebinterfaceBuffer(WebinterfaceBuffer * buffer)
{
	if (buffer->ptr == NULL)
	{
		buffer->ptr = new char[MAX_BUFFER_SIZE];
	}

	if (buffer->wptr == NULL)
	{
		buffer->wptr = new wchar_t[MAX_BUFFER_SIZE];
	}
}

void OpenAI::DeinitWebinterfaceBuffer(WebinterfaceBuffer * buffer)
{
	if (buffer->ptr != NULL)
	{
		delete buffer->ptr;
	}

	if (buffer->wptr != NULL)
	{
		delete buffer->wptr;
	}
}

void OpenAI::ClearWebinterfaceBuffer(WebinterfaceBuffer * buffer)
{
	if (buffer->ptr != NULL)
	{
		memset(buffer->ptr, '\0', MAX_BUFFER_SIZE);
		buffer->size = 0;
	}

	if (buffer->wptr != NULL)
	{
		memset(buffer->wptr, '\0', MAX_BUFFER_SIZE);
		buffer->wsize= 0;
	}
}

size_t OpenAI::ResponseBufferCallback(char * data, size_t size, size_t nmemb, void * response_buffer)
{
	size_t extra_size = size * nmemb;
	WebinterfaceBuffer *buffer = (WebinterfaceBuffer *)response_buffer;

	if (buffer->size + extra_size > MAX_BUFFER_SIZE)
	{
		return 0; // out of memory
	}
	
	// fill response buffer
	memcpy(&buffer->ptr[buffer->size], data, extra_size);
	buffer->size += extra_size;
	buffer->ptr[buffer->size + 1] = '\0';
	lstrcpyW(response_buffer_.wptr, current_task_.json_parsor(response_buffer_.ptr));
	response_buffer_.wsize = lstrlenW(response_buffer_.wptr);
	
	return extra_size;
}

void OpenAI::StartTask(TaskType task_type)
{
	// if running task
	if (current_task_.type != kNoTask)
	{
		return;
	}

	if (task_type != kNoTask)
	{
		SetTask(task_type);
	}
}

void OpenAI::SetTask(TaskType task_type)
{
	switch (task_type)
	{
	case kCompletion:
	case kNoTask:
		current_task_ = webinterface_tasklist_[task_type];
		break;
	default:
		current_task_ = webinterface_tasklist_[kCompletion];
		break;
	}
}

void OpenAI::EndTask(void)
{
	SetTask(kNoTask);
}

DWORD WINAPI OpenAI::WebinterfaceLoop(LPVOID param)
{
	while (webinterfaceloop_start_)
	{
		while (current_task_.type != kNoTask)
		{
			InitWebinterface();

			RequestProcess();

			ResponseProcess();

			DeinitWebinterface();

			EndTask();
		}
	}

	return 0;
}

void OpenAI::InitWebinterface(void)
{
	curl_ = curl_easy_init();
}

void OpenAI::RequestProcess()
{
	// fill request buffer
	lstrcpyW(request_buffer_.wptr, (wchar_t *)Common::GetInstance()->GetCurrentConversationUserBubble()->GetMsgText().GetData());
	request_buffer_.wsize = lstrlenW(request_buffer_.wptr);
	strcpy_s(request_buffer_.ptr, MAX_BUFFER_SIZE, current_task_.json_constructor(request_buffer_.wptr));
	request_buffer_.size = strlen(request_buffer_.ptr);

	// url
	curl_easy_setopt(curl_, CURLOPT_URL, current_task_.url);
	// ssl config
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, FALSE);
	// proxy
	if (Common::GetInstance()->LoadProxy().GetLength() != 0)
	{
		char proxy_ansi[100];
		Common::ConvertWcharToAnsi(Common::GetInstance()->LoadProxy(), proxy_ansi, 200);
		curl_easy_setopt(curl_, CURLOPT_PROXY, proxy_ansi);
		curl_easy_setopt(curl_, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
		curl_easy_setopt(curl_, CURLOPT_HTTPPROXYTUNNEL, 1L);
	}
	// http method
	curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "POST");
	// request header
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	char api_key_header[200];
	char api_key_ansi[100];
	Common::ConvertWcharToAnsi(Common::GetInstance()->LoadAPIKey(), api_key_ansi, 200);
	sprintf_s(api_key_header, "Authorization: Bearer %s", api_key_ansi);
	headers = curl_slist_append(headers, api_key_header);
	curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
	// post data
	curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, request_buffer_.ptr);
	// response fun & buffer
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, ResponseBufferCallback);
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void *)&response_buffer_);
	// repsonse code
	long response_code;
	curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);

	Common::GetInstance()->SetCurrentConversationStatus(kSending);

	// start connect
	curl_result_ = curl_easy_perform(curl_);
	if (curl_result_ == CURLE_OK)
	{
		Common::GetInstance()->SetCurrentConversationStatus(kSendSuccess);
	}
	else
	{
		Common::GetInstance()->SetCurrentConversationStatus(kSendFail);

		Common::GetInstance()->EndConversation();
	}
}

void OpenAI::ResponseProcess(void)
{
	if (curl_result_ != CURLE_OK)
	{
		return;
	}

	Common::GetInstance()->SetCurrentConversationStatus(kRecieving);

	Common::GetInstance()->GetCurrentConversationBotBubble()->SetMsgText(response_buffer_.wptr);
}

void OpenAI::DeinitWebinterface()
{
	curl_easy_cleanup(curl_);
	curl_result_ = CURLE_OK;

	ClearWebinterfaceBuffer(&request_buffer_);
	ClearWebinterfaceBuffer(&response_buffer_);
}

char *OpenAI::CompletionJsonConstructor(wchar_t *user_msg)
{
	//convert user_msg to utf8 code
	int user_msg_utf8_size = Common::Common::ConvertWcharToUtf8(user_msg);
	char *user_msg_utf8 = new char[user_msg_utf8_size];
	memset(user_msg_utf8, 0, user_msg_utf8_size);
	Common::ConvertWcharToUtf8(user_msg, user_msg_utf8, user_msg_utf8_size);

	// root 
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,  "model",    "gpt-3.5-turbo");
	// message 
	cJSON *message = cJSON_CreateArray();
	// completion 
	cJSON *current_user_msg = cJSON_CreateObject();
	// context
	if (Common::GetInstance()->GetPreviousConversationStatus() == kHistory)
	{
		char *previous_user_msg_ptr;
		int previous_user_msg_size;
		char *previous_bot_msg_ptr;
		int previous_bot_msg_size;

		previous_user_msg_size = Common::Common::ConvertWcharToUtf8((wchar_t *)Common::GetInstance()->GetPreviousConversationUserBubble()->GetMsgText().GetData());
	    previous_user_msg_ptr = new char[previous_user_msg_size];
		memset(previous_user_msg_ptr, 0, previous_user_msg_size);
		Common::ConvertWcharToUtf8((wchar_t *)Common::GetInstance()->GetPreviousConversationUserBubble()->GetMsgText().GetData(),previous_user_msg_ptr , previous_user_msg_size);

		cJSON *previous_user_msg = cJSON_CreateObject();
		cJSON_AddStringToObject(previous_user_msg, "role", "user");
		cJSON_AddStringToObject(previous_user_msg, "content", previous_user_msg_ptr);
		cJSON_AddItemToArray(message, previous_user_msg);

		previous_bot_msg_size = Common::Common::ConvertWcharToUtf8((wchar_t *)Common::GetInstance()->GetPreviousConversationBotBubble()->GetMsgText().GetData());
	    previous_bot_msg_ptr = new char[previous_bot_msg_size];
		memset(previous_bot_msg_ptr, 0, previous_bot_msg_size);
		Common::ConvertWcharToUtf8((wchar_t *)Common::GetInstance()->GetPreviousConversationUserBubble()->GetMsgText().GetData(),previous_bot_msg_ptr , previous_bot_msg_size);

		cJSON *previous_bot_msg  = cJSON_CreateObject();
		cJSON_AddStringToObject(previous_bot_msg, "role", "assistant");
		cJSON_AddStringToObject(previous_bot_msg, "content", previous_bot_msg_ptr);
		cJSON_AddItemToArray(message, previous_bot_msg);
	}
	cJSON_AddStringToObject(current_user_msg,  "role",    "user");
	cJSON_AddStringToObject(current_user_msg , "content", user_msg_utf8);
	cJSON_AddItemToArray(message, current_user_msg);
	cJSON_AddItemToObject(root , "messages", message);

	delete user_msg_utf8;

	// return json
	OutputDebugStringA(cJSON_Print(root));
	return cJSON_Print(root);
}

wchar_t * OpenAI::CompletionJsonParsor(char * json_buffer_utf8)
{
	size_t json_buffer_wide_size = 5000;
	wchar_t json_buffer_wide[5000];
	char json_buffer[5000];
	Common::ConvertUtf8ToWchar(json_buffer_utf8, json_buffer_wide, 5000);
	Common::ConvertWcharToAnsi(json_buffer_wide, json_buffer, 5000);

	cJSON *root    = cJSON_Parse(json_buffer);
	cJSON *choice  = NULL;
	cJSON *message = NULL;
	cJSON *role    = NULL;
	cJSON *content = NULL;

	int choice_size;

	choice = cJSON_GetObjectItem(root, "choices");
	choice_size = cJSON_GetArraySize(choice);
	if (choice != 0)
	{
		message = cJSON_GetObjectItem(cJSON_GetArrayItem(choice, 0), "message");
	}

	if (message != NULL)
	{
		role = cJSON_GetObjectItem(message, "role");
	}

	if (role != NULL) 
	{
		content = cJSON_GetObjectItem(message, "content");
	}

	if (content != NULL)
	{
		Common::ConvertAnisToWchar(content->valuestring, json_buffer_wide, 5000);
	}

	return json_buffer_wide;
}

