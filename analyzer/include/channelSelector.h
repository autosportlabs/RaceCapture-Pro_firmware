/*
 * channelSelectorDialog.h
 *
 *  Created on: 2009-07-08
 *      Author: brent
 */

#ifndef CHANNELSELECTORDIALOG_H_
#define CHANNELSELECTORDIALOG_H_
#include "wx/wxprec.h"
#include "wx/grid.h"
#include "datalogStore.h"
#include "datalogData.h"

class ChannelSelectorPanel: public wxPanel {

public:
	ChannelSelectorPanel();
	void SetDatalogStore(DatalogStore *store);
	void RefreshChannels();

	ChannelSelectorPanel(wxWindow *parent,
				wxWindowID id = -1,
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL,
				const wxString &name = "panel"
				);
	~ChannelSelectorPanel();
	void InitComponents();


private:
	wxGrid			*m_channelGrid;
	DatalogStore	*m_datalogStore;

	DECLARE_EVENT_TABLE()
};

#endif /* CHANNELSELECTORDIALOG_H_ */
