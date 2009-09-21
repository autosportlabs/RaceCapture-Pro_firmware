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
	void SetChartParams(ChartParams params);
	AnalogGaugePane(wxWindow *parent,
				wxWindowID id = -1,
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL,
				const wxString &name = "panel"
				);
	~AnalogGaugePane();
	void InitComponents();

	void CreateGauge(int datalogId, int channelId);
	void SetOffset(size_t offset);
	void RefreshGaugeValue();


private:
	AngularMeter 		*m_angularMeter;
	ChartParams			m_chartParams;
	DatalogStoreRows 	m_channelData;
	size_t				m_dataOffset;

	DECLARE_EVENT_TABLE()
};

#endif /* ANALOGGAUGEPANE_H_ */
