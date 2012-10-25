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

DECLARE_EVENT_TYPE ( REQUEST_DATALOG_DATA_EVENT, -1 )

class LineChartPane : public wxPanel, public RaceAnalyzerChannelView, public HistoricalView {

public:
	LineChartPane();
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
	void ConfigureChart(DatalogChannelSelectionSet *selectionSet);

	//from RaceAnalyzerChannelView
	void SetChartParams(ChartParams params);
	void SetBufferSize(wxString &channelName, size_t size);
	void UpdateValueRange(wxString &channel, size_t fromIndex, size_t toIndex, ChartValues &values);
	void UpdateValue(wxString &name, size_t index, double value);
	void ScrollLineChart(int thumbPosition);
	void SetOffset(int offset);

private:
	wxScrollBar *m_scrollBar;
	LineChart 	*m_lineChart;
	ChartParams	m_chartParams;

	DECLARE_EVENT_TABLE()
};

enum{

   ID_LOGVIEWER_SCROLL			= wxID_HIGHEST + 1,
   ID_REQUEST_DATALOG_DATA
};
#endif /* LOGVIEWER_H_ */
