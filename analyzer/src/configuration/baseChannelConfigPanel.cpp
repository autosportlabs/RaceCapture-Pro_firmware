/*
 * BaseChannelConfigPanel.cpp
 *
 *  Created on: Apr 30, 2009
 *      Author: brent
 */
#include "configuration/baseChannelConfigPanel.h"


BaseChannelConfigPanel::BaseChannelConfigPanel() : BaseConfigPanel()
{
}

BaseChannelConfigPanel::BaseChannelConfigPanel(wxWindow *parent,
			wxWindowID id,
			RaceCaptureConfig *config,
			const wxPoint &pos,
			const wxSize &size,
			long style,
			const wxString &name
			)
			: BaseConfigPanel(	parent,
						id,
						config,
						pos,
						size,
						style,
						name)
{
}

BaseChannelConfigPanel::~BaseChannelConfigPanel(){

}

ChannelConfigPanel * BaseChannelConfigPanel::FindChannelConfigPanel(int index){
	ChannelConfigPanel *p = dynamic_cast<ChannelConfigPanel*>(FindWindowByName(GetChannelConfigPanelName(index)));
	return p;
}

wxTextCtrl * BaseChannelConfigPanel::FindTextCtrl(wxString name){
	wxTextCtrl *p = dynamic_cast<wxTextCtrl*>(FindWindowByName(name));
	return p;
}

wxComboBox * BaseChannelConfigPanel::FindComboBoxCtrl(wxString name){
	wxComboBox *p = dynamic_cast<wxComboBox*>(FindWindowByName(name));
	return p;
}

wxCheckBox * BaseChannelConfigPanel::FindCheckBoxCtrl(wxString name){
	wxCheckBox *p = dynamic_cast<wxCheckBox*>(FindWindowByName(name));
	return p;
}

void BaseChannelConfigPanel::SetTextCtrlValue(const wxString &name, float value){
	wxTextCtrl *c = FindTextCtrl(name);
	if (NULL != c) c->SetValue(wxString::Format("%f",value));
}

void BaseChannelConfigPanel::SetCheckBoxValue(const wxString &name, bool value){
	wxCheckBox *c = FindCheckBoxCtrl(name);
	if (NULL != c) c->SetValue(value);
}

void BaseChannelConfigPanel::OnConfigUpdated(){
	int channelCount = ChannelCount();
	for (int i=0; i < channelCount;i++){
		ChannelConfigPanel *p = FindChannelConfigPanel(i);
		if (NULL != p) p->OnConfigUpdated();
		UpdateExtendedChannelFields(i);
	}
	UpdatedExtendedFields();
}

ChannelConfigPanel * BaseChannelConfigPanel::CreateNewChannelConfigPanel(int index,bool showHeaders, ChannelConfigExtraFields &extraFields, ChannelConfig *channelConfig){
	ChannelConfigPanel *p = new ChannelConfigPanel(this,extraFields,-1,showHeaders,channelConfig);
	p->SetName(GetChannelConfigPanelName(index));
	return p;
}

void BaseChannelConfigPanel::InitComponents(){

	wxPanel *topPanel = GetTopInnerPanel();
	wxPanel *bottomPanel = GetBottomInnerPanel();

	wxFlexGridSizer *sizer = new wxFlexGridSizer(1 + (NULL != topPanel) + (NULL != bottomPanel),1,3,3);
	sizer->AddGrowableCol(0);

	if (NULL != topPanel){
		sizer->Add(topPanel,1,wxEXPAND);
	}

	wxStaticBoxSizer *borderSizer= new wxStaticBoxSizer(new wxStaticBox(this,-1,GetChannelListTitle()),wxVERTICAL);
	wxFlexGridSizer *channelSizer = new wxFlexGridSizer(ChannelCount(),2,8,8);
	channelSizer->AddGrowableCol(1);

	int channelCount = ChannelCount();
	for (int i=0; i < channelCount;i++){
		ChannelConfig *channelConfig = GetChannelConfig(i);
		ChannelConfigExtraFields extraFields = CreateExtendedChannelFields(i);
		ChannelConfigPanel *ccPanel = CreateNewChannelConfigPanel(i,i == 0,extraFields,channelConfig);
		channelSizer->Add(new wxStaticText(this,-1,GetChannelLabel(i)),1,wxALIGN_BOTTOM);
		channelSizer->Add(ccPanel,1,wxALIGN_LEFT);
	}
	borderSizer->Add(channelSizer,1,wxEXPAND);
	sizer->Add(borderSizer,1,wxEXPAND);

	if (NULL != bottomPanel){
		sizer->Add(bottomPanel,1);
	}


	this->SetSizer(sizer);
}

BEGIN_EVENT_TABLE ( BaseChannelConfigPanel, wxPanel )
END_EVENT_TABLE()
