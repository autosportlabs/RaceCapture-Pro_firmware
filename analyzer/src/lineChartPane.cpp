/*
 * lineChartPane.cpp
 *
 *  Created on: Jun 7, 2009
 *      Author: brent
 */
#include "lineChartPane.h"

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

void LineChartPane::CreateChart(DatalogChannelSelectionSet *selectionSet){

	DatalogStore *store = m_chartParams.datalogStore;
	LineChart *lineChart = GetLineChart();

	AppOptions *appOptions = m_chartParams.appOptions;
	ChartColors &chartColors = appOptions->GetChartColors();
	size_t maxColors = chartColors.Count();
	size_t currentColor = 0;


	size_t selCount = selectionSet->Count();
	for (size_t selIndex = 0; selIndex < selCount; selIndex++){
		DatalogChannelSelection &sel = selectionSet->Item(selIndex);

		int datalogId = sel.datalogId;
		DatalogChannels channels;
		store->GetChannels(datalogId, channels);

		wxArrayInt &channelIds = sel.channelIds;
		wxArrayString channelNames;
		size_t channelCount = channelIds.Count();
		for (size_t channelIndex = 0; channelIndex < channelCount; channelIndex++){
			int channelId = channelIds[channelIndex];
			channelNames.Add(channels[channelId].name);
		}
		DatalogChannelTypes channelTypes;
		store->GetChannelTypes(channelTypes);
		DatalogStoreRows	*datalogData = new DatalogStoreRows();
		store->ReadDatalog(*datalogData,datalogId,channelNames,0);

		for (size_t channelIndex = 0; channelIndex < channelCount; channelIndex++){
			int channelId = channelIds[channelIndex];
			DatalogChannel &channel = channels[channelId];
			DatalogChannelType &channelType = channelTypes[channel.typeId];

			Range *range = new Range(channelType.minValue,channelType.maxValue,channelType.unitsLabel);

			int newRangeId = lineChart->AddRange(range);
			Series *series = new Series(datalogData,channelIndex,newRangeId,0,channels[channelId].name,chartColors[currentColor]);
			currentColor =  currentColor < maxColors - 1 ? currentColor + 1 : 0;
			lineChart->AddSeries(series);
		}
	}
}


void LineChartPane::SetChartParams(ChartParams params){
	m_chartParams = params;
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
	double pos = m_scrollBar->GetThumbPosition();

	double pct = pos / SCROLLBAR_RANGE;

	m_lineChart->SetViewOffsetPercent(pct);
	m_lineChart->Refresh();
}

BEGIN_EVENT_TABLE ( LineChartPane, wxPanel )
	EVT_COMMAND_SCROLL (ID_LOGVIEWER_SCROLL,LineChartPane::OnScroll)
END_EVENT_TABLE()
