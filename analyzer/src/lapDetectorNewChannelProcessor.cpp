/*
 * lapDetectorNewChannelProcessor.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: brent
 */
#include "lapDetectorNewChannelProcessor.h"

void LapDetectorNewChannelProcessor::Process(){

}

LapDetectorOptionsPage::LapDetectorOptionsPage(wxWizard *parent, AddChannelWizardParams *params) :
	wxWizardPageSimple(parent),m_params(params){
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(new wxStaticText(this, -1, "Lap Detector"),1,wxALL | wxEXPAND,1 );
    SetSizer(mainSizer);
    mainSizer->Fit(this);


}

void LapDetectorOptionsPage::OnWizardPageChanged(wxWizardEvent &event){

}

void LapDetectorOptionsPage::OnWizardPageChanging(wxWizardEvent &event){

}

void LapDetectorOptionsPage::OnWizardPageFinished(wxWizardEvent &event){

}

BEGIN_EVENT_TABLE ( LapDetectorOptionsPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, LapDetectorOptionsPage::OnWizardPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, LapDetectorOptionsPage::OnWizardPageChanging)
	EVT_WIZARD_FINISHED(wxID_ANY, LapDetectorOptionsPage::OnWizardPageFinished)

END_EVENT_TABLE()

