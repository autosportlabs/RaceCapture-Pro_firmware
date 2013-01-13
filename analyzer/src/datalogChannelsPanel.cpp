/*
 * datalogChannelsPanel.cpp
 *
 *  Created on: Jun 9, 2009
 *      Author: brent
 */

#include "datalogChannelsPanel.h"
#include "addChannelWizardDialog.h"
#include "list_add.xpm"
#include "media_play_forward.xpm"
#include "media_play_backward.xpm"
#include "media_stop.xpm"
#include "media_pause.xpm"
#include "media_seek_forward.xpm"
#include "media_seek_backward.xpm"
#include "media_skip_backward.xpm"
#include "media_skip_forward.xpm"



#define GRID_ROWS 5


DatalogChannelsPanel::DatalogChannelsPanel(DatalogChannelsParams params,
			wxWindow *parent,
			wxWindowID id,
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
						name),
						m_markerOffset(0)
{
	m_datalogStore = params.datalogStore;
	m_appOptions = params.appOptions;
	m_appPrefs = params.appPrefs;
	m_raceCaptureConfig = params.raceCaptureConfig;
	InitComponents();
}

DatalogChannelsPanel::~DatalogChannelsPanel(){

}

void DatalogChannelsPanel::InitComponents(){

	wxFlexGridSizer *sizer = new wxFlexGridSizer(2,1,3,3);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(1);


	//initialize tool bar
	wxToolBar* toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER);

	toolBar->AddTool(ID_ADD_CHANNEL, list_add_xpm, wxT("Add Channel View"));

	toolBar->AddTool(ID_SKIP_DATALOG_REV, media_skip_backward_xpm, 	"Skip datalog to beginning");
	toolBar->AddTool(ID_SEEK_DATALOG_REV, media_seek_backward_xpm, 	"Seek datalog reverse");
	toolBar->AddTool(ID_PLAY_DATALOG_REV, media_play_backward_xpm, 	"Play datalog reverse");
	toolBar->AddTool(ID_PAUSE_DATALOG,	  media_pause_xpm, 			"Pause datalog");
	toolBar->AddTool(ID_PLAY_DATALOG_FWD, media_play_forward_xpm, 	"Play datalog forward");
	toolBar->AddTool(ID_SEEK_DATALOG_FWD, media_seek_forward_xpm, 	"Seek datalog forward");
	toolBar->AddTool(ID_SKIP_DATALOG_FWD, media_skip_forward_xpm, 	"Skip datalog to end");

	toolBar->Realize();
	sizer->Add(toolBar,1,wxEXPAND);

	m_datalogSessionsNotebook = new wxNotebook(this, ID_DATALOG_SESSIONS_NOTEBOOK);

	sizer->Add(m_datalogSessionsNotebook,1,wxEXPAND);
	this->SetSizer(sizer);

	m_gridPopupMenu = new wxMenu();
	m_gridPopupMenu->Append(ID_NEW_LINE_CHART,"New Line Chart");
	m_gridPopupMenu->Append(ID_NEW_ANALOG_GAUGE,"New Analog Gauge");
	m_gridPopupMenu->Append(ID_NEW_DIGITAL_GAUGE, "New Digital Gauge");
	m_gridPopupMenu->Append(ID_NEW_GPS_VIEW, "New GPS View");


}


void DatalogChannelsPanel::InitOptions(){

}

void DatalogChannelsPanel::AddDatalogSession(int datalogId){

	INFO(FMT("Adding Datalog Session ID %d", datalogId));

	wxGrid *grid = new wxGrid(m_datalogSessionsNotebook, ID_DATALOG_CHANNELS_GRID);


	grid->SetEditable(false);
	wxString name;
	wxString notes;
	int timeOffset;
	m_datalogStore->ReadDatalogInfo(datalogId,timeOffset,name,notes);

	INFO(FMT("Read Datalog Info %s",name.ToAscii()));
	m_datalogSessionsNotebook->AddPage(grid,name);

	grid->CreateGrid(0,GRID_ROWS);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->EnableDragRowSize(false);

	grid->SetColLabelValue(0,"Channel");
	grid->SetColLabelValue(1,"Value");
	grid->SetColLabelValue(2,"Units");
	grid->SetColLabelValue(3,"Min");
	grid->SetColLabelValue(4,"Max");

	DatalogChannels channels;
	m_datalogStore->GetChannels(datalogId, channels);

	DatalogChannelTypes channelTypes;
	m_datalogStore->GetChannelTypes(channelTypes);

	ReloadChannels(channels,channelTypes,grid);
	grid->AutoSize();


}


