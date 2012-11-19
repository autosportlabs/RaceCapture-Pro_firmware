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


class GPSPane : public wxPanel, public RaceAnalyzerChannelView, public HistoricalView {

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
	GPSPoint ProjectPoint(double latitude, double longitude);

	void CreateGPSView(int datalogId, wxString &latitudeChannelName, wxString &longitudeChannelName);
	void ClearGPSPoints();
	void UpdateValueRange(ViewDataHistoryArray &historyArray, size_t fromIndex, size_t toIndex);

	void SetChartParams(ChartParams params);
	void SetBufferSize(wxArrayString &channels, size_t size);

	void UpdateValue(wxString &name, size_t index, double value);

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
	double				m_currentLatitude;
	double 				m_currentLongitude;

	DECLARE_EVENT_TABLE()
};



#endif /* GPSPANEL_H_ */
