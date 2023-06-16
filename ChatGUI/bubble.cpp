#include "stdafx.h"
#include "bubble.h"
#include "common.h"
#include "mainwindow.h"

Bubble::Bubble(CListUI *owner, BubbleType bubble_type) :
	bubble_type_(bubble_type)
{
	SetOwner(owner);
}

Bubble::~Bubble()
{
}

LPCWSTR Bubble::GetClass() const
{
	return L"Bubble";
}

LPVOID Bubble::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("Bubble")) == 0)
	{
		return static_cast<Bubble*>(this);
	}

	return CControlUI::GetInterface(pstrName);
}

void Bubble::SetMsgText(CDuiString text)
{
	msg_->SetText(text);

	((CListUI *)GetOwner())->EndDown();

	NeedParentUpdate();
}

CDuiString Bubble::GetMsgText(void)
{
	return msg_->GetText();
}

void Bubble::SetMetaMsgText(LPCTSTR text, MetaMsgType type)
{
	meta_msg_->SetText(text);

	switch (type)
	{
	case kNormalMetaMsg:
		meta_msg_->SetTextColor(0xFF70CC97);
		break;
	case kErrorMetaMsg:
		meta_msg_->SetTextColor(0xFFFF697D);
		break;
	case kWarningMetaMsg:
		meta_msg_->SetTextColor(0xFFFDE566);
		break;
	default:
		meta_msg_->SetTextColor(0xFF70CC97);
		break;
	}

	// update new text
	NeedParentUpdate();
}

void Bubble::DoInit() 
{
	CDialogBuilder builder;
	CContainerUI* bubble_style;

	// init bubble
	bubble_style = static_cast<CVerticalLayoutUI*>(builder.Create(_T("bubble.xml"), _T("xml"), NULL, GetManager()));
	if (bubble_style) 
	{
		this->Add(bubble_style);
		GetManager()->AddNotifier(this);
	}
	else 
	{
		this->RemoveAll();
	}

	GetManager()->SendNotify(this, DUI_MSGTYPE_WINDOWINIT);
}

SIZE Bubble::EstimateSize(SIZE szAvailable)
{
	// choose the font
	HDC  dc;
	HFONT font;
	dc = ::GetDC(Common::GetInstance()->GetMainWindowHWND());
	font = GetManager()->GetFont(msg_->GetFont());
	::SelectObject(dc, font);

	// max msg width limit
	int max_msg_width = 0;
	max_msg_width = szAvailable.cx / 3;

	// caculate msg size
	SIZE msg_check_sz = { 0 }, msg_sz = { 0 };
	int current_line_len = 0, next_line_len = 0, line_height = 0;
	CDuiPoint current_line_point, next_line_point;
	if (msg_->GetLineCount() > 1)
	{
		// multi line
		current_line_len = msg_->LineLength(1);
		next_line_len = msg_->LineLength(2);
		current_line_point = msg_->PosFromChar(current_line_len);
		next_line_point = msg_->PosFromChar(current_line_len + next_line_len);
		line_height = next_line_point.y - current_line_point.y;
		if (line_height == 0)
		{
			::GetTextExtentExPoint(dc, msg_->GetText(), lstrlenW(msg_->GetText()), max_msg_width, NULL, NULL, &msg_check_sz);
			if (msg_check_sz.cx > max_msg_width)
			{
				msg_sz.cx = max_msg_width;
				line_height = msg_check_sz.cy + 5;
				msg_sz.cy = line_height * (msg_check_sz.cx / max_msg_width + ((msg_check_sz.cx%max_msg_width > msg_check_sz.cy) ? 1 : 0));
			}
			else
			{
				msg_sz = msg_check_sz;
			}
		}
		else
		{
			msg_sz.cx = max_msg_width;
			msg_sz.cy = line_height * msg_->GetLineCount();
		}
	}
	else
	{
		// single line
		::GetTextExtentExPoint(dc, msg_->GetText(), lstrlenW(msg_->GetText()), max_msg_width, NULL, NULL, &msg_check_sz);
		if (msg_check_sz.cx > max_msg_width)
		{
			msg_sz.cx = msg_check_sz.cx;
			line_height = msg_check_sz.cy + 5;
			msg_sz.cy = line_height * (msg_check_sz.cx / max_msg_width + ((msg_check_sz.cx%max_msg_width > 2 * msg_check_sz.cy) ? 1 : 0));
		}
		else
		{
			msg_sz = msg_check_sz;
		}
	}

	// caculate meta msg size
	SIZE meta_msg_check_sz, meta_msg_sz;
	::GetTextExtentExPoint(dc, meta_msg_->GetText(), lstrlenW(meta_msg_->GetText()), max_msg_width, NULL, NULL, &meta_msg_check_sz);
	meta_msg_sz.cx = meta_msg_check_sz.cx;
	meta_msg_sz.cy = 30;

	// place msg & meta msg
	if (bubble_type_ == kUserBubble)
	{
		outline_container_->SetPos(RECT{ szAvailable.cx - msg_sz.cx - 20, 10, szAvailable.cx, msg_sz.cy + 14 + 10 });
		meta_msg_->SetPos(RECT{ szAvailable.cx - meta_msg_check_sz.cx, msg_sz.cy + 14 + 10, szAvailable.cx, msg_sz.cy + 14 + 10 + meta_msg_sz.cy });
	}
	else
	{
		outline_container_->SetPos(RECT{ 0, 10, msg_sz.cx + 20, msg_sz.cy + 14 + 10 });
		meta_msg_->SetPos(RECT{ 0, msg_sz.cy + 14 + 10, meta_msg_sz.cx, msg_sz.cy + 14 + 10 + meta_msg_sz.cy });
	}

	// caculate bubble size
	SIZE bubble_sz;
	bubble_sz.cx = szAvailable.cx;
	bubble_sz.cy = msg_sz.cy + meta_msg_sz.cy + 20;

	// clean temp value
	::ReleaseDC(Common::GetInstance()->GetMainWindowHWND(), dc);

	return bubble_sz;
}

void Bubble::Notify(TNotifyUI& msg)
{
	if (msg.pSender == this && msg.sType == DUI_MSGTYPE_WINDOWINIT)
	{
		// init some controlUI 
		outline_container_ = dynamic_cast<CContainerUI *>(GetManager()->FindControl(_T("outline_container")));
		msg_               = dynamic_cast<CRichEditUI *>(GetManager()->FindControl(_T("msg")));
		meta_msg_          = dynamic_cast<CLabelUI *>(GetManager()->FindControl(_T("meta_msg")));

		// set bubble style xml
		switch (bubble_type_)
		{
		case kUserBubble:
			outline_container_->SetBorderColor(0xFFBFCBD9);
			outline_container_->SetBkColor(0xFFFFFFFF);
			msg_->SetTextColor(0XFF333333);
			break;
		case kBotBubble:
			outline_container_->SetBorderColor(0xFFBFCBD9);
			outline_container_->SetBkColor(0xFF333333);
			msg_->SetTextColor(0XFFFFFFFF);
			break;
		default:
			outline_container_->SetBorderColor(0xFFBFCBD9);
			outline_container_->SetBkColor(0xFFFFFFFF);
			msg_->SetTextColor(0XFF333333);
			break;
		}
	}
}

