/*
 * datalogChannelsPanel.h
 *
 *  Created on: Jun 9, 2009
 *      Author: brent
 */

#ifndef DATALOGCHANNELSPANEL_H_
#define DATALOGCHANNELSPANEL_H_

#include "wx/wxprec.h"
#include "wx/grid.h"
#include "wx/notebook.h"
#include "datalogStore.h"


class DatalogChannelsPanel : public wxPanel{

	public:
		DatalogChannelsPanel();
		DatalogChannelsPanel(wxWindow *parent,
					wxWindowID id = -1,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "panel"
					);

		void UpdateDatalogSessions();
		void AddDatalogSession(int id);
		void ReloadChannels(DatalogChannels &channels, DatalogChannelTypes &channelTypes, wxGrid *grid);
		void SetDatalogStore(DatalogStore *datalogStore);
		void UpdateRuntimeValues();

		void SetMarkerOffset(size_t offset);
		size_t GetMarkerOffset();

		~DatalogChannelsPanel();
		//event handlers
	private:

		void InitComponents();
		void InitOptions();

		wxNotebook 		*m_datalogSessions;
		DatalogStore 	*m_datalogStore;
		size_t 			m_markerOffset;


	DECLARE_EVENT_TABLE()
};

enum{

	ID_DATALOG_SESSIONS_NOTEBOOK = wxID_HIGHEST + 1,
	ID_DATALOG_CHANNELS_GRID

};

#endif /* DATALOGCHANNELSPANEL_H_ */
