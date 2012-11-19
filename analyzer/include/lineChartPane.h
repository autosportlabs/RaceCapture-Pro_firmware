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
	void SetBufferSize(wxArrayString &channelNames, size_t size);
	void UpdateValueRange(ViewDataHistoryArray &historyArray, size_t fromIndex, size_t toIndex);
	void UpdateValue(wxString &name, size_t index, double value);
	void ScrollLineChart(int thumbPosition);
	void SetOffset(int offset);

private:
	wxScrollBar *m_scrollBar;
	LineChart 	*m_lineChart;
	ChartParams	m_chartParams;

	DECLARE_EVENT_TABLE()
};

#endif /* LOGVIEWER_H_ */
