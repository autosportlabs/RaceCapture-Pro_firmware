/*
 * DigitalGaugePane.h
 *
 *  Created on: 2009-07-20
 *      Author: brent
 */

#ifndef DigitalGaugePane_H_
#define DIGITALGAUGEPANE_H_

#include "wx/wxprec.h"
#include "lineChart.h"
#include "chartBase.h"
#include "LCDWindow.h"

class DigitalGaugePane : public wxPanel, public RaceAnalyzerChannelView {

public:
	DigitalGaugePane();
	void SetChartParams(ChartParams params);
	DigitalGaugePane(wxWindow *parent,
				wxWindowID id = -1,
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL,
				const wxString &name = "panel"
				);
	~DigitalGaugePane();
	void InitComponents();

	void CreateGauge(int datalogId, int channelId);
	void SetOffset(size_t offset);
	void RefreshGaugeValue();


private:
	LCDDisplay			*m_lcdDisplay;
	ChartParams			m_chartParams;
	DatalogStoreRows 	m_channelData;
	size_t				m_dataOffset;
	unsigned int		m_valuePrecision;

	DECLARE_EVENT_TABLE()
};

#endif /* DigitalGaugePane_H_ */
