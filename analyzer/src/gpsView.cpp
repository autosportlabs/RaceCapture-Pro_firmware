/*
 * gpsView.cpp
 *
 *  Created on: 2009-07-22
 *      Author: brent
 */

#include "gpsView.h"
#include <wx/stdpaths.h>
#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
#include "logging.h"

WX_DEFINE_OBJARRAY(GPSPoints);


GPSView::GPSView(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
    {
	m_init = false;
	m_tx = -0.6f;
	    m_ty = -0.2f;
	    m_tz = -10;

	ClearGPSPoints();
    }

GPSView::~GPSView(){}

void GPSView::OnPaint( wxPaintEvent& event )
{
    Render();
}

void GPSView::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);

    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);

#ifndef __WXMOTIF__
    if (GetContext())
#endif
    {
        if (GetParent()->IsShown()) SetCurrent();
        glViewport(0, 0, (GLint) w, (GLint) h);
    }
}


void GPSView::InitGL()
{
    if (GetParent()->IsShown()) SetCurrent();

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


//	GLfloat LightAmbient[]= { 0.3f, 0.3f, 0.3f, 1.0f };
//	GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };
//	GLfloat LightSpecular[]= { 1.0f, 1.0f, 1.0f, 1.0f };
//	GLfloat LightPosition[]= { 0.0f, 0.0f, 1.0f, 1.0f };

//	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
//	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
//	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
//    glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);
	glEnable ( GL_COLOR_MATERIAL ) ;
 	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );







	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { -1.0, 0.0, 1.0, 0.0 };
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);









    info.beginx = 0.2f;
    info.beginy = 0.2f;
    //trackball( info.quat, -0.3f, 0.0f, 0.05f, -0.3f );
   trackball( info.quat, 0.0f, -10.0f, 0.0f, 0.0f );

	glfInit();
	glfStringCentering(GL_FALSE);
	glfSetRotateAngle(0.0f);

	//glfLoadFont("arial1.glf");
	//wxStandardPaths paths;
	//glfLoadFont(paths.GetDataDir() + "/" + "cricket1.glf");
	//glfLoadFont("compact1.glf");
}


void GPSView::Render()
{
    wxPaintDC dc(this);

#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif

    if (GetParent()->IsShown()) SetCurrent();
    // Init OpenGL once, but after SetCurrent
    if (!m_init)
    {
        InitGL();
        m_init = true;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.1f, 0.1f, -0.1f, 0.1f, 1.0f, 20.0f);
    //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 10.0f);
    glMatrixMode(GL_MODELVIEW);

    /* clear color and depth buffers */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Transformations
    GLfloat m[4][4];
    glLoadIdentity();
    glTranslatef( m_tx,m_ty,m_tz);
    build_rotmatrix( m,info.quat );
    glMultMatrixf( &m[0][0] );

    size_t pointCount = m_gpsPoints.Count();

	glColor3f(1.0,1.0,1.0);

	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.01,0,0);
	glVertex3f(0.01,0,0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(0,-0.01,0);
	glVertex3f(0,0.01,0);
	glEnd();

	glBegin(GL_LINE_LOOP);
	double minX = m_minX;
	double minY = m_minY;
	double halfX = (m_maxX - m_minX ) / 2;
	double halfY = (m_maxY - m_minY ) / 2;

    for (size_t i = 0; i < pointCount; i++){
    	GPSPoint point = m_gpsPoints[i];
    	double xVal = (point.x - minX - halfX );
    	double yVal = (point.y - minY - halfY);
    	glVertex3f(xVal ,yVal,0);

    }
	glEnd();

//	glPushMatrix();
	glColor3f(1.0f,0.0f,0.0f);
	GLUquadricObj *sphereObj;
	sphereObj=gluNewQuadric();


	gluSphere(sphereObj, 0.016f, 16, 16);
	glTranslatef(0.0,0.0,0.0);
	glScalef(0.05f, 0.05f, 0.05f);
//	glPopMatrix();



    glEndList();

    glFlush();
    SwapBuffers();
}

void GPSView::OnMouse( wxMouseEvent& event ){


	static long lastX = -1;
	static long lastY = -1;
	static long lastZ = -1;

	if (! event.Dragging()){
		lastX = -1;
		lastY = -1;
		lastZ = -1;
	}
	else
    {
	   if (event.ShiftDown()){
		   long x = event.GetX();
		   long y = event.GetY();
		   if (lastX >=0 && lastY >=0){
			   float xDelta = ((float)(x-lastX)) * 0.01;
			   float yDelta = ((float)(y-lastY)) * 0.01;

			   m_tx+=xDelta;
			   m_ty-=yDelta;
		   }
		   lastX = x;
		   lastY = y;

	   }
	   else if (event.ControlDown()){

		   long x = event.GetX();
		   long z = event.GetY();
		   if (lastX >=0 && lastZ >=0){
			   float xDelta = ((float)(x-lastX)) * 0.01;
			   float zDelta = ((float)(z-lastZ)) * 0.01;

			   m_tx+=xDelta;
			   m_tz-=zDelta;
		   }
		   lastX = x;
		   lastZ = z;
	   }
	   else{
	        wxSize sz( GetClientSize() );

	        /* drag in progress, simulate trackball */
	        float spin_quat[4];
	        trackball(spin_quat,
	            (2.0*info.beginx -       sz.x) / sz.x,
	            (     sz.y - 2.0*info.beginy) / sz.y,
	            (     2.0*event.GetX() - sz.x) / sz.x,
	            (    sz.y - 2.0*event.GetY()) / sz.y);

	        add_quats( spin_quat, info.quat, info.quat );

	   }
	//   VERBOSE(FMT("%3.3f,%3.3f,%3.3f",m_tx,m_ty,m_tz));
       /* orientation has changed, redraw mesh */
       Refresh(false);
    }

    info.beginx = event.GetX();
    info.beginy = event.GetY();

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

BEGIN_EVENT_TABLE(GPSView, wxGLCanvas)
    EVT_SIZE(GPSView::OnSize)
    EVT_PAINT(GPSView::OnPaint)
    EVT_ENTER_WINDOW(GPSView::OnEnterWindow)
    EVT_MOUSE_EVENTS(GPSView::OnMouse)
    EVT_ERASE_BACKGROUND(GPSView::OnEraseBackground)
END_EVENT_TABLE()

