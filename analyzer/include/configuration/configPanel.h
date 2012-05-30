#ifndef CONFIGPANEL_H_
#define CONFIGPANEL_H_

#include "wx/wxprec.h"
#include "wx/treebook.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/analogInputPanel.h"
#include "configuration/pulseInputPanel.h"
#include "configuration/accelInputPanel.h"
#include "configuration/analogPwmOutputPanel.h"
#include "configuration/loggerOutputConfigPanel.h"
#include "configuration/gpioPanel.h"
#include "configuration/gpsConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"

class ConfigPanel : public wxPanel{

	public:
		ConfigPanel();
		ConfigPanel(wxWindow *parent,
					wxWindowID id = -1,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "panel"
					);


		~ConfigPanel();

		void SetComm(RaceAnalyzerComm *comm);

		//event handlers
	private:

		void OnReadConfig(wxCommandEvent &event);
		void OnWriteConfig(wxCommandEvent &event);
		void InitComponents();
		void InitOptions();

		wxPanel *m_configSettings;
		wxTreebook *m_configNavigation;
		AnalogInputPanel *m_analogInputPanel;
		PulseInputPanel *m_timerInputPanel;
		AccelInputPanel *m_accelInputPanel;
		AnalogPulseOutputPanel *m_analogPulseOutPanel;
		GpioPanel *m_gpioPanel;
		GpsConfigPanel *m_gpsPanel;
		LoggerOutputConfigPanel *m_loggerOutputPanel;

		RaceCaptureConfig m_raceCaptureConfig;

		RaceAnalyzerComm *m_comm;


	DECLARE_EVENT_TABLE()
};




#endif /* CONFIGPANEL_H_ */
