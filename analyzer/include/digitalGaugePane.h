/*
 * DigitalGaugePane.h
 *
 *  Created on: 2009-07-20
 *      Author: brent
 */

#ifndef DIGITALGAUGEPANE_H_
#define DIGITALGAUGEPANE_H_

#include "wx/wxprec.h"
#include "lineChart.h"
#include "chartBase.h"
#include "LCDWindow.h"

class DigitalGaugePane : public wxPanel, public RaceAnalyzerChannelView {

public:
	DigitalGaugePane();
	DigitalGaugePane(wxWindow *parent,
				wxWindowID id = -1,
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL,
				const wxString &name = "panel"
				);
	~DigitalGaugePane();
	void InitComponents();

	void CreateGauge(int datalogId, wxString channelName);

	//from RaceAnalyzerChannelView
	void SetChartParams(ChartParams params);
	void UpdateValue(wxString &name, size_t index, double value);

private:
	wxString			m_channelName;
	LCDDisplay			*m_lcdDisplay;
	ChartParams			m_chartParams;
	unsigned int		m_valuePrecision;

	DECLARE_EVENT_TABLE()
};

#endif /* DigitalGaugePane_H_ */
