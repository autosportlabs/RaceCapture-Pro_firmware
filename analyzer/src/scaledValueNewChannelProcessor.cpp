/*
 * scaledValueNewChannelProcessor.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: brent
 */
#include "scaledValueNewChannelProcessor.h"

void ScaledValueNewChannelProcessor::Process(){

}

ScaledValueOptionsPage::ScaledValueOptionsPage(wxWizard *parent, AddChannelWizardParams *params) :
	wxWizardPageSimple(parent),m_params(params){
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(new wxStaticText(this, -1, "Scaled value"),1,wxALL | wxEXPAND,1 );
    SetSizer(mainSizer);
    mainSizer->Fit(this);

}

void ScaledValueOptionsPage::OnWizardPageChanged(wxWizardEvent &event){

}

void ScaledValueOptionsPage::OnWizardPageChanging(wxWizardEvent &event){

}

void ScaledValueOptionsPage::OnWizardPageFinished(wxWizardEvent &event){

}


BEGIN_EVENT_TABLE ( ScaledValueOptionsPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, ScaledValueOptionsPage::OnWizardPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, ScaledValueOptionsPage::OnWizardPageChanging)
	EVT_WIZARD_FINISHED(wxID_ANY, ScaledValueOptionsPage::OnWizardPageFinished)

END_EVENT_TABLE()

