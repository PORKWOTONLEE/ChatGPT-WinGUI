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
		kBotBubble

	}BubbleType;

	typedef enum
	{
		kNormalMetaMsg,
		kErrorMetaMsg,
		kWarningMetaMsg,

	}MetaMsgType;

	Bubble(CListUI *owner, BubbleType bubble_type = kUserBubble);
	~Bubble();

	void Notify(TNotifyUI& msg);
	void DoInit();
    SIZE EstimateSize(SIZE szAvailable);

	LPCWSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	void SetMsgText(CDuiString text);
	CDuiString GetMsgText(void);

	void SetMetaMsgText(LPCTSTR text, MetaMsgType type = kNormalMetaMsg);

private:
	CContainerUI *outline_container_;
	CRichEditUI  *msg_;
	CLabelUI     *meta_msg_;

	BubbleType bubble_type_;

};

