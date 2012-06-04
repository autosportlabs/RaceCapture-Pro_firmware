#ifndef CONFIGPANEL_H_
#define CONFIGPANEL_H_

#include "wx/wxprec.h"
#include "wx/treebook.h"
#include "commonEvents.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/analogInputPanel.h"
#include "configuration/pulseInputPanel.h"
#include "configuration/accelInputPanel.h"
#include "configuration/analogPwmOutputPanel.h"
#include "configuration/loggerOutputConfigPanel.h"
#include "configuration/gpioPanel.h"
#include "configuration/gpsConfigPanel.h"
#include "comm.h"

class ConfigPanel : public wxPanel, public RaceAnalyzerCommCallback{

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

		void OnProgress(int pct);
		void ReadConfigComplete(bool success, wxString msg);
		void WriteConfigComplete(bool success, wxString msg);
		void FlashConfigComplete(bool success, wxString msg);


		//event handlers
	private:

		void OnReadConfig(wxCommandEvent &event);
		void OnWriteConfig(wxCommandEvent &event);
		void InitComponents();
		void InitOptions();
		void UpdateActivity(wxString msg);
		void UpdateStatus(wxString msg);


		wxPanel *m_configSettings;
		wxTreebook *m_configNavigation;
		wxCheckBox *m_alsoFlashConfigCheckBox;
		AnalogInputPanel *m_analogInputPanel;
		PulseInputPanel *m_timerInputPanel;
		AccelInputPanel *m_accelInputPanel;
		AnalogPulseOutputPanel *m_analogPulseOutPanel;
		GpioPanel *m_gpioPanel;
		GpsConfigPanel *m_gpsPanel;
		LoggerOutputConfigPanel *m_loggerOutputPanel;


		RaceCaptureConfig m_raceCaptureConfig;
		AsyncRaceAnalyzerComm * m_asyncComm;
		RaceAnalyzerComm *m_comm;


	DECLARE_EVENT_TABLE()
};




#endif /* CONFIGPANEL_H_ */
