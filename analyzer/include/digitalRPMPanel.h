#ifndef DigitalRPMPanel_H_
#define DigitalRPMPanel_H_

#include "wx/wxprec.h"
#include <wx/timer.h>
#include <wx/valgen.h>

#include "LCDWindow.h"
#include "raceAnalyzerConfigBase.h"
#include "commonEvents.h"


#define REALTIME_PANEL_UPDATE_INTERVAL 100
class DigitalRPMPanel : public wxPanel{

	public:
		DigitalRPMPanel();
		DigitalRPMPanel(wxWindow *parent,
					wxWindowID id = -1,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "panel"
					);

		void SetValue(int value);
		void SetAccel(int accel);

		~DigitalRPMPanel();
		//event handlers
	private:

		void InitComponents();
		void InitOptions();

		LCDDisplay *_rpmLCD;


	DECLARE_EVENT_TABLE()
};


#endif /*DigitalRPMPanel_H_*/