void DatalogChannelsPanel::SetMarkerOffset(size_t offset){
	m_markerOffset = offset;
	UpdateRuntimeValues();
}


size_t DatalogChannelsPanel::GetMarkerOffset(){
	return m_markerOffset;
}

void DatalogChannelsPanel::UpdateRuntimeValues(){

	//DatastoreRow row = m_
	//m_markerOffset =

//	wxArrayString names;
//	m_datalogStore->GetExistingChannels(names);

}

void DatalogChannelsPanel::UpdateDatalogSessions(){

	if (m_datalogStore->IsOpen()){
		m_datalogIdList.Clear();
		m_datalogStore->ReadDatalogIds(m_datalogIdList);
		m_datalogSessionsNotebook->DeleteAllPages();

		size_t ids = m_datalogIdList.size();
		for (size_t i = 0; i < ids; i++){
			int id = m_datalogIdList[i];
			AddDatalogSession(id);
		}
	}
}

void DatalogChannelsPanel::ReloadChannels(DatalogChannels &channels, DatalogChannelTypes &channelTypes, wxGrid *grid){

	if (NULL == m_datalogStore){
		return;
	}

	int existingRows = grid->GetNumberRows();
	if (existingRows > 0) grid->DeleteRows(0,existingRows);
	size_t channelsSize = channels.size();
	grid->AppendRows(channelsSize);
	for (size_t i = 0; i < channelsSize; i++){
		DatalogChannel &channel = channels[i];
		grid->SetCellValue(i,0,channel.name);

		int typeId = channel.typeId;
		if (typeId >=0){
			DatalogChannelType &type = channelTypes[channel.typeId];

			grid->SetCellValue(i,1,type.name);
			grid->SetCellValue(i,2,type.unitsLabel);
			grid->SetCellValue(i,3,wxString::Format("%.2f",type.minValue));
			grid->SetCellValue(i,4,wxString::Format("%.2f",type.maxValue));
		}
	}

	UpdateRuntimeValues();
}

void DatalogChannelsPanel::PopulateSelectedChannels(DatalogChannelSelectionSet *selectionSet){

	size_t selectedPage = m_datalogSessionsNotebook->GetSelection();

	int datalogId = m_datalogIdList[selectedPage];

	wxArrayString selectedChannelIds;

	DatalogChannels channels;
	m_datalogStore->GetChannels(datalogId,channels);
	m_datalogSessionsNotebook->GetPage(selectedPage);
	wxGrid *grid = (wxGrid *)m_datalogSessionsNotebook->GetPage(selectedPage);

	size_t rowCount = grid->GetRows();
	for (size_t i = 0; i < rowCount; i++){
		if (grid->IsInSelection(i,0)){
			wxString channelName = grid->GetCellValue(i,0);
			selectedChannelIds.Add(channelName);
		}
	}

	selectionSet->Add(DatalogChannelSelection(datalogId,selectedChannelIds));

}

void DatalogChannelsPanel::OnNewLineChart(wxCommandEvent &event){

	DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
	PopulateSelectedChannels(selectionSet);
	wxCommandEvent addEvent( ADD_NEW_LINE_CHART_EVENT, ADD_NEW_LINE_CHART );
	addEvent.SetClientData(selectionSet);
	GetParent()->GetEventHandler()->AddPendingEvent(addEvent);
}

void DatalogChannelsPanel::OnNewAnalogGauge(wxCommandEvent &event){

	DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
	PopulateSelectedChannels(selectionSet);
	wxCommandEvent addEvent( ADD_NEW_ANALOG_GAUGE_EVENT, ADD_NEW_ANALOG_GAUGE );
	addEvent.SetClientData(selectionSet);
	GetParent()->GetEventHandler()->AddPendingEvent(addEvent);
}

void DatalogChannelsPanel::OnNewDigitalGauge(wxCommandEvent &event){

	DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
	PopulateSelectedChannels(selectionSet);
	wxCommandEvent addEvent( ADD_NEW_DIGITAL_GAUGE_EVENT, ADD_NEW_DIGITAL_GAUGE );
	addEvent.SetClientData(selectionSet);
	GetParent()->GetEventHandler()->AddPendingEvent(addEvent);
}

