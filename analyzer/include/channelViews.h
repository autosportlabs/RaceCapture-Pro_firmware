/*
 * channelViews.h
 *
 *  Created on: Aug 30, 2012
 *      Author: brent
 */

#ifndef CHANNELVIEWS_H_
#define CHANNELVIEWS_H_
#include "wx/wxprec.h"
#include "wx/dynarray.h"
#include "commonEvents.h"


class ChannelView{

public:
	ChannelView(wxString name, wxString description, wxEventType eventType, int eventId);

	wxString m_name;
	wxString m_description;
	wxEventType m_eventType;
	int m_eventId;
};

WX_DECLARE_OBJARRAY(ChannelView,ChannelViews);



#endif /* CHANNELVIEWS_H_ */
