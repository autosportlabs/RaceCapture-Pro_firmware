/*
 * datalogChannelsPanel.cpp
 *
 *  Created on: Jun 9, 2009
 *      Author: brent
 */

#include "datalogChannelsPanel.h"
#include "addChannelWizardDialog.h"
#include "add.xpm"

#define GRID_ROWS 5

DatalogChannelsPanel::DatalogChannelsPanel() : wxPanel(), m_markerOffset(0)
{
	InitComponents();
}

DatalogChannelsPanel::DatalogChannelsPanel(wxWindow *parent,
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
	wxBitmap bmpNew(add_xpm);

	toolBar->AddTool(ID_ADD_CHANNEL, bmpNew, wxT("Add Channel"));
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

	m_datalogIdList.Clear();
	m_datalogStore->ReadDatalogIds(m_datalogIdList);
	m_datalogSessionsNotebook->DeleteAllPages();

	size_t ids = m_datalogIdList.size();
	for (size_t i = 0; i < ids; i++){
		AddDatalogSession(m_datalogIdList[i]);
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
		DatalogChannelType &type = channelTypes[channel.typeId];

		grid->SetCellValue(i,1,type.name);
		grid->SetCellValue(i,2,type.unitsLabel);
		grid->SetCellValue(i,3,wxString::Format("%.2f",type.minValue));
		grid->SetCellValue(i,4,wxString::Format("%.2f",type.maxValue));
	}

	UpdateRuntimeValues();
}


void DatalogChannelsPanel::SetDatalogStore(DatalogStore * datalogStore){
	m_datalogStore = datalogStore;
}

void DatalogChannelsPanel::SetAppOptions(AppOptions *appOptions){
	m_appOptions = appOptions;
}

void DatalogChannelsPanel::SetAppPrefs(AppPrefs *appPrefs){
	m_appPrefs = appPrefs;
}

void DatalogChannelsPanel::PopulateSelectedChannels(DatalogChannelSelectionSet *selectionSet){

	size_t selectedPage = m_datalogSessionsNotebook->GetSelection();

	int datalogId = m_datalogIdList[selectedPage];

	wxArrayInt selectedChannelIds;

	DatalogChannels channels;
	m_datalogStore->GetChannels(datalogId,channels);
	m_datalogSessionsNotebook->GetPage(selectedPage);
	wxGrid *grid = (wxGrid *)m_datalogSessionsNotebook->GetPage(selectedPage);

	size_t rowCount = grid->GetRows();
	for (size_t i = 0; i < rowCount; i++){
		if (grid->IsInSelection(i,0)){
			wxString channelName = grid->GetCellValue(i,0);
			int channelId = DatalogChannelUtil::FindChannelIdByName(channels,channelName);
			if (channelId >= 0) selectedChannelIds.Add(channelId);
		}
	}

	selectionSet->Add(DatalogChannelSelection(datalogId,selectedChannelIds));

}

void DatalogChannelsPanel::OnNewLineChart(wxCommandEvent &event){

	DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
	PopulateSelectedChannels(selectionSet);
	wxCommandEvent addEvent( ADD_NEW_LINE_CHART_EVENT, ADD_NEW_LINE_CHART );
	addEvent.SetClientData(selectionSet);
	GetParent()->AddPendingEvent(addEvent);
}

void DatalogChannelsPanel::OnNewAnalogGauge(wxCommandEvent &event){

	DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
	PopulateSelectedChannels(selectionSet);
	wxCommandEvent addEvent( ADD_NEW_ANALOG_GAUGE_EVENT, ADD_NEW_ANALOG_GAUGE );
	addEvent.SetClientData(selectionSet);
	GetParent()->AddPendingEvent(addEvent);
}

void DatalogChannelsPanel::OnNewDigitalGauge(wxCommandEvent &event){

	DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
	PopulateSelectedChannels(selectionSet);
	wxCommandEvent addEvent( ADD_NEW_DIGITAL_GAUGE_EVENT, ADD_NEW_DIGITAL_GAUGE );
	addEvent.SetClientData(selectionSet);
	GetParent()->AddPendingEvent(addEvent);
}

void DatalogChannelsPanel::OnAddChannel(wxCommandEvent &event){

	AddChannelWizard *wiz = new AddChannelWizard(GetParent(),AddChannelWizardParams(m_appPrefs,m_appOptions,m_datalogStore));
	wiz->ShowPage(wiz->GetFirstPage());
	wiz->Show(true);
}

void DatalogChannelsPanel::OnNewGPSView(wxCommandEvent &event){
	DatalogChannelSelectionSet *selectionSet = new DatalogChannelSelectionSet();
	PopulateSelectedChannels(selectionSet);
	wxCommandEvent addEvent( ADD_NEW_GPS_VIEW_EVENT, ADD_NEW_GPS_VIEW );
	addEvent.SetClientData(selectionSet);
	GetParent()->AddPendingEvent(addEvent);
}

void DatalogChannelsPanel::DoGridContextMenu(wxGridEvent &event){
	PopupMenu(m_gridPopupMenu);
}

BEGIN_EVENT_TABLE ( DatalogChannelsPanel, wxPanel )
	EVT_MENU(ID_NEW_LINE_CHART,DatalogChannelsPanel::OnNewLineChart)
	EVT_MENU(ID_NEW_ANALOG_GAUGE, DatalogChannelsPanel::OnNewAnalogGauge)
	EVT_MENU(ID_NEW_DIGITAL_GAUGE, DatalogChannelsPanel::OnNewDigitalGauge)
	EVT_MENU(ID_NEW_GPS_VIEW, DatalogChannelsPanel::OnNewGPSView)
	EVT_MENU(ID_ADD_CHANNEL, DatalogChannelsPanel::OnAddChannel)

	EVT_GRID_CELL_RIGHT_CLICK(DatalogChannelsPanel::DoGridContextMenu)
END_EVENT_TABLE()
