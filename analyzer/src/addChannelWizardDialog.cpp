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


enum{
	ID_ADD_CHANNEL_WIZ_SELECT = wxID_HIGHEST + 1,
	ID_ADD_CHANNEL_NEW_CHANNELS_GRID,
	ID_ADD_CHANNEL_GRID,
	ID_ADD_CHANNEL_WIZ_START_ADD_CHANNEL,
	ID_ADD_CHANNEL_WIZ_ADD_CHANNEL_PROGRESS,
	ID_ADD_CHANNEL_WIZ_ADD_CHANNEL_RESULT
};


NewChannelFinishPage::NewChannelFinishPage(wxWizard *parent) : wxWizardPageSimple(parent){

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(new wxStaticText(this, -1, "Complete. Press finish to add the selected channels to the view."),1,wxALL | wxEXPAND,1 );
    SetSizer(mainSizer);
    mainSizer->Fit(this);
}


ChannelSelectorPage::ChannelSelectorPage(wxWizard *parent, AddChannelWizardParams *params) :
	wxWizardPageSimple(parent),m_params(params){

	wxFlexGridSizer *innerSizer = new wxFlexGridSizer(2,1,3,3);
	innerSizer->AddGrowableCol(0);
	innerSizer->AddGrowableRow(1);

	innerSizer->Add(new wxStaticText(this, -1, "Select Channel"));

	wxGrid *grid = CreateChannelsGrid();
	PopulateAvailableChannels(grid, params->raceCaptureConfig);
	grid->SelectRow(0,false);

	grid->AutoSize();

	innerSizer->Add(grid,1, wxALL | wxEXPAND);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(innerSizer,1,wxALL | wxEXPAND,1 );

    m_channelsGrid = grid;

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

wxArrayString ChannelSelectorPage::GetSelectedChannels(){

	wxArrayString selectedNames;
	size_t count = m_channelsGrid->GetRows();
	for (size_t i = 0; i < count; i++){
		if (m_channelsGrid->IsInSelection(i,0)){
			selectedNames.Add(m_channelsGrid->GetCellValue(i,0));
		}
	}
	return selectedNames;
}

wxGrid * ChannelSelectorPage::CreateChannelsGrid(){

	wxGrid *grid = new wxGrid(this, ID_ADD_CHANNEL_GRID);

	grid->SetEditable(false);
	wxString name;
	wxString notes;

	grid->CreateGrid(0,2);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->EnableDragRowSize(false);

	grid->SetColLabelValue(0,"Channel");
	grid->SetColLabelValue(1,"Units");

	return grid;
}


void ChannelSelectorPage::AppendChannelItem(wxGrid *grid, ChannelConfig *channelConfig){
	grid->AppendRows(1);
	int lastIndex = grid->GetRows() - 1;
	grid->SetCellValue(lastIndex,0,channelConfig->label);
	grid->SetCellValue(lastIndex,1,channelConfig->units);
}

void ChannelSelectorPage::PopulateAvailableChannels(wxGrid *grid, RaceCaptureConfig *config){

	AppendChannelItem(grid, &config->gpsConfig.lapCountCfg);
	AppendChannelItem(grid, &config->gpsConfig.lapTimeCfg);
	AppendChannelItem(grid, &config->gpsConfig.qualityCfg);
	AppendChannelItem(grid, &config->gpsConfig.satellitesCfg);
	AppendChannelItem(grid, &config->gpsConfig.latitudeCfg);
	AppendChannelItem(grid, &config->gpsConfig.longitudeCfg);
	AppendChannelItem(grid, &config->gpsConfig.timeCfg);
	AppendChannelItem(grid, &config->gpsConfig.velocityCfg);

	for (size_t i = 0; i < CONFIG_ANALOG_CHANNELS - 1; i++ ){
		AppendChannelItem(grid, &config->analogConfigs[i].channelConfig);
	}

	for (size_t i = 0; i < CONFIG_TIMER_CHANNELS - 1; i++){
		AppendChannelItem(grid, &config->timerConfigs[i].channelConfig);
	}

	for (size_t i = 0; i < CONFIG_ANALOG_PULSE_CHANNELS - 1; i++){
		AppendChannelItem(grid, &config->pwmConfigs[i].channelConfig);
	}

	for (size_t i = 0; i < CONFIG_GPIO_CHANNELS - 1; i++){
		AppendChannelItem(grid, &config->gpioConfigs[i].channelConfig);
	}

	for (size_t i = 0; i < CONFIG_ACCEL_CHANNELS - 1; i++){
		AppendChannelItem(grid, &config->accelConfigs[i].channelConfig);
	}
}

void ChannelSelectorPage::OnWizardPageChanged(wxWizardEvent &event){
	int i=1;
		int b=i;
	/*
	m_importing = false;
	UpdateUIState();
	*/
}

void ChannelSelectorPage::OnWizardPageChanging(wxWizardEvent &event){
	int i=1;
		int b=i;
	/*
	m_importing = false;
	UpdateUIState();
	*/
}

BEGIN_EVENT_TABLE ( ChannelSelectorPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, ChannelSelectorPage::OnWizardPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, ChannelSelectorPage::OnWizardPageChanging)

END_EVENT_TABLE()


SelectChannelViewPage::SelectChannelViewPage(wxWizard *parent, AddChannelWizardParams *params, ChannelViews *channelViews) :
	wxWizardPageSimple(parent),m_params(params),selectedChannelIndex(-1),m_channelViews(channelViews){

	m_wizardParent = parent;
	wxFlexGridSizer *innerSizer = new wxFlexGridSizer(2,1,3,3);
	innerSizer->AddGrowableCol(0);
	innerSizer->AddGrowableRow(1);

	innerSizer->Add(new wxStaticText(this, -1, "Select Channel Type"));

	m_channelViewSelectorGrid = CreateViewSelectorGrid();
	PopulateNewChannelViews(parent, m_channelViewSelectorGrid, channelViews);
	m_channelViewSelectorGrid->SelectRow(0,false);

	m_channelViewSelectorGrid->AutoSize();

	innerSizer->Add(m_channelViewSelectorGrid,1, wxALL | wxEXPAND);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(innerSizer,1,wxALL | wxEXPAND,1 );

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

ChannelViews SelectChannelViewPage::GetSelectedChannelViews(){

	ChannelViews selectedViews;

	size_t count = m_channelViewSelectorGrid->GetRows();
	for (size_t i = 0; i < count; i++){
		if (m_channelViewSelectorGrid->IsInSelection(i,0)){
			selectedViews.Add(m_channelViews->Item(i));
		}
	}
	return selectedViews;
}

void SelectChannelViewPage::AppendNewChannelView(wxGrid *grid, ChannelView &channelViewDescription){
	grid->AppendRows(1);
	int lastIndex = grid->GetRows() - 1;
	grid->SetCellValue(lastIndex,0,channelViewDescription.m_name);
	grid->SetCellValue(lastIndex,1,channelViewDescription.m_description);

}

void SelectChannelViewPage::PopulateNewChannelViews(wxWizard *wizardParent, wxGrid *grid, ChannelViews *channelViews){

	for (size_t i = 0; i < channelViews->size();i++){
		ChannelView d = channelViews->Item(i);
		AppendNewChannelView(grid,d);
	}
}

wxGrid * SelectChannelViewPage::CreateViewSelectorGrid(){

	wxGrid *grid = new wxGrid(this, ID_ADD_CHANNEL_NEW_CHANNELS_GRID);

	grid->SetEditable(false);
	wxString name;
	wxString notes;

	grid->CreateGrid(0,2);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->EnableDragRowSize(false);

	grid->SetColLabelValue(0,"View");
	grid->SetColLabelValue(1,"Description");

	return grid;
}


void SelectChannelViewPage::UpdateUIState(){

	bool itemSelected = selectedChannelIndex >= 0;
	GetParent()->FindWindow(wxID_FORWARD)->Enable(itemSelected);
}

void SelectChannelViewPage::OnWizardPageChanged(wxWizardEvent &event){

	UpdateUIState();
}

void SelectChannelViewPage::OnWizardPageChanging(wxWizardEvent &event){

	/*
	m_importing = false;
	UpdateUIState();
	*/
}
void SelectChannelViewPage::OnGridSelected(wxGridEvent &event){
	selectedChannelIndex = event.GetRow();
	UpdateUIState();
}

BEGIN_EVENT_TABLE ( SelectChannelViewPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, SelectChannelViewPage::OnWizardPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, SelectChannelViewPage::OnWizardPageChanging)
	EVT_GRID_SELECT_CELL(SelectChannelViewPage::OnGridSelected)

END_EVENT_TABLE()


AddChannelWizard::AddChannelWizard(wxWindow *frame, AddChannelWizardParams params) :
	wxWizard(frame,wxID_ANY,_T("Add Channel"),
	                 wxBitmap(),wxDefaultPosition,
	                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
	                 m_params(params)

{
	InitChannelViews();
	m_viewSelectorPage = new SelectChannelViewPage(this, &m_params,&m_channelViews);
	m_channelSelectorPage = new ChannelSelectorPage(this, &m_params);
	m_finishPage = new NewChannelFinishPage(this);

	m_viewSelectorPage->SetNext(m_channelSelectorPage);

	m_channelSelectorPage->SetNext(m_finishPage);
	m_channelSelectorPage->SetPrev(m_viewSelectorPage);

	m_finishPage->SetPrev(m_channelSelectorPage);

	GetPageAreaSizer()->Add(m_viewSelectorPage);
}

void AddChannelWizard::InitChannelViews(){

	ChannelView lineChartView("Line Chart", "A channel's value over time", ADD_NEW_LINE_CHART_EVENT, ADD_NEW_LINE_CHART);
	ChannelView analogGaugeView("Analog Gauge", "A channel's value displayed on an analog gauge", ADD_NEW_ANALOG_GAUGE_EVENT, ADD_NEW_ANALOG_GAUGE);
	ChannelView digitalGaugeView("Digital Gauge", "A channel's value displayed on a digital gauge", ADD_NEW_DIGITAL_GAUGE_EVENT, ADD_NEW_DIGITAL_GAUGE);
	ChannelView gpsGaugeView("GPS View", "A View showing the GPS coordinates of a pair of Latitude / Longitude channels", ADD_NEW_GPS_VIEW_EVENT, ADD_NEW_GPS_VIEW);

	m_channelViews.Add(lineChartView);
	m_channelViews.Add(analogGaugeView);
	m_channelViews.Add(digitalGaugeView);
	m_channelViews.Add(gpsGaugeView);
}

void AddChannelWizard::OnWizardFinished(wxWizardEvent &event){
	ChannelViews selectedViews = m_viewSelectorPage->GetSelectedChannelViews();
	wxArrayString selectedChannelNames = m_channelSelectorPage->GetSelectedChannels();

	size_t selectedViewsCount = selectedViews.Count();
	for (size_t viewIndex = 0; viewIndex < selectedViewsCount; viewIndex++){
		DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
		selectionSet->Add(DatalogChannelSelection(1,selectedChannelNames));
		ChannelView selectedView = selectedViews[viewIndex];
		wxCommandEvent addEvent(selectedView.m_eventType, selectedView.m_eventId);
		addEvent.SetClientData(selectionSet);
		GetParent()->AddPendingEvent(addEvent);
	}
}

BEGIN_EVENT_TABLE ( AddChannelWizard, wxWizard )

	EVT_WIZARD_FINISHED(wxID_ANY, AddChannelWizard::OnWizardFinished)

END_EVENT_TABLE()
