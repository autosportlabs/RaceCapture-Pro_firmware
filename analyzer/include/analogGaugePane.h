/*
 * analogGaugePane.h
 *
 *  Created on: 2009-07-20
 *      Author: brent
 */

#ifndef ANALOGGAUGEPANE_H_
#define ANALOGGAUGEPANE_H_

#include "wx/wxprec.h"
#include "lineChart.h"
#include "chartBase.h"
#include "AngularMeter.h"

class AnalogGaugePane : public wxPanel, public RaceAnalyzerChannelView {

public:
	AnalogGaugePane();

	AnalogGaugePane(wxWindow *parent,
				wxWindowID id = -1,
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL,
				const wxString &name = "panel"
				);
	~AnalogGaugePane();
	void InitComponents();

	void CreateGauge(int datalogId, wxString channelName);

	//from RaceAnalyzerChannelView
	void SetChartParams(ChartParams params);
	void UpdateValue(wxString &channelName, size_t index, double value);


private:
	AngularMeter 		*m_angularMeter;
	ChartParams			m_chartParams;
	DatalogStoreRows 	m_channelData;
	int					m_dataOffset;
	wxString			m_channelName;

	DECLARE_EVENT_TABLE()
};

#endif /* ANALOGGAUGEPANE_H_ */
