/*
 * lineChartPane.cpp
 *
 *  Created on: Jun 7, 2009
 *      Author: brent
 */
#include "lineChartPane.h"
#include "datalogData.h"

#define SCROLLBAR_RANGE 10000
#define	SCROLLBAR_THUMBSIZE 100
#define SCROLLBAR_PAGESIZE 100


LineChartPane::LineChartPane() : wxPanel()
{
	InitComponents();
}

LineChartPane::LineChartPane(wxWindow *parent,
			wxWindowID id,
			const wxPoint &pos,
			const wxSize &size,
			long style,
			const wxString &name
			)
			: wxPanel(	parent,
						id,
						pos,
						size,
						style,
						name)
{
	InitComponents();
}

LineChartPane::~LineChartPane(){

}

void LineChartPane::ConfigureChart(DatalogChannelSelectionSet *selectionSet){

	LineChart *lineChart = GetLineChart();

	AppOptions *appOptions = m_chartParams.appOptions;
	ChartColors &chartColors = appOptions->GetChartColors();
	size_t maxColors = chartColors.Count();
	size_t currentColor = 0;

	lineChart->ClearAllSeries();

	size_t selCount = selectionSet->Count();
	for (size_t selIndex = 0; selIndex < selCount; selIndex++){
		DatalogChannelSelection &sel = selectionSet->Item(selIndex);

		wxArrayString &channelNames = sel.channelNames;

		for (size_t channelIndex = 0; channelIndex < channelNames.Count(); channelIndex++){

			wxString channelName = channelNames[channelIndex];

			DatalogChannelType channelType = appOptions->GetChannelTypeForChannel(channelName);

			Range *range = new Range(channelType.minValue,channelType.maxValue,channelType.unitsLabel);

			int newRangeId = lineChart->AddRange(range);

			Series *series = new Series(0, newRangeId, 0, channelName, chartColors[currentColor], channelType.precision);
			currentColor =  currentColor < maxColors - 1 ? currentColor + 1 : 0;
			lineChart->AddSeries(channelName, series);
		}
	}
}


void LineChartPane::SetChartParams(ChartParams params){
	m_chartParams = params;
}

void LineChartPane::SetBufferSize(wxArrayString &channels, size_t size){

	wxArrayString enabledChannels;
	for (wxArrayString::iterator it = channels.begin(); it != channels.end(); ++it){
		Series *series = m_lineChart->GetSeries(*it);
		if (NULL != series){
			enabledChannels.Add(*it);
			series->SetBufferSize(size);
		}
	}

	wxCommandEvent addEvent(REQUEST_DATALOG_DATA_EVENT, ID_REQUEST_DATALOG_DATA);
	RequestDatalogRangeParams *params = new RequestDatalogRangeParams(this, enabledChannels, 0, size - 1);
	addEvent.SetClientData(params);
	GetParent()->GetEventHandler()->AddPendingEvent(addEvent);
}

void LineChartPane::UpdateValueRange(ViewDataHistoryArray &historyArray, size_t fromIndex, size_t toIndex){

	for (size_t i = 0; i < historyArray.size(); i++){
		ViewDataHistory &history = historyArray[i];
		Series *series = m_lineChart->GetSeries(history.channelName);
		if (NULL != series){
			for (size_t i = fromIndex; i < toIndex; i++){
				series->SetValueAt(i, history.values[i]);
			}
			m_lineChart->Refresh();
		}
	}
}

void LineChartPane::UpdateValue(wxString &name, size_t index, double value){
	Series *series = m_lineChart->GetSeries(name);
	if (NULL != series){
		m_lineChart->SetMarkerIndex(index);
		int center = m_lineChart->GetSize().GetWidth() / 2;
		double adjustedIndex = (double)index - center;
		adjustedIndex = adjustedIndex >= 0 ? adjustedIndex : 0;
		size_t len = series->GetBufferSize();
		double factor = adjustedIndex / (double)len;
		double thumbPos = SCROLLBAR_RANGE * factor;
		m_scrollBar->SetThumbPosition(thumbPos);
		ScrollLineChart(thumbPos);
	}
}

void LineChartPane::ScrollLineChart(int thumbPosition){
	double pct = ((double)thumbPosition) / SCROLLBAR_RANGE;
	m_lineChart->SetViewOffsetFactor(pct);
}

void LineChartPane::SetOffset(int offset){

}

void LineChartPane::InitComponents(){

	wxFlexGridSizer *sizer = new wxFlexGridSizer(2,1,0,0);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);

	m_lineChart = new LineChart(this);
	m_lineChart->SetBackgroundColour(*wxBLACK);
	m_scrollBar = new wxScrollBar(this,ID_LOGVIEWER_SCROLL);

	m_scrollBar->SetScrollbar(0,SCROLLBAR_THUMBSIZE, SCROLLBAR_RANGE, SCROLLBAR_PAGESIZE,false);

	sizer->Add(m_lineChart,1,wxEXPAND);
	sizer->Add(m_scrollBar,1,wxEXPAND);
	this->SetSizer(sizer);

}

LineChart * LineChartPane::GetLineChart(){
	return m_lineChart;
}

void LineChartPane::OnScroll(wxScrollEvent &event){
	ScrollLineChart(m_scrollBar->GetThumbPosition());
}

BEGIN_EVENT_TABLE ( LineChartPane, wxPanel )
	EVT_COMMAND_SCROLL (ID_LOGVIEWER_SCROLL,LineChartPane::OnScroll)
END_EVENT_TABLE()
