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
	void SetChartParams(ChartParams params);
	void InitComponents();

	void CreateGPSView(int datalogId, int latitudeChannelId, int longitudeChannelId);
	void SetOffset(size_t offset);
	void ReloadGPSPoints();


private:
	void AddGPSPoint(double latitude, double longitude);

	static const size_t		LATITUDE_INDEX = 0;
	static const size_t		LONGITUDE_INDEX = 1;

	GPSView				*m_gpsView;
	ChartParams			m_chartParams;
	DatalogStoreRows 	m_channelData;
	size_t				m_dataOffset;

	int					m_datalogId;
	int					m_longitudeChannelId;
	int					m_latitudeChannelId;

	DECLARE_EVENT_TABLE()
};



#endif /* GPSPANEL_H_ */
