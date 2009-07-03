/*
 * lineChart.cpp
 *
 *  Created on: May 24, 2009
 *      Author: brent
 */
#include "lineChart.h"
#include "logging.h"

#define UNITS_LABEL_SPACING 	2
#define GRID_SIZE 				20
#define DEFAULT_ZOOM 			100
#define MIN_ZOOM				25
#define DEFAULT_OFFSET_SECONDS	0
#define DEFAULT_MIN_VALUE		0
#define DEFAULT_MAX_VALUE		100

BEGIN_EVENT_TABLE( LineChart, wxWindow )
	EVT_PAINT( LineChart::OnPaint )
	EVT_SIZE( LineChart::OnSize )
	EVT_MOTION(LineChart::OnMouseMove)
	EVT_ENTER_WINDOW(LineChart::OnMouseEnter)
	EVT_LEAVE_WINDOW(LineChart::OnMouseExit)
    EVT_ERASE_BACKGROUND(LineChart::OnEraseBackground)
END_EVENT_TABLE()

#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
WX_DEFINE_OBJARRAY(RangeArray);
WX_DEFINE_OBJARRAY(SeriesArray);


LineChart::LineChart(): wxWindow(),
	_zoomPercentage(DEFAULT_ZOOM),
	_showScale(true)
{}

LineChart::LineChart(		wxWindow *parent,
							wxWindowID id,
							const wxPoint &pos,
							const wxSize &size)
							: wxWindow(parent, id, pos, size),
								m_viewOffsetPercent(0),
								_zoomPercentage(DEFAULT_ZOOM),
								_showScale(true),
								_showData(false)


{
	if (parent){
		SetBackgroundColour(parent->GetBackgroundColour());
	}
	else{
		SetBackgroundColour(*wxBLACK);
	}
	_currentWidth = size.GetWidth();
	_currentHeight = size.GetHeight();
	_memBitmap = new wxBitmap(_currentWidth, _currentHeight);
}


LineChart::~LineChart(){
	delete (_memBitmap);
}

void LineChart::OnMouseEnter(wxMouseEvent &event){

	_showData = true;
	_mouseX = event.GetX();
	_mouseY = event.GetY();
	Refresh();
}

void LineChart::OnMouseMove(wxMouseEvent &event){

	_mouseX = event.GetX();
	_mouseY = event.GetY();
	Refresh();
}

void LineChart::OnMouseExit(wxMouseEvent &event){

	_showData = false;
	Refresh();
}

void LineChart::SetViewOffsetPercent(double offset){
	m_viewOffsetPercent = offset;
}

double LineChart::GetViewOffsetPercent(){
	return m_viewOffsetPercent;
}

void LineChart::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
  // Do nothing, to avoid flashing.
}

int LineChart::GetZoom(){
	return _zoomPercentage;
}

void LineChart::SetZoom(int zoomPercentage){
	_zoomPercentage = zoomPercentage >= MIN_ZOOM ? zoomPercentage : MIN_ZOOM;
	Refresh();
}

void LineChart::ShowScale(bool showScale){
	_showScale = showScale;
}

bool LineChart::GetShowScale(){
	return _showScale;
}

void LineChart::OnSize(wxSizeEvent &event){
	Refresh();
}


void LineChart::OnPaint(wxPaintEvent &event){

	wxPaintDC old_dc(this);

	float zoomFactor = (float)_zoomPercentage / 100;

	int w,h ;
	GetClientSize(&w,&h);

	if (w != _currentWidth || h != _currentHeight){
		delete (_memBitmap);
		_currentWidth = w;
		_currentHeight = h;
		_memBitmap = new wxBitmap(_currentWidth, _currentHeight);
	}
	/////////////////
	// Create a memory DC
	wxMemoryDC dc;
	dc.SelectObject(*_memBitmap);

	wxColor backColor = GetBackgroundColour();
	dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(backColor,wxSOLID));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(backColor,wxSOLID));
	dc.Clear();
	DrawGrid(dc);

	if (_showScale) DrawScale(dc);


	size_t seriesLen = m_seriesArray.size();

	for (size_t sId = 0; sId < seriesLen; sId++){

		float currentX = (float)0;
		int lastX = (int)currentX;
		int lastY;
		Series *series = m_seriesArray[sId];
		size_t col = series->GetBufferCol();
		dc.SetPen(*wxThePenList->FindOrCreatePen(series->GetColor(), 1, wxSOLID));
		DatalogStoreRows *buf = series->GetDataBuffer();
		size_t bufSize = buf->GetCount();
		Range *range = m_rangeArray[series->GetRangeId()];
		if (bufSize > 0){

			double minValue = range->GetMin();
			double maxValue = range->GetMax();

			double loggedValue = (*buf)[0].values[col];

			double percentageOfMax = (loggedValue - minValue) / (maxValue - minValue);
			lastY = h - (int)(((double)h) * percentageOfMax);

			size_t i = (size_t)(((double)bufSize) * m_viewOffsetPercent);

			while (i < bufSize && currentX < _currentWidth ){
				double loggedValue = (*buf)[i].values[col];

				double percentageOfMax = (loggedValue - minValue) / (maxValue - minValue);

				int y = h - (int)(((double)h) * percentageOfMax);

				dc.DrawLine(lastX, lastY, (int)currentX, y);
				lastX = (int)currentX;
				lastY = y;
				currentX += zoomFactor;
				i++;
			}
		}
	}


	//blit into the real DC
	old_dc.Blit(0,0,_currentWidth,_currentHeight,&dc,0,0);

}


void LineChart::DrawScale(wxMemoryDC &dc){

}

void LineChart::DrawGrid(wxMemoryDC &dc){

	dc.SetPen(*wxThePenList->FindOrCreatePen(wxColor(40,40,40), 1, wxSOLID));

	int width = _currentWidth;
	int height = _currentHeight;

	float zoomFactor = (float)_zoomPercentage / 100;

	int gridIncrement = (int)(GRID_SIZE * zoomFactor);

	for (int x = width; x >=0 ; x -= gridIncrement){
		dc.DrawLine(x, 0, x, height);
	}

	float i = 0;
	while (i < 1){
		int y = (int)(((float)height) * i);
		dc.DrawLine(0, y, width, y);
		i = i + 0.1;
	}
}


int LineChart::AddRange(Range *range){
	m_rangeArray.Add(range);
	return m_rangeArray.size() - 1;
}

int LineChart::AddSeries(Series *series){
	m_seriesArray.Add(series);
	return m_seriesArray.size() - 1;
}

