#include "configuration/analogPulseOutputAdvancedOptions.h"
#include "raceCapture/raceCaptureConfig.h"

IMPLEMENT_CLASS (AnalogPulseOutputAdvancedDialog, wxDialog)

AnalogPulseOutputAdvancedDialog::AnalogPulseOutputAdvancedDialog() : m_dialogInitialized(false)  {}

AnalogPulseOutputAdvancedDialog::AnalogPulseOutputAdvancedDialog(wxWindow* parent,
  			wxWindowID id,
  			const wxString& caption,
  			const wxPoint& pos,
  			const wxSize& size,
  			long style) : m_dialogInitialized(false){
	Create(parent,id,caption,pos,size,style);
}

bool AnalogPulseOutputAdvancedDialog::Create (wxWindow* parent,
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

void AnalogPulseOutputAdvancedDialog::RefreshControls(){

	m_startupDutyCycle->SetValue(m_config.startupDutyCycle);
	m_startupPeriod->SetValue(m_config.startupPeriod);
	m_voltageScaling->SetValue(wxString::Format("%f",m_config.voltageScaling));
}

void AnalogPulseOutputAdvancedDialog::CreateControls(){

 	wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outerSizer);

	wxBoxSizer* okCancelSizer = new wxBoxSizer(wxHORIZONTAL);

	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		wxStaticBoxSizer *startupPwmSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Startup PWM Duty Cycle"),wxVERTICAL);

		startupPwmSizer->Add(new wxStaticText(this,-1,"Specify the Pulse Width Modulated (PWM) duty cycle of when RaceCapture first starts up"));
		startupPwmSizer->AddSpacer(10);

		wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(1,2,6,6);
		optionsSizer->AddGrowableCol(0,1);

		optionsSizer->Add(new wxStaticText(this,-1,"Duty Cycle"),1,wxALIGN_RIGHT);

		m_startupDutyCycle = new SteppedSpinCtrl(this,STARTUP_DUTY_CYCLE_SPIN);
		m_startupDutyCycle->SetStepValue(1);
		m_startupDutyCycle->SetRange(PwmConfig::MIN_PWM_DUTY_CYCLE, PwmConfig::MAX_PWM_DUTY_CYCLE);
		optionsSizer->Add(m_startupDutyCycle,1,wxEXPAND);

		startupPwmSizer->Add(optionsSizer,1,wxEXPAND);
		outerSizer->Add(startupPwmSizer, 1, wxEXPAND | wxALL,1);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		wxStaticBoxSizer *startupPeriodSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Startup PWM Period"),wxVERTICAL);

		startupPeriodSizer->Add(new wxStaticText(this,-1,"Specify the Pulse Width Modulated Period when RaceCapture first starts up"));
		startupPeriodSizer->AddSpacer(10);

		wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(1,2,6,6);
		optionsSizer->AddGrowableCol(0,1);

		optionsSizer->Add(new wxStaticText(this,-1,"Period"),1,wxALIGN_RIGHT);

		m_startupPeriod = new SteppedSpinCtrl(this,STARTUP_PERIOD_SPIN);
		m_startupPeriod->SetStepValue(1);
		m_startupPeriod->SetRange(PwmConfig::MIN_PWM_PERIOD, PwmConfig::MAX_PWM_PERIOD);
		optionsSizer->Add(m_startupPeriod,1,wxEXPAND);

		startupPeriodSizer->Add(optionsSizer,1,wxEXPAND);
		outerSizer->Add(startupPeriodSizer, 1, wxEXPAND | wxALL,1);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		wxStaticBoxSizer *voltageScalingSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Voltage Scaling"),wxVERTICAL);
		voltageScalingSizer->Add(new wxStaticText(this,-1,"Specify the Voltage Scaling used when logging the value of this PWM/Analog Output Channel"));
		voltageScalingSizer->AddSpacer(10);

		wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(1,2,6,6);
		optionsSizer->AddGrowableCol(0,1);


		optionsSizer->Add(new wxStaticText(this,-1,"Voltage Scaling"),1,wxALIGN_RIGHT);

		m_voltageScaling = new wxTextCtrl(this,VOLTAGE_SCALING_TEXT_CTRL,"",wxDefaultPosition,wxDefaultSize,0,wxTextValidator(wxFILTER_NUMERIC));

		optionsSizer->Add(m_voltageScaling,1,wxEXPAND);

		voltageScalingSizer->Add(optionsSizer,1,wxEXPAND);
		outerSizer->Add(voltageScalingSizer, 1, wxEXPAND | wxALL,1);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////

 	wxButton* ok = new wxButton(this, wxID_OK, wxT("&OK"), wxDefaultPosition, wxDefaultSize,0);
	okCancelSizer->Add(ok, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

	wxButton* cancel = new wxButton(this, wxID_CANCEL, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize,0);
	okCancelSizer->Add(cancel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
	outerSizer->Add(okCancelSizer,0, wxALIGN_CENTER_HORIZONTAL | wxALL,3);
}

void AnalogPulseOutputAdvancedDialog::SetConfig(PwmConfig *config){
	if (NULL != config) m_config = *config;
}

PwmConfig AnalogPulseOutputAdvancedDialog::GetConfig(){
	return m_config;
}

void AnalogPulseOutputAdvancedDialog::OnStartupDutyCycleChanged(wxCommandEvent &event){
	if (!m_dialogInitialized) return;
	SteppedSpinCtrl *spinner = dynamic_cast<SteppedSpinCtrl*>(event.GetEventObject());
	if (NULL != spinner) m_config.startupDutyCycle = spinner->GetValue();
}

void AnalogPulseOutputAdvancedDialog::OnStartupPeriodChanged(wxCommandEvent &event){
	if (!m_dialogInitialized) return;
	SteppedSpinCtrl *spinner = dynamic_cast<SteppedSpinCtrl*>(event.GetEventObject());
	if (NULL != spinner) m_config.startupPeriod = spinner->GetValue();
}

void AnalogPulseOutputAdvancedDialog::OnVoltageScalingChanged(wxCommandEvent &event){
	if (!m_dialogInitialized) return;
	wxTextCtrl *textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != textCtrl) m_config.voltageScaling = atof(textCtrl->GetValue());
}

BEGIN_EVENT_TABLE( AnalogPulseOutputAdvancedDialog, wxDialog)
	EVT_TEXT(STARTUP_DUTY_CYCLE_SPIN, AnalogPulseOutputAdvancedDialog::OnStartupDutyCycleChanged)
	EVT_TEXT(STARTUP_PERIOD_SPIN, AnalogPulseOutputAdvancedDialog::OnStartupPeriodChanged)
	EVT_TEXT(VOLTAGE_SCALING_TEXT_CTRL, AnalogPulseOutputAdvancedDialog::OnVoltageScalingChanged)
END_EVENT_TABLE()

