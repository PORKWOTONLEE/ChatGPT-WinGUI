#pragma once

#include "UIlib.h"

using namespace DuiLib;

class Bubble :
	public CListContainerElementUI,
	public INotifyUI
{
public:
	typedef enum
	{
		kUserBubble,
		kBotBubble,
		kInfoBubble

	}BubbleType;

	typedef enum
	{
		kNormalMetaMsg,
		kErrorMetaMsg,
		kInfoMetaMsg,

	}MetaMsgType;

	Bubble(CListUI *owner, BubbleType bubble_type = kUserBubble);
	~Bubble();

	void SetMsgText(CDuiString text);
	CDuiString GetMsgText(void);

	void SetMetaMsgText(LPCTSTR text, MetaMsgType type = kNormalMetaMsg);

private:
	void Notify(TNotifyUI& msg);
	void DoInit();
    SIZE EstimateSize(SIZE szAvailable);

	LPCWSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

private:
	CContainerUI *outline_container_;
	CRichEditUI  *msg_;
	CLabelUI     *meta_msg_;

	BubbleType bubble_type_;

	int line_height_;

	CDuiString bot_msg_print_buffer_;
	int        bot_msg_print_counter_{1};
};

