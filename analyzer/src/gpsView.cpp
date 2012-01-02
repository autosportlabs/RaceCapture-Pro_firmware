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

void GPSView::OnPaint( wxPaintEvent& event )
{
	double minX = m_minX;
	double minY = m_minY;
	double halfX = (m_maxX - m_minX ) / 2;
	double halfY = (m_maxY - m_minY ) / 2;

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

	wxColor backColor = GetBackgroundColour();
	dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(backColor,wxSOLID));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(backColor,wxSOLID));
	dc.Clear();

	dc.SetPen(*wxThePenList->FindOrCreatePen(*wxLIGHT_GREY, 1, wxSOLID));

	double lastX,lastY;
	int pointCount = m_gpsPoints.size();

	if (pointCount > 0){
		GPSPoint p = m_gpsPoints[0];
		lastX = p.x;
		lastY = p.y;
	}

	double scaleX = _currentWidth / m_maxX;
	std::cout << "current width " << _currentWidth << " current height " << _currentHeight << "max x " << m_maxX << std::endl;

	std::cout << "scale " << scaleX << std::endl;


    for (int i = 0; i < pointCount; i++){
    	GPSPoint point = m_gpsPoints[i];
    	double x = point.x - minX;
    	double y = point.y - minY;
		//std::cout << "prescale x " << x << " y " << y << std::endl;
    	x = x * scaleX;
    	y = y * scaleX;

    	if (x < 0) x = - x;
    	if (y < 0) y = - y;


    	dc.DrawLine(lastX, lastY, x, y);
		lastX = x;
		lastY = y;
		//	std::cout << "scaled x " << x << " y " << y << std::endl;
    }

	//blit into the real DC
	old_dc.Blit(0,0,_currentWidth,_currentHeight,&dc,0,0);
}

void GPSView::OnSize(wxSizeEvent& event)
{
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
}

void GPSView::AddGPSPoint(GPSPoint point){
	if (m_minX == 0 || point.x < m_minX){
		m_minX = point.x;
	}
	if (m_minY == 0 || point.y < m_minY){
		m_minY = point.y;
	}
	if (m_maxY == 0 || point.y > m_maxY){
		m_maxY = point.y;
	}
	if (m_maxX == 0 || point.x > m_maxX){
		m_maxX = point.x;
	}
	m_gpsPoints.Add(point);


}

BEGIN_EVENT_TABLE(GPSView, wxWindow)
    EVT_SIZE(GPSView::OnSize)
    EVT_PAINT(GPSView::OnPaint)
    EVT_ENTER_WINDOW(GPSView::OnEnterWindow)
    EVT_ERASE_BACKGROUND(GPSView::OnEraseBackground)
END_EVENT_TABLE()

