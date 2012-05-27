#include "configuration/pulseInputAdvancedOptions.h"
#include "raceCapture/raceCaptureConfig.h"

IMPLEMENT_CLASS (PulseInputAdvancedDialog, wxDialog)

PulseInputAdvancedDialog::PulseInputAdvancedDialog() : m_dialogInitialized(false)  {}

PulseInputAdvancedDialog::PulseInputAdvancedDialog(wxWindow* parent,
  			wxWindowID id,
  			const wxString& caption,
  			const wxPoint& pos,
  			const wxSize& size,
  			long style) : m_dialogInitialized(false){
	Create(parent,id,caption,pos,size,style);
}

bool PulseInputAdvancedDialog::Create (wxWindow* parent,
  			wxWindowID id,
  			const wxString& caption,
  			const wxPoint& pos,
  			const wxSize& size,
  			long style){

 	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

 	if (! wxDialog::Create (parent, id, caption, pos, size, style)) return false;

 	CreateControls();

 	GetSizer()->Fit(this);
 	GetSizer()->SetSizeHints(this);
 	Center();

 	RefreshControls();
 	m_dialogInitialized = true;
 	return true;
}

void PulseInputAdvancedDialog::RefreshControls(){

	m_timerModeCombo->Select(m_config.mode);

	m_pulsePerRevSpin->SetValue(m_config.pulsePerRev);

	m_timerDividerCombo->Select(MapTimerDividerToCombo(m_config.timerDivider));

}

int PulseInputAdvancedDialog::MapTimerDividerToCombo(int dividerValue){
	switch(dividerValue){
		case 2:
			return 0;
		case 8:
			return 1;
		case 32:
			return 2;
		case 128:
			return 3;
		case 1024:
			return 4;
		default:
			return 0;
	}
}

int PulseInputAdvancedDialog::MapTimerDividerFromCombo(int selectedIndex){
	switch(selectedIndex){
		case 0:
			return 2;
		case 1:
			return 8;
		case 2:
			return 32;
		case 3:
			return 128;
		case 4:
			return 1024;
		default:
			return 2;
	}
}


