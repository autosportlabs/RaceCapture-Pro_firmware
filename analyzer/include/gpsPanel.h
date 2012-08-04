/*
 * gpsPanel.h
 *
 *  Created on: 2009-07-22
 *      Author: brent
 */

#ifndef GPSPANEL_H_
#define GPSPANEL_H_

#include "wx/wxprec.h"
#include "gpsView.h"
#include "chartBase.h"


class GPSPane : public wxPanel, public RaceAnalyzerChannelView {

public:
	GPSPane();
	GPSPane(wxWindow *parent,
				wxWindowID id = -1,
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL,
				const wxString &name = "panel"
				);
	~GPSPane();
	void InitComponents();

	void CreateGPSView(int datalogId, wxString &latitudeChannelName, wxString &longitudeChannelName);
	void SetOffset(size_t offset);
	void ReloadGPSPoints();


	//from RaceAnalyzerChannelView
	void SetChartParams(ChartParams params);
	void UpdateValue(wxString &name, float value);
	void SetOffset(int offset);

private:
	void AddGPSPoint(double latitude, double longitude);

	static const size_t		LATITUDE_INDEX = 0;
	static const size_t		LONGITUDE_INDEX = 1;

	GPSView				*m_gpsView;
	ChartParams			m_chartParams;
	DatalogStoreRows 	m_channelData;
	size_t				m_dataOffset;

	int					m_datalogId;
	wxString			m_longitudeChannelName;
	wxString 			m_latitudeChannelName;

	DECLARE_EVENT_TABLE()
};



#endif /* GPSPANEL_H_ */
