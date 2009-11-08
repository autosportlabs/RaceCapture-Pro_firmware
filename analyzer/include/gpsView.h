/*
 * gpsView.h
 *
 *  Created on: 2009-07-22
 *      Author: brent
 */

#ifndef GPSVIEW_H_
#define GPSVIEW_H_
#include "wx/wxprec.h"
#include <wx/glcanvas.h>
#include <wx/dynarray.h>



extern "C"
{
	#include "glf.h"
	#include "trackball.h"
}

typedef struct
{
    float beginx,beginy;  /* position of mouse */
    float quat[4];        /* orientation of object */
} mesh_info;

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

class GPSView : public wxGLCanvas
{
public:

	 GPSView( wxWindow *parent, wxWindowID id = wxID_ANY,
	        const wxPoint& pos = wxDefaultPosition,
	        const wxSize& size = wxDefaultSize,
	        long style = 0, const wxString& name = _T("GPSView") );

	    ~GPSView();
	    void OnSize(wxSizeEvent& event);
	    void OnPaint(wxPaintEvent& event);
	    void Render();
	    void InitGL();
	    void ClearGPSPoints();
	    void AddGPSPoint(GPSPoint point);
	    void OnMouse( wxMouseEvent& event );
	    void OnEraseBackground(wxEraseEvent& event);
	    void OnEnterWindow( wxMouseEvent& event );
private:
	    GLuint 			m_gllist;
	    bool			m_init;
	    float 			m_tx,m_ty,m_tz;
	    mesh_info 		info;
	    GPSPoints		m_gpsPoints;
	    double			m_minX;
	    double			m_maxX;
	    double			m_minY;
	    double			m_maxY;

	    DECLARE_EVENT_TABLE()

};



#endif /* GPSVIEW_H_ */
