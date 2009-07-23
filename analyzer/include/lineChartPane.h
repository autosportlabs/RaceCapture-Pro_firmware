/*
 * lineChartPane.h
 *
 *  Created on: Jun 7, 2009
 *      Author: brent
 */

#ifndef LOGVIEWER_H_
#define LOGVIEWER_H_

#include "wx/wxprec.h"
#include "lineChart.h"
#include "chartBase.h"


class LineChartPane : public wxPanel, public RaceAnalyzerChannelView {

public:
	LineChartPane();
	void SetChartParams(ChartParams params);
	LineChartPane(wxWindow *parent,
				wxWindowID id = -1,
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL,
				const wxString &name = "panel"
				);
	~LineChartPane();
	void InitComponents();
	LineChart * GetLineChart();

	void OnScroll(wxScrollEvent &event);
	void CreateChart(DatalogChannelSelectionSet *selectionSet);

private:
	wxScrollBar *m_scrollBar;
	LineChart 	*m_lineChart;
	ChartParams	m_chartParams;

	DECLARE_EVENT_TABLE()
};

enum{

   ID_LOGVIEWER_SCROLL			= wxID_HIGHEST + 1,

};
#endif /* LOGVIEWER_H_ */
