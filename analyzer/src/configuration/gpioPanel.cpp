#include "configuration/configPanel.h"
#include "configuration/gpioPanel.h"

GpioPanel::GpioPanel() : BaseChannelConfigPanel()
{
}

GpioPanel::GpioPanel(wxWindow *parent,
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

GpioPanel::~GpioPanel(){

}

void GpioPanel::UpdateExtendedChannelFields(int i){
	wxComboBox *ctrl = FindComboBoxCtrl(GetModeFieldName(i));
	if (NULL != ctrl){
		ctrl->SetSelection(m_raceCaptureConfig->gpioConfigs[i].mode);
	}
}

void GpioPanel::InitModeCombo(wxComboBox * modeCombo){
	modeCombo->Append("Input");
	modeCombo->Append("Output");
	modeCombo->SetSelection(0);
}

ChannelConfig * GpioPanel::GetChannelConfig(int i){
	return &(m_raceCaptureConfig->gpioConfigs[i].channelConfig);
}

int GpioPanel::ChannelCount(){
	return CONFIG_GPIO_CHANNELS;
}

ChannelConfigExtraFields GpioPanel::CreateExtendedChannelFields(int i){
	ChannelConfigExtraFields extraFields;
	{
		ChannelConfigExtraField f;
		wxComboBox *modeCombo = new wxComboBox(this,-1,"",wxDefaultPosition,wxDefaultSize,0,NULL,wxCB_DROPDOWN | wxCB_READONLY);
		modeCombo->SetName(GetModeFieldName(i));
		InitModeCombo(modeCombo);
		f.control = modeCombo;
		f.header = "Mode";
		extraFields.Add(f);
	}
	return extraFields;
}

wxString GpioPanel::GetChannelLabel(int index){
	return wxString::Format("GPIO %d",index + 1);
}

wxString GpioPanel::GetChannelListTitle(){
	return "General Purpose Input/Output Channels";
}

wxString GpioPanel::GetChannelConfigPanelName(int index){
	return wxString::Format("gpio_channel_%d",index);
}

wxString GpioPanel::GetModeFieldName(int index){
	return wxString::Format("gpio_channel_mode_%d",index);
}

wxPanel * GpioPanel::GetTopInnerPanel(){
	return NULL;
}

wxPanel * GpioPanel::GetBottomInnerPanel(){
	return NULL;
}

void GpioPanel::UpdatedExtendedFields()
{
}

BEGIN_EVENT_TABLE ( GpioPanel, wxPanel )
END_EVENT_TABLE()
