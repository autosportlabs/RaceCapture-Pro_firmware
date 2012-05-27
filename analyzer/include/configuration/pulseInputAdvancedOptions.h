#ifndef PULSEINPUTADVANCEDOPTIONS_H_
#define PULSEINPUTADVANCEDOPTIONS_H_

#include "wx/wx.h"
#include "StripChart.h"
#include "wx/grid.h"
#include "raceCapture/raceCaptureConfig.h"
#include "controls/steppedSpinCtrl.h"

#define ADVANCED_PULSE_INPUT_SETTINGS_TITLE "Advanced Pulse Input Settings"

class PulseInputAdvancedDialog : public wxDialog
  {
  	DECLARE_CLASS( OptionsDialog )
  	DECLARE_EVENT_TABLE();

  	public:
  		PulseInputAdvancedDialog();

  		PulseInputAdvancedDialog(wxWindow* parent,
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

  		void SetConfig(TimerConfig *config);
  		TimerConfig GetConfig();
  		void CreateControls();

  	private:
  		void RefreshControls();

  		void OnTimerModeSelected(wxCommandEvent &event);
  		void OnTimerDividerSelected(wxCommandEvent &event);
  		void OnPulsePerRevChanged(wxCommandEvent &event);

  		int MapTimerDividerToCombo(int dividerValue);
  		int MapTimerDividerFromCombo(int selectedIndex);
  		TimerConfig m_config;
  		wxComboBox * m_timerModeCombo;
  		wxComboBox * m_timerDividerCombo;
  		SteppedSpinCtrl * m_pulsePerRevSpin;

  		bool m_dialogInitialized;
  };

enum{
	TIMER_MODE_COMBO = wxID_HIGHEST + 1,
	TIMER_DIVIDER_COMBO,
	PULSE_PER_REV_SPIN
};

#endif /* PULSEINPUTADVANCEDOPTIONS_H_ */
