/*
 * AddChannelWizardDialog.cpp
 *
 *  Created on: Oct 3, 2011
 *      Author: brent
 */

#include "addChannelWizardDialog.h"
#include "wx/file.h"
#include "logging.h"
#include "wxUtils.h"

#include "lapDetectorNewChannelProcessor.h"
#include "scaledValueNewChannelProcessor.h"


enum{
	ID_ADD_CHANNEL_WIZ_SELECT = wxID_HIGHEST + 1,
	ID_ADD_CHANNEL_NEW_CHANNELS_GRID,
	ID_ADD_CHANNEL_WIZ_START_ADD_CHANNEL,
	ID_ADD_CHANNEL_WIZ_ADD_CHANNEL_PROGRESS,
	ID_ADD_CHANNEL_WIZ_ADD_CHANNEL_RESULT
};

#include <wx/arrimpl.cpp> // This is a magic incantation which must be done!
 WX_DEFINE_OBJARRAY(ChannelProcessorDescriptions);

ChannelProcessorDescription::ChannelProcessorDescription(wxString name, wxString description, NewChannelProcessor processor, NewChannelOptionsPageFactory * wizardPageFactory) :
		m_name(name),m_description(description),m_processor(processor), m_wizardPageFactory(wizardPageFactory)
{}

