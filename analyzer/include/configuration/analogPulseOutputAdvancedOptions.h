#ifndef ANALOGPULSEOUTPUTADVANCEDOPTIONS_H_
#define ANALOGPULSEOUTPUTADVANCEDOPTIONS_H_

#include "wx/wx.h"
#include "StripChart.h"
#include "wx/grid.h"
#include "raceCapture/raceCaptureConfig.h"
#include "controls/steppedSpinCtrl.h"

#define ADVANCED_PULSE_INPUT_SETTINGS_TITLE "Advanced Pulse Input Settings"

class AnalogPulseOutputAdvancedDialog : public wxDialog
  {
  	DECLARE_CLASS( OptionsDialog )
  	DECLARE_EVENT_TABLE();

  	public:
  		AnalogPulseOutputAdvancedDialog();

  		AnalogPulseOutputAdvancedDialog(wxWindow* parent,
  			wxWindowID id = wxID_ANY,
  			const wxString& caption = wxT(ADVANCED_PULSE_INPUT_SETTINGS_TITLE),
  			const wxPoint& pos = wxDefaultPosition,
  			const wxSize& size = wxDefaultSize,
  			long style = wxCAPTION | wxSYSTEM_MENU );

  		bool Create (wxWindow* parent,
  			wxWindowID id = wxID_ANY,
  			const wxString& caption = wxT(ADVANCED_PULSE_INPUT_SETTINGS_TITLE),
  			const wxPoint& pos = wxDefaultPosition,
  			const wxSize& size = wxDefaultSize,
  			long style = wxCAPTION | wxSYSTEM_MENU );

  		void SetConfig(PwmConfig *config);
  		PwmConfig GetConfig();
  		void CreateControls();

  	private:
  		void RefreshControls();

  		void OnStartupDutyCycleChanged(wxCommandEvent &event);
  		void OnStartupPeriodChanged(wxCommandEvent &event);
  		void OnVoltageScalingChanged(wxCommandEvent &event);

  		SteppedSpinCtrl * m_startupDutyCycle;
  		SteppedSpinCtrl * m_startupPeriod;
  		wxTextCtrl *m_voltageScaling;

  		PwmConfig m_config;

  		bool m_dialogInitialized;
  };

enum{

	STARTUP_DUTY_CYCLE_SPIN= wxID_HIGHEST + 1,
	STARTUP_PERIOD_SPIN,
	VOLTAGE_SCALING_TEXT_CTRL
};

#endif /* ANALOGPULSEOUTPUTADVANCEDOPTIONS_H_ */
