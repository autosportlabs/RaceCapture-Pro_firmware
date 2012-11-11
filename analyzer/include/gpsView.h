/*
 * gpsView.h
 *
 *  Created on: 2009-07-22
 *      Author: brent
 */

#ifndef GPSVIEW_H_
#define GPSVIEW_H_
#include <wx/wx.h>
#include <wx/dynarray.h>

#define POINT_SCALING	100000

class GPSPoint{
public:
	GPSPoint() :
		x(0),y(0),z(0),l(0),r(0)
		{}
	GPSPoint(double newX, double newY, double newZ, double newL, double newR) :
		x(newX), y(newY), z(newZ), l(newL), r(newR)
		{}
	double x;
	double y;
	double z;
	double l;
	double r;
};

WX_DECLARE_OBJARRAY(GPSPoint,GPSPoints);

class GPSView : public wxWindow
{
public:

	 GPSView( wxWindow *parent, wxWindowID id = wxID_ANY,
	        const wxPoint& pos = wxDefaultPosition,
	        const wxSize& size = wxDefaultSize,
	        long style = 0, const wxString& name = _T("GPSView") );

	    ~GPSView();
	    void OnSize(wxSizeEvent& event);
	    void OnPaint(wxPaintEvent& event);
	    void ClearGPSPoints();
	    void AddGPSPoint(GPSPoint point);
	    void OnEraseBackground(wxEraseEvent& event);
	    void OnEnterWindow( wxMouseEvent& event );
	    void SetMarker(GPSPoint p);
	    GPSPoint GetMarker();
private:
	    GPSPoints		m_gpsPoints;
	    GPSPoint		m_marker;
	    double			m_minX;
	    double			m_maxX;
	    double			m_minY;
	    double			m_maxY;
		int				_currentWidth;
		int				_currentHeight;
		wxBitmap 		*_memBitmap;

	    DECLARE_EVENT_TABLE()

};



#endif /* GPSVIEW_H_ */
