#pragma once

#include "curl.h"
#include "cJson.h"

#define MAX_BUFFER_SIZE 5000

class OpenAI 
{
public:
	typedef struct
	{
		char    *ptr;
		size_t  size;
		wchar_t *wptr;
		size_t  wsize;

	}WebinterfaceBuffer;

	typedef enum
	{
		kNoTask,
		// chat
		kCompletion,
		kTaskAll
		
	}TaskType;

	typedef struct
	{
		TaskType type;
		char     *url;
		char *(* json_constructor)(wchar_t *);
		wchar_t *(* json_parsor)(char *);
	}WebinterfaceTask;

	static const WebinterfaceTask webinterface_tasklist_[kTaskAll]; 

	OpenAI();
	~OpenAI();

	static void InitOpenAI(void);
	static void DeinitOpenAI(void);
	static void InitWebinterfaceBuffer(WebinterfaceBuffer *buffer);
	static void DeinitWebinterfaceBuffer(WebinterfaceBuffer *buffer);
	static void ClearWebinterfaceBuffer(WebinterfaceBuffer *buffer);
	static DWORD WINAPI WebinterfaceLoop(LPVOID param);
	static void InitWebinterface(void);
	static void RequestConfig(void);
	static void RequestProcess(void);
	static void ResponseProcess(void);
	static void DeinitWebinterface();
	static size_t ResponseBufferCallback(char * data, size_t size, size_t nmemb, void * response_buffer);

	static void SetTask(TaskType task_type);
	static void StartTask(TaskType task_type);
	static void EndTask(void);

	// completion
	static char *CompletionJsonConstructor(wchar_t * user_msg);
	static wchar_t *CompletionJsonParsor(char * json_buffer_utf8);

private:
	static BOOL                 webinterfaceloop_start_;
	static HANDLE			    webinterface_thread_;
	static WebinterfaceBuffer   request_buffer_;
	static WebinterfaceBuffer   response_buffer_;
	static CURL   *curl_;
	static CURLcode curl_result_;

	static WebinterfaceTask current_task_;

};

