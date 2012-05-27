/*
 * ChannelConfigPanel.cpp
 *
 *  Created on: Apr 30, 2009
 *      Author: brent
 */
#include "configuration/channelConfigPanel.h"
#include "wx/sizer.h"

#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
WX_DEFINE_OBJARRAY(ChannelConfigExtraFields);

ChannelConfigPanel::ChannelConfigPanel() : wxPanel()
{
	ChannelConfigExtraFields emptyFields;
	InitComponents(false,emptyFields);
}

ChannelConfigPanel::ChannelConfigPanel(wxWindow *parent,
			ChannelConfigExtraFields extraFields,
			wxWindowID id,
			bool showHeaders,
			ChannelConfig *channelConfig,
			const wxPoint &pos,
			const wxSize &size,
			long style,
			const wxString &name
			)
			: wxPanel(	parent,
						id,
						pos,
						size,
						style,
						name)
{
	m_channelConfig = channelConfig;
	InitComponents(showHeaders,extraFields);
	//OnConfigUpdated();
}

ChannelConfigPanel::~ChannelConfigPanel(){

}

void ChannelConfigPanel::InitComponents(bool showHeaders, ChannelConfigExtraFields &extraFields){

//	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,title),wxHORIZONTAL);
	wxFlexGridSizer *sizer = new wxFlexGridSizer(1 + (int)showHeaders,3 + extraFields.Count(),3,3);
	this->SetSizer(sizer);

	if (showHeaders){
		sizer->Add(new wxStaticText(this,-1,"Label"),1,wxEXPAND);
		sizer->Add(new wxStaticText(this,-1,"Units"),1,wxEXPAND);
		sizer->Add(new wxStaticText(this,-1,"Sample Rate"),1,wxEXPAND);
		for (size_t i = 0;i < extraFields.Count();i++){
			ChannelConfigExtraField f = extraFields[i];
			sizer->Add(new wxStaticText(this,-1,f.header),1,wxEXPAND);
		}
	}
	m_channelLabel = new wxTextCtrl(this,wxID_ANY);
	m_channelLabel->SetMaxLength(ChannelConfig::MAX_LABEL_LENGTH);
	m_channelLabel->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ChannelConfigPanel::OnLabelChanged),NULL,this);
	sizer->Add(m_channelLabel,1,wxEXPAND);

	m_channelUnits = new wxTextCtrl(this,wxID_ANY);
	m_channelUnits->SetMaxLength(ChannelConfig::MAX_UNITS_LENGTH);
	m_channelUnits->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ChannelConfigPanel::OnUnitsChanged),NULL,this);
	sizer->Add(m_channelUnits,1,wxEXPAND);

	m_channelSampleRate = new wxComboBox(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,NULL,wxCB_DROPDOWN | wxCB_READONLY);
	InitSampleRateCombo(m_channelSampleRate);
	m_channelSampleRate->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(ChannelConfigPanel::OnSampleRateChanged),NULL,this);
	sizer->Add(m_channelSampleRate,1,wxEXPAND);

	for (size_t i = 0;i < extraFields.Count();i++){
		ChannelConfigExtraField f = extraFields[i];
		f.control->Reparent(this);
		sizer->Add(f.control,1,wxEXPAND);
	}
}

void ChannelConfigPanel::OnLabelChanged(wxCommandEvent &event){
	wxTextCtrl *c = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != c) m_channelConfig->label = c->GetValue();
}

void ChannelConfigPanel::OnUnitsChanged(wxCommandEvent &event){
	wxTextCtrl *c = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != c) m_channelConfig->units = c->GetValue();
}

void ChannelConfigPanel::OnSampleRateChanged(wxCommandEvent &event){
	wxComboBox *c = dynamic_cast<wxComboBox*>(event.GetEventObject());
	if (NULL != c) m_channelConfig->sampleRate = MapSampleRateFromCombo(c->GetSelection());
}

void ChannelConfigPanel::SetChannelConfig(ChannelConfig *config){
	m_channelConfig = config;
	OnConfigUpdated();
}

void ChannelConfigPanel::InitSampleRateCombo(wxComboBox *combo){
	SampleRates samplerates = ChannelConfig::GetSampleRates();
	for (SampleRates::iterator it = samplerates.begin(); it != samplerates.end(); ++it){
		int sr = *it;
		combo->Append((sr == 0 ? wxString("Disabled") : wxString::Format("%d Hz",*it)));
	}
}

sample_rate_t ChannelConfigPanel::MapSampleRateFromCombo(int index){
	SampleRates sampleRates = ChannelConfig::GetSampleRates();
	return sampleRates[index];
}

int ChannelConfigPanel::MapSampleRateToCombo(int sampleRate){
	SampleRates samplerates = ChannelConfig::GetSampleRates();
	int i=0;
	for (SampleRates::iterator it = samplerates.begin(); it != samplerates.end(); ++it){
		if (*it == sampleRate) return i;
		i++;
	}
	return 0;
}

void ChannelConfigPanel::OnConfigUpdated(){
	if (NULL == m_channelConfig) return;
	m_channelLabel->SetValue(m_channelConfig->label);
	m_channelUnits->SetValue(m_channelConfig->units);
	m_channelSampleRate->SetSelection(MapSampleRateToCombo(m_channelConfig->sampleRate));
}

wxPanel * GetTopInnerPanel(){
	return NULL;
}

wxPanel * getBottomInnerPanel(){
	return NULL;
}

BEGIN_EVENT_TABLE ( ChannelConfigPanel, wxPanel )

END_EVENT_TABLE()
