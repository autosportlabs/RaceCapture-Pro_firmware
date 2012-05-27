#include "configuration/configPanel.h"
#include "configuration/pulseInputPanel.h"
#include "configuration/pulseInputAdvancedOptions.h"

PulseInputPanel::PulseInputPanel() : BaseChannelConfigPanel()
{
}

PulseInputPanel::PulseInputPanel(wxWindow *parent,
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

PulseInputPanel::~PulseInputPanel(){

}


void PulseInputPanel::UpdateExtendedChannelFields(int i){
	wxTextCtrl *ctrl = FindTextCtrl(GetScalingFieldName(i));
	if (NULL != ctrl){
		ctrl->SetValue(wxString::Format("%d",m_raceCaptureConfig->timerConfigs[i].scaling));
	}
}

ChannelConfig * PulseInputPanel::GetChannelConfig(int i){
	return &(m_raceCaptureConfig->timerConfigs[i].channelConfig);
}

int PulseInputPanel::ChannelCount(){
	return CONFIG_TIMER_CHANNELS;
}

ChannelConfigExtraFields PulseInputPanel::CreateExtendedChannelFields(int i){
	ChannelConfigExtraFields extraFields;
	{
		ChannelConfigExtraField f;
		SteppedSpinCtrl *ctrl = new SteppedSpinCtrl(this,wxID_ANY);
		ctrl->SetStepValue(1);
		ctrl->SetRange(ChannelConfig::MIN_PRECISION,ChannelConfig::MAX_PRECISION);
		ctrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(PulseInputPanel::OnPrecisionChanged),NULL,this);
		f.control = ctrl;
		f.header = "Logging Precision";
		extraFields.Add(f);
	}
	{
		ChannelConfigExtraField f;
		wxButton * but = new wxButton(this,wxID_ANY,"...");
		but->SetClientData(&(m_raceCaptureConfig->timerConfigs[i]));
		but->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PulseInputPanel::OnAdvancedSettings),NULL,this);
		f.control = but;
		f.header = "Advanced";
		extraFields.Add(f);
	}
	return extraFields;
}

wxString PulseInputPanel::GetChannelLabel(int index){
	return wxString::Format("Channel %d",index + 1);
}

wxString PulseInputPanel::GetChannelListTitle(){
	return "Pulse/Timer Input Channels";
}

const wxString PulseInputPanel::GetScalingFieldName(int index){
	return wxString::Format("pulse_channel_scaling_%d",index);
}

wxString PulseInputPanel::GetChannelConfigPanelName(int index){
	return wxString::Format("pulse_channel_%d",index);
}

wxPanel * PulseInputPanel::GetTopInnerPanel(){
	return NULL;
}

wxPanel * PulseInputPanel::GetBottomInnerPanel(){
	return NULL;
}

void PulseInputPanel::UpdatedExtendedFields()
{
}

void PulseInputPanel::OnPrecisionChanged(wxCommandEvent &event){
	SteppedSpinCtrl *s = dynamic_cast<SteppedSpinCtrl*>(event.GetEventObject());
	if (NULL != s) {
		AnalogConfig *cfg = (AnalogConfig*)s->GetClientData();
		cfg->loggingPrecision = s->GetValue();
	}
}

void PulseInputPanel::OnAdvancedSettings(wxCommandEvent &event){

	wxButton *b = (wxButton*)event.GetEventObject();
	TimerConfig *c = (TimerConfig*)(b->GetClientData());

	PulseInputAdvancedDialog dlg;
	dlg.SetConfig(c);
	dlg.Create(this);
	if (dlg.ShowModal() == wxID_OK){
		*c = dlg.GetConfig();
	}
}


BEGIN_EVENT_TABLE ( PulseInputPanel, wxPanel )
END_EVENT_TABLE()