void DatalogChannelsPanel::OnNewGPSView(wxCommandEvent &event){
	DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
	PopulateSelectedChannels(selectionSet);
	wxCommandEvent addEvent( ADD_NEW_GPS_VIEW_EVENT, ADD_NEW_GPS_VIEW );
	addEvent.SetClientData(selectionSet);
	GetParent()->GetEventHandler()->AddPendingEvent(addEvent);
}


void DatalogChannelsPanel::OnAddChannelView(wxCommandEvent &event){

	AddChannelWizard *wiz = new AddChannelWizard(GetParent(),AddChannelWizardParams(m_appPrefs,m_appOptions,m_datalogStore, m_raceCaptureConfig));
	wiz->ShowPage(wiz->GetFirstPage());
	wiz->Show(true);
}

void DatalogChannelsPanel::OnPlayForward(wxCommandEvent &event){
	wxCommandEvent playEvent( PLAY_FWD_DATALOG_EVENT, PLAY_FWD_DATALOG);
	GetParent()->GetEventHandler()->AddPendingEvent(playEvent);
}

void DatalogChannelsPanel::OnPause(wxCommandEvent &event){
	wxCommandEvent pauseEvent( PAUSE_DATALOG_EVENT, PAUSE_DATALOG);
	GetParent()->GetEventHandler()->AddPendingEvent(pauseEvent);
}

void DatalogChannelsPanel::OnPlayReverse(wxCommandEvent &event){
	wxCommandEvent playEvent( PLAY_REV_DATALOG_EVENT, PLAY_REV_DATALOG);
	GetParent()->GetEventHandler()->AddPendingEvent(playEvent);
}

void DatalogChannelsPanel::OnSkipForward(wxCommandEvent &event){
	wxCommandEvent evt( JUMP_END_DATALOG_EVENT, JUMP_END_DATALOG);
	GetParent()->GetEventHandler()->AddPendingEvent(evt);
}

void DatalogChannelsPanel::OnSkipReverse(wxCommandEvent &event){
	wxCommandEvent evt( JUMP_BEGINNING_DATALOG_EVENT, JUMP_BEGINNING_DATALOG);
	GetParent()->GetEventHandler()->AddPendingEvent(evt);
}

void DatalogChannelsPanel::OnSeekForward(wxCommandEvent &event){
	wxCommandEvent evt( SEEK_FWD_DATALOG_EVENT, SEEK_FWD_DATALOG);
	GetParent()->GetEventHandler()->AddPendingEvent(evt);
}

void DatalogChannelsPanel::OnSeekReverse(wxCommandEvent &event){
	wxCommandEvent evt( SEEK_REV_DATALOG_EVENT, SEEK_REV_DATALOG);
	GetParent()->GetEventHandler()->AddPendingEvent(evt);
}

void DatalogChannelsPanel::DoGridContextMenu(wxGridEvent &event){
	PopupMenu(m_gridPopupMenu);
}

BEGIN_EVENT_TABLE ( DatalogChannelsPanel, wxPanel )
	EVT_MENU(ID_NEW_LINE_CHART,DatalogChannelsPanel::OnNewLineChart)
	EVT_MENU(ID_NEW_ANALOG_GAUGE, DatalogChannelsPanel::OnNewAnalogGauge)
	EVT_MENU(ID_NEW_DIGITAL_GAUGE, DatalogChannelsPanel::OnNewDigitalGauge)
	EVT_MENU(ID_NEW_GPS_VIEW, DatalogChannelsPanel::OnNewGPSView)
	EVT_MENU(ID_ADD_CHANNEL, DatalogChannelsPanel::OnAddChannelView)

	EVT_MENU(ID_SKIP_DATALOG_REV, DatalogChannelsPanel::OnSkipReverse)
	EVT_MENU(ID_SEEK_DATALOG_REV, DatalogChannelsPanel::OnSeekReverse)
	EVT_MENU(ID_PLAY_DATALOG_REV, DatalogChannelsPanel::OnPlayReverse)
	EVT_MENU(ID_PAUSE_DATALOG, DatalogChannelsPanel::OnPause)
	EVT_MENU(ID_PLAY_DATALOG_FWD, DatalogChannelsPanel::OnPlayForward)
	EVT_MENU(ID_SEEK_DATALOG_FWD, DatalogChannelsPanel::OnSeekForward)
	EVT_MENU(ID_SKIP_DATALOG_FWD, DatalogChannelsPanel::OnSkipForward)

	EVT_GRID_CELL_RIGHT_CLICK(DatalogChannelsPanel::DoGridContextMenu)
END_EVENT_TABLE()