void PulseInputAdvancedDialog::CreateControls(){

 	wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outerSizer);

	wxBoxSizer* okCancelSizer = new wxBoxSizer(wxHORIZONTAL);

	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		wxStaticBoxSizer *timerModeSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Pulse Mode"),wxVERTICAL);

		timerModeSizer->Add(new wxStaticText(this,-1,"Specify the type of measurement made for this Pulse Input Channel.\nYou can measure RPM, pulse frequency, or duration between pulses."));
		timerModeSizer->AddSpacer(10);

		wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(1,2,6,6);
		optionsSizer->AddGrowableCol(0,1);

		optionsSizer->Add(new wxStaticText(this,-1,"Mode"),1,wxALIGN_RIGHT);

		m_timerModeCombo = new wxComboBox(this,TIMER_MODE_COMBO,"",wxDefaultPosition,wxDefaultSize);
		m_timerModeCombo->Append("RPM");
		m_timerModeCombo->Append("Frequency");
		m_timerModeCombo->Append("Duration (mS)");
		m_timerModeCombo->Append("Duration (uS)");

		optionsSizer->Add(m_timerModeCombo,1,wxEXPAND);

		timerModeSizer->Add(optionsSizer,1,wxEXPAND);
		outerSizer->Add(timerModeSizer, 1, wxEXPAND | wxALL,1);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		wxStaticBoxSizer *timerDividerSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Timer  Options"),wxVERTICAL);

		timerDividerSizer->Add(new wxStaticText(this,-1,"Specify the Clock divider used by the timer to measure pulses. \nA larger divider slows down the clock so longer duration pulses can be measured."));
		timerDividerSizer->AddSpacer(10);

		wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(1,2,6,6);
		optionsSizer->AddGrowableCol(0,1);

		optionsSizer->Add(new wxStaticText(this,-1,"Clock Divider"),1,wxALIGN_RIGHT);

		m_timerDividerCombo = new wxComboBox(this,TIMER_DIVIDER_COMBO,"",wxDefaultPosition,wxDefaultSize);
		m_timerDividerCombo->Append("CLOCK / 2");
		m_timerDividerCombo->Append("CLOCK / 8");
		m_timerDividerCombo->Append("CLOCK / 32");
		m_timerDividerCombo->Append("CLOCK / 128");
		m_timerDividerCombo->Append("CLOCK / 1024");

		optionsSizer->Add(m_timerDividerCombo,1,wxEXPAND);

		timerDividerSizer->Add(optionsSizer,1,wxEXPAND);
		outerSizer->Add(timerDividerSizer, 1, wxEXPAND | wxALL,1);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		wxStaticBoxSizer *pulsePerRevSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Pulse Per Revolution"),wxVERTICAL);
		pulsePerRevSizer->Add(new wxStaticText(this,-1,"When measuring rotational speed, like RPM, specify the number of\npulses generated for a complete revolution of a shaft/wheel.\nOften this is the number of teeth on a wheel."));
		pulsePerRevSizer->AddSpacer(10);

		wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(1,2,6,6);
		optionsSizer->AddGrowableCol(0,1);

		optionsSizer->Add(new wxStaticText(this,-1,"Pulse Per Revolution"),1,wxALIGN_RIGHT);

		m_pulsePerRevSpin = new SteppedSpinCtrl(this,PULSE_PER_REV_SPIN);
		m_pulsePerRevSpin->SetStepValue(1);
		m_pulsePerRevSpin->SetRange(TimerConfig::MIN_PULSE_PER_REV, TimerConfig::MAX_PULSE_PER_REV);
		optionsSizer->Add(m_pulsePerRevSpin,1,wxEXPAND);

		pulsePerRevSizer->Add(optionsSizer,1,wxEXPAND);
		outerSizer->Add(pulsePerRevSizer, 1, wxEXPAND | wxALL,1);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////

 	wxButton* ok = new wxButton(this, wxID_OK, wxT("&OK"), wxDefaultPosition, wxDefaultSize,0);
	okCancelSizer->Add(ok, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

	wxButton* cancel = new wxButton(this, wxID_CANCEL, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize,0);
	okCancelSizer->Add(cancel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
	outerSizer->Add(okCancelSizer,0, wxALIGN_CENTER_HORIZONTAL | wxALL,3);
}

void PulseInputAdvancedDialog::SetConfig(TimerConfig *config){
	if (NULL != config) m_config = *config;
}

TimerConfig PulseInputAdvancedDialog::GetConfig(){
	return m_config;
}

void PulseInputAdvancedDialog::OnTimerModeSelected(wxCommandEvent &event){
	if (!m_dialogInitialized) return;
	wxComboBox *combo = dynamic_cast<wxComboBox*>(event.GetEventObject());
	if (NULL != combo) m_config.mode = (timer_mode_t)combo->GetSelection();
}

void PulseInputAdvancedDialog::OnTimerDividerSelected(wxCommandEvent &event){
	if (!m_dialogInitialized) return;
	wxComboBox *combo = dynamic_cast<wxComboBox*>(event.GetEventObject());
	if (NULL != combo) m_config.timerDivider = MapTimerDividerFromCombo(combo->GetSelection());
}

void PulseInputAdvancedDialog::OnPulsePerRevChanged(wxCommandEvent &event){
	if (!m_dialogInitialized) return;
	SteppedSpinCtrl *spinner = dynamic_cast<SteppedSpinCtrl*>(event.GetEventObject());
	if (NULL != spinner) m_config.pulsePerRev = spinner->GetValue();
}

BEGIN_EVENT_TABLE( PulseInputAdvancedDialog, wxDialog)
	EVT_COMBOBOX(TIMER_MODE_COMBO,PulseInputAdvancedDialog::OnTimerModeSelected)
	EVT_COMBOBOX(TIMER_DIVIDER_COMBO, PulseInputAdvancedDialog::OnTimerDividerSelected)
	EVT_TEXT(PULSE_PER_REV_SPIN, PulseInputAdvancedDialog::OnPulsePerRevChanged)
END_EVENT_TABLE()

