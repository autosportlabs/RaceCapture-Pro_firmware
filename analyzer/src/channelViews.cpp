/*
 * ChannelViews.cpp
 *
 *  Created on: Aug 30, 2012
 *      Author: brent
 */
#include "channelViews.h"

#include <wx/arrimpl.cpp> // This is a magic incantation which must be done!
 WX_DEFINE_OBJARRAY(ChannelViews);

ChannelView::ChannelView(wxString name, wxString description, wxEventType eventType, int eventId) :
		m_name(name), m_description(description), m_eventType(eventType), m_eventId(eventId)
{}




