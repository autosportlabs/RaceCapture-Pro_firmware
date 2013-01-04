
#ifndef LOGGEROUTPUTPANEL_H_
#define LOGGEROUTPUTPANEL_H_

#include "wx/wxprec.h"
#include "wx/treebook.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/channelConfigPanel.h"
#include "configuration/baseChannelConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"

class LoggerOutputConfigPanel :  public BaseConfigPanel{

	public:
		LoggerOutputConfigPanel();
		LoggerOutputConfigPanel(wxWindow *parent,
					wxWindowID id = -1,
					RaceCaptureConfig *config = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "LoggerOutputPanel"
					);

		~LoggerOutputConfigPanel();
		void OnConfigUpdated();
		void InitComponents();

	private:
		//event handlers
		void OnP2PAddressHighChanged(wxCommandEvent &event);
		void OnP2PAddressLowChanged(wxCommandEvent &event);
		void OnTelemetryModeChanged(wxCommandEvent &event);
		void OnLoggingModeChanged(wxCommandEvent &event);

		wxComboBox * m_sdLoggingModeCombo;
		wxComboBox * m_telemetryModeCombo;
		wxTextCtrl * m_p2pAddressHighTextCtrl;
		wxTextCtrl * m_p2pAddressLowTextCtrl;
		wxTextCtrl * m_telemetryServerTextCtrl;
		wxTextCtrl * m_deviceIdTextCtrl;


		DECLARE_EVENT_TABLE()
};




#endif /* LOGGEROUTPUTPANEL_H_ */
