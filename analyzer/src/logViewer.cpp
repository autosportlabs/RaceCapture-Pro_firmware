/*
 * logViewer.cpp
 *
 *  Created on: Jun 7, 2009
 *      Author: brent
 */
#include "logViewer.h"

#define SCROLLBAR_RANGE 10000
#define	SCROLLBAR_THUMBSIZE 100
#define SCROLLBAR_PAGESIZE 100


LogViewer::LogViewer() : wxPanel()
{
	InitComponents();
}

LogViewer::LogViewer(wxWindow *parent,
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

LogViewer::~LogViewer(){

}


void LogViewer::InitComponents(){

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

LineChart * LogViewer::GetLineChart(){
	return m_lineChart;
}

void LogViewer::OnScroll(wxScrollEvent &event){
	double pos = m_scrollBar->GetThumbPosition();

	double pct = pos / SCROLLBAR_RANGE;

	m_lineChart->SetViewOffsetPercent(pct);
	m_lineChart->Refresh();
}

BEGIN_EVENT_TABLE ( LogViewer, wxPanel )
	EVT_COMMAND_SCROLL (ID_LOGVIEWER_SCROLL,LogViewer::OnScroll)
END_EVENT_TABLE()
