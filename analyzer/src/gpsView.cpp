/*
 * gpsView.cpp
 *
 *  Created on: 2009-07-22
 *      Author: brent
 */

#include "gpsView.h"
#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
#include "logging.h"
#include <iostream>

WX_DEFINE_OBJARRAY(GPSPoints);


GPSView::GPSView(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxWindow(parent, id, pos, size,style,name)
    {
	ClearGPSPoints();

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

GPSView::~GPSView(){}

#define SCALE(P, MIN, MAX, VIEWSIZE) ((P - MIN) / (MAX - MIN)) * VIEWSIZE

void GPSView::OnPaint( wxPaintEvent& event )
{

	wxPaintDC old_dc(this);

	/////////////////
	// Create a memory DC
	/////////////////
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

	wxColor backColor = *wxBLACK; //GetBackgroundColour();
	dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(backColor,wxSOLID));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(backColor,wxSOLID));
	dc.Clear();

	dc.SetPen(*wxThePenList->FindOrCreatePen(*wxLIGHT_GREY, 1, wxSOLID));

	double lastX,lastY;
	int pointCount = m_gpsPoints.size();

	if (pointCount > 0){
		GPSPoint p = m_gpsPoints[0];
		lastX = SCALE(p.x, m_minX, m_maxX, _currentWidth);
		lastY = SCALE(p.y, m_minY, m_maxY, _currentHeight);
	}


    for (int i = 0; i < pointCount; i++){
    	GPSPoint p = m_gpsPoints[i];

    	double x = SCALE(p.x, m_minX, m_maxX, _currentWidth);
    	double y = SCALE(p.y, m_minY, m_maxY, _currentHeight);

    	dc.DrawLine(lastX, lastY, x, y);
		lastX = x;
		lastY = y;
    }


    dc.SetPen(*wxThePenList->FindOrCreatePen(*wxRED, 10, wxSOLID));
    double markerX = SCALE(m_marker.x, m_minX, m_maxX, _currentWidth);
    double markerY = SCALE(m_marker.y, m_minY, m_maxY, _currentHeight);

    dc.DrawCircle(wxPoint(markerX, markerY), 8);

	//blit into the real DC
	old_dc.Blit(0,0,_currentWidth,_currentHeight,&dc,0,0);
}

void GPSView::OnSize(wxSizeEvent& event)
{
	Refresh();
}

GPSPoint GPSView::GetMarker()
{
	return m_marker;
}

void GPSView::SetMarker(GPSPoint p)
{
	m_marker.x = p.x;
	m_marker.y = p.y;
	Refresh();
}


void GPSView::OnEnterWindow( wxMouseEvent& event )
{
    SetFocus();
    event.Skip();
}

void GPSView::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
  // Do nothing, to avoid flashing.
}

void GPSView::ClearGPSPoints(){
	m_gpsPoints.Clear();
	m_minX = 0;
	m_maxX = 0;
	m_minY = 0;
	m_maxY = 0;
	m_marker.x = 0;
	m_marker.y = 0;
	Refresh();
}

void GPSView::AddGPSPoint(GPSPoint point){
	if (m_minX == 0 || point.x < m_minX){
		m_minX = point.x;
	}
	if (m_minY == 0 || point.y < m_minY){
		m_minY = point.y;
	}
	if (m_maxX == 0 || point.x > m_maxX){
		m_maxX = point.x;
	}
	if (m_maxY == 0 || point.y > m_maxY){
		m_maxY = point.y;
	}
	m_gpsPoints.Add(point);
	Refresh();
}

BEGIN_EVENT_TABLE(GPSView, wxWindow)
    EVT_SIZE(GPSView::OnSize)
    EVT_PAINT(GPSView::OnPaint)
    EVT_ENTER_WINDOW(GPSView::OnEnterWindow)
    EVT_ERASE_BACKGROUND(GPSView::OnEraseBackground)
END_EVENT_TABLE()

