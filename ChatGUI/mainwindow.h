#pragma once

#include "UIlib.h"

#define BOT_MSG_PRINT_TIMER 20

using namespace DuiLib;

class MainWindow :
	public WindowImplBase
{
public:
	MainWindow();
	~MainWindow();

	virtual LPCTSTR GetWindowClassName() const;
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void Notify(TNotifyUI& msg);

	void StartConversation(void);

private:
	CListUI     *chat_box_;
	CRichEditUI *user_msg_edit_;
	CEditUI     *api_key_edit_;
	CEditUI     *proxy_edit_;
	CButtonUI   *close_btn_;
	CButtonUI   *send_msg_btn_;
	CButtonUI   *api_key_btn_;
	CButtonUI   *proxy_btn_;
	CButtonUI   *endown_btn_;

};

