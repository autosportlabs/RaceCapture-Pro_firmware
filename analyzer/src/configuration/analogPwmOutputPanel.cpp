#include "configuration/configPanel.h"
#include "configuration/analogPwmOutputPanel.h"
#include "configuration/analogPulseOutputAdvancedOptions.h"

AnalogPulseOutputPanel::AnalogPulseOutputPanel() : BaseChannelConfigPanel()
{
}

AnalogPulseOutputPanel::AnalogPulseOutputPanel(wxWindow *parent,
			wxWindowID id,
			RaceCaptureConfig *config,
			const wxPoint &pos,
			const wxSize &size,
			long style,
			const wxString &name
			)
			: BaseChannelConfigPanel(	parent,
						id,
						config,
						pos,
						size,
						style,
						name)
{

}

AnalogPulseOutputPanel::~AnalogPulseOutputPanel(){

}


void AnalogPulseOutputPanel::UpdateExtendedChannelFields(int i){
	PwmConfig &cfg = (m_raceCaptureConfig->pwmConfigs[i]);
	m_loggingPrecisionSpinner[i]->SetValue(cfg.loggingPrecision);
	m_modeCombo[i]->SetSelection(cfg.outputMode);
}

ChannelConfig * AnalogPulseOutputPanel::GetChannelConfig(int i){
	return &(m_raceCaptureConfig->pwmConfigs[i].channelConfig);
}

int AnalogPulseOutputPanel::ChannelCount(){
	return CONFIG_ANALOG_PULSE_CHANNELS;
}

ChannelConfigExtraFields AnalogPulseOutputPanel::CreateExtendedChannelFields(int i){
	PwmConfig &cfg = (m_raceCaptureConfig->pwmConfigs[i]);
	ChannelConfigExtraFields extraFields;
	{
		ChannelConfigExtraField f;
		SteppedSpinCtrl *spin = new SteppedSpinCtrl(this,wxID_ANY);
		spin->SetStepValue(1);
		spin->SetRange(ChannelConfig::MIN_PRECISION,ChannelConfig::MAX_PRECISION);
		spin->SetClientData(&cfg);
		spin->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(AnalogPulseOutputPanel::OnPrecisionChanged),NULL,this);
		m_loggingPrecisionSpinner[i] = spin;
		f.control = spin;
		f.header = "Logging Precision";
		extraFields.Add(f);
	}
	{
		ChannelConfigExtraField f;
		wxComboBox *modeCombo = new wxComboBox(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,NULL,wxCB_DROPDOWN | wxCB_READONLY);
		modeCombo->SetClientData(&cfg);
		modeCombo->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(AnalogPulseOutputPanel::OnModeChanged),NULL,this);
		InitModeCombo(modeCombo);
		m_modeCombo[i] = modeCombo;
		f.control = modeCombo;
		f.header = "Mode";
		extraFields.Add(f);
	}
	{
		ChannelConfigExtraField f;
		wxButton * but = new wxButton(this,wxID_ANY,"...");
		but->SetClientData(&cfg);
		but->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnalogPulseOutputPanel::OnAdvancedSettings),NULL,this);
		f.control = but;
		f.header = "Advanced";
		extraFields.Add(f);
	}
	return extraFields;
}

void AnalogPulseOutputPanel::InitModeCombo(wxComboBox *modeCombo){
	modeCombo->Append("Analog");
	modeCombo->Append("Pulse");
	modeCombo->Select(0);
}

wxString AnalogPulseOutputPanel::GetChannelLabel(int index){
	return wxString::Format("Output %d",index + 1);
}

wxString AnalogPulseOutputPanel::GetChannelListTitle(){
	return "Analog / Pulse Output Channels";
}

wxString AnalogPulseOutputPanel::GetChannelConfigPanelName(int index){
	return wxString::Format("pwmout_channel_%d",index);
}

wxPanel * AnalogPulseOutputPanel::GetTopInnerPanel(){
	return NULL;
}

wxPanel * AnalogPulseOutputPanel::GetBottomInnerPanel(){
	return NULL;
}

void AnalogPulseOutputPanel::UpdatedExtendedFields()
{
}

void AnalogPulseOutputPanel::OnPrecisionChanged(wxCommandEvent &event){
	SteppedSpinCtrl *s = dynamic_cast<SteppedSpinCtrl*>(event.GetEventObject());
	if (NULL != s) {
		PwmConfig *cfg = (PwmConfig*)s->GetClientData();
		cfg->loggingPrecision = s->GetValue();
	}
}

void AnalogPulseOutputPanel::OnModeChanged(wxCommandEvent &event){
	wxComboBox *c = dynamic_cast<wxComboBox*>(event.GetEventObject());
	if (NULL != c) {
		PwmConfig *cfg = (PwmConfig*)c->GetClientData();
		cfg->outputMode = (pwm_output_mode_t)c->GetSelection();
	}
}

void AnalogPulseOutputPanel::OnAdvancedSettings(wxCommandEvent &event){

	wxButton *b = (wxButton*)event.GetEventObject();
	PwmConfig *c = (PwmConfig*)(b->GetClientData());

	AnalogPulseOutputAdvancedDialog dlg;
	dlg.SetConfig(c);
	dlg.Create(this);
	if (dlg.ShowModal() == wxID_OK){
		*c = dlg.GetConfig();
	}
}

BEGIN_EVENT_TABLE ( AnalogPulseOutputPanel, wxPanel )
END_EVENT_TABLE()
