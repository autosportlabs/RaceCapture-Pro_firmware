/*
 * AnalogInputPanel.cpp
 *
 *  Created on: Apr 30, 2009
 *      Author: brent
 */
#include "configuration/configPanel.h"
#include "configuration/analogInputPanel.h"
#include "configuration/analogInputAdvancedOptions.h"


AnalogInputPanel::AnalogInputPanel() : BaseChannelConfigPanel()
{
}

AnalogInputPanel::AnalogInputPanel(wxWindow *parent,
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

AnalogInputPanel::~AnalogInputPanel(){

}

void AnalogInputPanel::UpdateExtendedChannelFields(int i){
	AnalogConfig &cfg = (m_raceCaptureConfig->analogConfigs[i]);
	m_loggingPrecisionSpinner[i]->SetValue(cfg.loggingPrecision);
}

ChannelConfig * AnalogInputPanel::GetChannelConfig(int i){
	return &(m_raceCaptureConfig->analogConfigs[i].channelConfig);
}

int AnalogInputPanel::ChannelCount(){
	return CONFIG_ANALOG_CHANNELS;
}

ChannelConfigExtraFields AnalogInputPanel::CreateExtendedChannelFields(int i){
	AnalogConfig &cfg = (m_raceCaptureConfig->analogConfigs[i]);
	ChannelConfigExtraFields extraFields;
	{
		ChannelConfigExtraField f;
		SteppedSpinCtrl *spin = new SteppedSpinCtrl(this,wxID_ANY);
		spin->SetStepValue(1);
		spin->SetRange(ChannelConfig::MIN_PRECISION,ChannelConfig::MAX_PRECISION);
		spin->SetClientData(&cfg);
		spin->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(AnalogInputPanel::OnPrecisionChanged),NULL,this);
		m_loggingPrecisionSpinner[i] = spin;
		f.control = spin;
		f.header = "Logging Precision";
		extraFields.Add(f);
	}
	{
		ChannelConfigExtraField f;
		wxButton * but = new wxButton(this,wxID_ANY,"...");
		but->SetClientData(&cfg);
		but->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnalogInputPanel::OnAdvancedSettings),NULL,this);
		f.control = but;
		f.header = "Advanced";
		extraFields.Add(f);
	}
	return extraFields;
}

wxString AnalogInputPanel::GetChannelConfigPanelName(int index){
	return wxString::Format("analog_channel_%d",index);
}

wxString AnalogInputPanel::GetChannelLabel(int index){
	return wxString::Format("Channel %d",index + 1);
}

wxString AnalogInputPanel::GetChannelListTitle(){
	return "Analog Input Channels";
}

wxPanel * AnalogInputPanel::GetTopInnerPanel(){
	return NULL;
}

wxPanel * AnalogInputPanel::GetBottomInnerPanel(){
	return NULL;
}

void AnalogInputPanel::UpdatedExtendedFields()
{

}

void AnalogInputPanel::OnPrecisionChanged(wxCommandEvent &event){
	SteppedSpinCtrl *s = dynamic_cast<SteppedSpinCtrl*>(event.GetEventObject());
	if (NULL != s) {
		AnalogConfig *cfg = (AnalogConfig*)s->GetClientData();
		cfg->loggingPrecision = s->GetValue();
	}
}

void AnalogInputPanel::OnAdvancedSettings(wxCommandEvent &event){

	wxButton *b = (wxButton*)event.GetEventObject();
	AnalogConfig *c = (AnalogConfig*)(b->GetClientData());

	AnalogInputAdvancedDialog dlg;
	dlg.SetConfig(c);
	dlg.Create(this);
	if (dlg.ShowModal() == wxID_OK){
		*c = dlg.GetConfig();
	}
}


BEGIN_EVENT_TABLE ( AnalogInputPanel, wxPanel )
END_EVENT_TABLE()
