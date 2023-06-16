#include "stdafx.h"
#include "mainwindow.h"
#include "common.h"
#include "bubble.h"

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
}

LPCTSTR MainWindow::GetWindowClassName() const
{
	return _T("MainWindow");
}

CDuiString MainWindow::GetSkinFolder()
{
	return _T("./Skins");
}

CDuiString MainWindow::GetSkinFile()
{
	return _T("mainwindow.xml");
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
}

void MainWindow::Notify(TNotifyUI & msg)
{
	if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
	{
		chat_box_      = dynamic_cast<CListUI *>(m_PaintManager.FindControl(_T("chat_box")));
		user_msg_edit_ = dynamic_cast<CRichEditUI *>(m_PaintManager.FindControl(_T("user_msg_edit")));
		api_key_edit_  = dynamic_cast<CEditUI *>(m_PaintManager.FindControl(_T("api_key_edit")));
		proxy_edit_    = dynamic_cast<CEditUI *>(m_PaintManager.FindControl(_T("proxy_edit")));
		close_btn_     = dynamic_cast<CButtonUI *>(m_PaintManager.FindControl(_T("close_btn")));
		send_msg_btn_  = dynamic_cast<CButtonUI *>(m_PaintManager.FindControl(_T("send_msg_btn")));
		api_key_btn_   = dynamic_cast<CButtonUI *>(m_PaintManager.FindControl(_T("api_key_btn")));
		proxy_btn_     = dynamic_cast<CButtonUI *>(m_PaintManager.FindControl(_T("proxy_btn")));
		endown_btn_    = dynamic_cast<CButtonUI *>(m_PaintManager.FindControl(_T("endown_btn")));

		// set mainwin hwnd
		Common::GetInstance()->GetInstance()->SetMainWindowHWND(GetHWND());

		// show taskbar icon
		SetIcon(IDI_ICON1);

		// load user config 
		api_key_edit_->SetText(Common::GetInstance()->LoadAPIKey());
		proxy_edit_->SetText(Common::GetInstance()->LoadProxy());

		// hide the vscrollbar
		chat_box_->GetVerticalScrollBar()->SetAttribute(_T("width"), _T("0"));
	}
	else if (msg.sType == DUI_MSGTYPE_TEXTCHANGED)
	{
		if (msg.pSender == api_key_edit_)
		{
			Common::GetInstance()->SaveAPIKey(api_key_edit_->GetText());
		}
		else if (msg.pSender == proxy_edit_)
		{
			Common::GetInstance()->SaveProxy(proxy_edit_->GetText());
		}
	}
	else if (msg.sType == DUI_MSGTYPE_RETURN)
	{
		if (msg.pSender == api_key_edit_)
		{
			api_key_edit_->SetVisible(FALSE);
		}
		else if (msg.pSender == proxy_edit_)
		{
			proxy_edit_->SetVisible(FALSE);
		}
	}
	else if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender == close_btn_)
		{
			exit(0);
		}
		else if (msg.pSender == send_msg_btn_)
		{
			StartConversation();
		}
		else if (msg.pSender == api_key_btn_)
		{
			api_key_edit_->SetVisible(!api_key_edit_->IsVisible());
			proxy_edit_->SetVisible(FALSE);
		}
		else if (msg.pSender == proxy_btn_)
		{
			proxy_edit_->SetVisible(!proxy_edit_->IsVisible());
			api_key_edit_->SetVisible(FALSE);
		}
		else if (msg.pSender == endown_btn_)
		{
			chat_box_->EndDown();
		}
	}
}

void MainWindow::StartConversation(void)
{
	// ignore empty msg
	if (Common::GetInstance()->GetCurrentConversationStatus() != kIdle)
	{
		return;
	}

	// ignore empty msg
	if (user_msg_edit_->GetText().GetLength()==0)
	{
		return;
	}

	// please fill api key
	if (Common::GetInstance()->LoadAPIKey().GetLength() == 0)
	{
		api_key_edit_->SetVisible(TRUE);

		return;
	}

	// start conversation
	if (!Common::GetInstance()->StartConversation(chat_box_, user_msg_edit_->GetText()))
	{
		Common::GetInstance()->StopConversation();

		return;
	}
	
	user_msg_edit_->SetText(_T(""));
}