NewChannelFinishPage::NewChannelFinishPage(wxWizard *parent) : wxWizardPageSimple(parent){

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(new wxStaticText(this, -1, "Complete. The New channel has been added to the channel list"),1,wxALL | wxEXPAND,1 );
    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

NewChannelProgressPage::NewChannelProgressPage(wxWizard *parent, AddChannelWizardParams *params) :
	wxWizardPageSimple(parent),m_params(params){
}

void NewChannelProgressPage::OnWizardPageChanged(wxWizardEvent &event){

	/*
	m_importing = false;
	UpdateUIState();
	*/
}

void NewChannelProgressPage::OnWizardPageChanging(wxWizardEvent &event){

	/*
	m_importing = false;
	UpdateUIState();
	*/
}

void NewChannelProgressPage::OnWizardPageFinished(wxWizardEvent &event){

}

void NewChannelProgressPage::OnStartNewChannel(wxCommandEvent &event){

}

void NewChannelProgressPage::OnNewChannelProgress(wxCommandEvent &event){
	//	int percent = event.GetInt();
	//	m_progressGauge->SetValue(percent);
	//	m_progressMessage->SetLabel(wxString::Format("Progress: %d %%",percent));

}

void NewChannelProgressPage::OnNewChannelResult(wxCommandEvent &event){

}

BEGIN_EVENT_TABLE ( NewChannelProgressPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, NewChannelProgressPage::OnWizardPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, NewChannelProgressPage::OnWizardPageChanging)
	EVT_WIZARD_FINISHED(wxID_ANY, NewChannelProgressPage::OnWizardPageFinished)
	EVT_BUTTON(ID_ADD_CHANNEL_WIZ_START_ADD_CHANNEL, NewChannelProgressPage::OnStartNewChannel)
	EVT_COMMAND( ID_ADD_CHANNEL_WIZ_ADD_CHANNEL_PROGRESS, ADD_CHANNEL_PROGRESS_EVENT, NewChannelProgressPage::OnNewChannelProgress )
	EVT_COMMAND( ID_ADD_CHANNEL_WIZ_ADD_CHANNEL_RESULT, ADD_CHANNEL_RESULT_EVENT, NewChannelProgressPage::OnNewChannelResult)

END_EVENT_TABLE()


SelectNewChannelTypePage::SelectNewChannelTypePage(wxWizard *parent, AddChannelWizardParams *params, ChannelProcessorDescriptions *processorDescriptions) :
	wxWizardPageSimple(parent),m_params(params),selectedChannelIndex(-1),m_processorDescriptions(processorDescriptions){

	m_wizardParent = parent;
	wxFlexGridSizer *innerSizer = new wxFlexGridSizer(2,1,3,3);
	innerSizer->AddGrowableCol(0);
	innerSizer->AddGrowableRow(1);

	innerSizer->Add(new wxStaticText(this, -1, "Select Channel Type"));

	wxGrid *grid = CreateNewChannelsGrid();
	PopulateAvailableNewChannels(parent, grid,processorDescriptions);
	grid->SelectRow(0,false);

	grid->AutoSize();

	innerSizer->Add(grid,1, wxALL | wxEXPAND);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(innerSizer,1,wxALL | wxEXPAND,1 );

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

void SelectNewChannelTypePage::AppendNewChannelType(wxGrid *grid, ChannelProcessorDescription &channelProcessorDescription){
	grid->AppendRows(1);
	int lastIndex = grid->GetRows() - 1;
	grid->SetCellValue(lastIndex,0,channelProcessorDescription.m_name);
	grid->SetCellValue(lastIndex,1,channelProcessorDescription.m_description);
}

void SelectNewChannelTypePage::PopulateAvailableNewChannels(wxWizard *wizardParent, wxGrid *grid, ChannelProcessorDescriptions *processorDescriptions){

	for (size_t i = 0; i < processorDescriptions->size();i++){
		ChannelProcessorDescription d = processorDescriptions->Item(i);
		AppendNewChannelType(grid,d);
	}
}

wxGrid * SelectNewChannelTypePage::CreateNewChannelsGrid(){

	wxGrid *grid = new wxGrid(this, ID_ADD_CHANNEL_NEW_CHANNELS_GRID);

	grid->SetEditable(false);
	wxString name;
	wxString notes;

	grid->CreateGrid(0,2);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->EnableDragRowSize(false);

	grid->SetColLabelValue(0,"Channel");
	grid->SetColLabelValue(1,"Description");

	return grid;
}


void SelectNewChannelTypePage::UpdateUIState(){

	bool itemSelected = selectedChannelIndex >= 0;
	GetParent()->FindWindow(wxID_FORWARD)->Enable(itemSelected);
}

void SelectNewChannelTypePage::OnWizardPageChanged(wxWizardEvent &event){

	UpdateUIState();
}

void SelectNewChannelTypePage::OnWizardPageChanging(wxWizardEvent &event){

	/*
	m_importing = false;
	UpdateUIState();
	*/
}
void SelectNewChannelTypePage::OnGridSelected(wxGridEvent &event){
	selectedChannelIndex = event.GetRow();
	wxWizardPage *oldPage = this->GetNext();
	if (NULL != oldPage){
		wxWizardPageSimple *nextPage = m_processorDescriptions->Item(selectedChannelIndex).m_wizardPageFactory->createChannelProcessor(m_wizardParent,m_params);
		nextPage->SetNext(oldPage->GetNext());
		nextPage->SetPrev(this);
		this->SetNext(nextPage);
		delete(oldPage);
	}
	UpdateUIState();
}

BEGIN_EVENT_TABLE ( SelectNewChannelTypePage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, SelectNewChannelTypePage::OnWizardPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, SelectNewChannelTypePage::OnWizardPageChanging)
	EVT_GRID_SELECT_CELL(SelectNewChannelTypePage::OnGridSelected)

END_EVENT_TABLE()


AddChannelWizard::AddChannelWizard(wxWindow *frame, AddChannelWizardParams params) :
	wxWizard(frame,wxID_ANY,_T("Add Channel"),
	                 wxBitmap(),wxDefaultPosition,
	                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
	                 m_params(params)

{
	initChannelProcessors();
	m_startPage = new SelectNewChannelTypePage(this, &m_params,&m_channelProcessors);
	wxWizardPageSimple *blankPage = new wxWizardPageSimple(this);
	m_progressPage = new NewChannelProgressPage(this, &m_params);
	wxWizardPageSimple *finishPage = new NewChannelFinishPage(this);

	m_startPage->SetNext(blankPage);
	blankPage->SetPrev(m_startPage);
	blankPage->SetNext(m_progressPage);
	m_progressPage->SetNext(finishPage);

	GetPageAreaSizer()->Add(m_startPage);
}

void AddChannelWizard::SetAddChannelOptionsPage(wxWizardPageSimple *page){

	m_startPage->SetNext(page);
	page->SetPrev(m_startPage);
	page->SetNext(m_progressPage);
	m_progressPage->SetPrev(page);
}

void AddChannelWizard::initChannelProcessors(){

	ChannelProcessorDescription scaledValue("Scaled Value",
			"Create a new channel that is linearly scaled from an existing channel",
			ScaledValueNewChannelProcessor(),
			new ScaledValueNewChannelOptionsPageFactory()
			);
	m_channelProcessors.Add(scaledValue);

	ChannelProcessorDescription lapDetector("Lap Detector",
			"Detect current Lap via GPS-based Start/Finish detection",
			LapDetectorNewChannelProcessor(),
			new LapDetectorNewChannelOptionsPageFactory()
			);
	m_channelProcessors.Add(lapDetector);
}
