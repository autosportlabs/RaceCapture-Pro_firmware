#include "configuration/configPanel.h"
#include "configuration/accelInputPanel.h"

AccelInputPanel::AccelInputPanel() : BaseChannelConfigPanel()
{
}

AccelInputPanel::AccelInputPanel(wxWindow *parent,
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

AccelInputPanel::~AccelInputPanel(){

}


void AccelInputPanel::UpdateExtendedChannelFields(int i){
	AccelConfig &cfg = (m_raceCaptureConfig->accelConfigs[i]);
	m_accelMode[i]->Select(cfg.mode);
	m_accelMapping[i]->Select(cfg.channel);
	m_accelZeroValue[i]->SetValue(cfg.zeroValue);
}

ChannelConfig * AccelInputPanel::GetChannelConfig(int i){
	return &(m_raceCaptureConfig->accelConfigs[i].channelConfig);
}

int AccelInputPanel::ChannelCount(){
	return CONFIG_ACCEL_CHANNELS;
}

ChannelConfigExtraFields AccelInputPanel::CreateExtendedChannelFields(int i){
	AccelConfig &cfg = (m_raceCaptureConfig->accelConfigs[i]);
	ChannelConfigExtraFields extraFields;
	{
		ChannelConfigExtraField f;
		wxComboBox *c = new wxComboBox(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,NULL,wxCB_DROPDOWN | wxCB_READONLY);
		c->SetClientData(&cfg);
		c->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(AccelInputPanel::OnChannelModeChanged),NULL,this);
		InitModeCombo(c);
		m_accelMode[i] = c;
		f.control = c;
		f.header = "Mode";
		extraFields.Add(f);
	}
	{
		ChannelConfigExtraField f;
		wxComboBox *c = new wxComboBox(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,NULL,wxCB_DROPDOWN | wxCB_READONLY);
		c->SetClientData(&cfg);
		c->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(AccelInputPanel::OnChannelMappingChanged),NULL,this);
		InitMappingCombo(c);
		m_accelMapping[i] = c;
		f.control = c;
		f.header = "Mapping";
		extraFields.Add(f);
	}
	{
		ChannelConfigExtraField f;
		SteppedSpinCtrl *spin = new SteppedSpinCtrl(this,wxID_ANY);
		spin->SetStepValue(1);
		spin->SetRange(MIN_ANALOG_RAW,MAX_ANALOG_RAW);
		spin->SetClientData(&cfg);
		spin->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(AccelInputPanel::OnAccelZeroChanged),NULL,this);
		m_accelZeroValue[i] = spin;
		f.control = spin;
		f.header = "Zero Value";
		extraFields.Add(f);
	}

	return extraFields;
}

void AccelInputPanel::InitMappingCombo(wxComboBox *combo){
	combo->Append("X");
	combo->Append("Y");
	combo->Append("Z");
	combo->Append("Yaw");
	combo->Select(0);
}

void AccelInputPanel::InitModeCombo(wxComboBox *combo){
	combo->Append("Disabled");
	combo->Append("Normal");
	combo->Append("Inverted");
	combo->Select(0);
}

wxString AccelInputPanel::GetChannelLabel(int index){
	return wxString::Format("Axis %d",index + 1);
}

wxString AccelInputPanel::GetChannelListTitle(){
	return "Accelerometer Channels";
}

wxString AccelInputPanel::GetChannelConfigPanelName(int index){
	return wxString::Format("accel_channel_%d",index);
}

wxPanel * AccelInputPanel::GetTopInnerPanel(){
	return NULL;
}

wxPanel * AccelInputPanel::GetBottomInnerPanel(){
	return NULL;
}

void AccelInputPanel::UpdatedExtendedFields()
{
}

void AccelInputPanel::OnChannelModeChanged(wxCommandEvent &event){
	wxComboBox *c = dynamic_cast<wxComboBox*>(event.GetEventObject());
	if (NULL != c) {
		AccelConfig *cfg = (AccelConfig*)c->GetClientData();
		cfg->mode = (accel_mode_t)c->GetSelection();
	}
}

void AccelInputPanel::OnChannelMappingChanged(wxCommandEvent &event){
	wxComboBox *c = dynamic_cast<wxComboBox*>(event.GetEventObject());
	if (NULL != c) {
		AccelConfig *cfg = (AccelConfig*)c->GetClientData();
		cfg->channel = (accel_channel_t)c->GetSelection();
	}
}

void AccelInputPanel::OnAccelZeroChanged(wxCommandEvent &event){
	SteppedSpinCtrl *s = dynamic_cast<SteppedSpinCtrl*>(event.GetEventObject());
	if (NULL != s) {
		AccelConfig *cfg = (AccelConfig*)s->GetClientData();
		cfg->zeroValue = s->GetValue();
	}
}

BEGIN_EVENT_TABLE ( AccelInputPanel, wxPanel )
END_EVENT_TABLE()
