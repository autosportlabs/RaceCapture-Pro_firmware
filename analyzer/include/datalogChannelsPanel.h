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
#include "commonEvents.h"
#include "appOptions.h"
#include "appPrefs.h"
#include "raceCapture/raceCaptureConfig.h"


class DatalogChannelsParams{
public:
	DatalogChannelsParams(
			RaceCaptureConfig *config,
			AppPrefs *prefs,
			AppOptions *options,
			DatalogStore *store) :
				raceCaptureConfig(config),
				appPrefs(prefs),
				appOptions(options),
				datalogStore(store)
	{}

	RaceCaptureConfig *raceCaptureConfig;
	AppPrefs *appPrefs;
	AppOptions *appOptions;
	DatalogStore *datalogStore;
};

class DatalogChannelsPanel : public wxPanel{

	public:
		DatalogChannelsPanel(DatalogChannelsParams params,
					wxWindow *parent,
					wxWindowID id = -1,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "panel"
					);

		void UpdateDatalogSessions();
		void AddDatalogSession(int id);
		void ReloadChannels(DatalogChannels &channels, DatalogChannelTypes &channelTypes, wxGrid *grid);
		void UpdateRuntimeValues();

		void SetMarkerOffset(size_t offset);
		size_t GetMarkerOffset();

		~DatalogChannelsPanel();
		//event handlers
	private:

		void InitComponents();
		void InitOptions();
		void PopulateSelectedChannels(DatalogChannelSelectionSet *selectionSet);
		void OnNewLineChart(wxCommandEvent &event);
		void OnNewAnalogGauge(wxCommandEvent &event);
		void OnNewDigitalGauge(wxCommandEvent &event);
		void OnNewGPSView(wxCommandEvent &event);
		void OnAddChannelView(wxCommandEvent &event);
		void DoGridContextMenu(wxGridEvent &event);
		void OnPlay(wxCommandEvent &event);
		void OnPause(wxCommandEvent &event);


		wxArrayInt		m_datalogIdList;
		wxNotebook 		*m_datalogSessionsNotebook;
		size_t 			m_markerOffset;
		DatalogStore 	*m_datalogStore;
		AppOptions		*m_appOptions;
		AppPrefs		*m_appPrefs;
		RaceCaptureConfig *m_raceCaptureConfig;
		wxMenu			*m_gridPopupMenu;


	DECLARE_EVENT_TABLE()
};

enum{

	ID_DATALOG_SESSIONS_NOTEBOOK = wxID_HIGHEST + 1,
	ID_DATALOG_CHANNELS_GRID,
	ID_NEW_LINE_CHART,
	ID_NEW_ANALOG_GAUGE,
	ID_NEW_DIGITAL_GAUGE,
	ID_NEW_GPS_VIEW,
	ID_ADD_CHANNEL,
	ID_PLAY_DATALOG,
	ID_PAUSE_DATALOG,
	ID_JUMP_BEGINNING_DATALOG,
	ID_JUMP_END_DATALOG

};



#endif /* DATALOGCHANNELSPANEL_H_ */
