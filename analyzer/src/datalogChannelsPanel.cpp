/*
 * datalogChannelsPanel.cpp
 *
 *  Created on: Jun 9, 2009
 *      Author: brent
 */

#include "datalogChannelsPanel.h"

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

	wxFlexGridSizer *sizer = new wxFlexGridSizer(1,1,3,3);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);

	m_datalogSessions = new wxNotebook(this, ID_DATALOG_SESSIONS_NOTEBOOK);

	sizer->Add(m_datalogSessions,1,wxEXPAND);
	this->SetSizer(sizer);
}


void DatalogChannelsPanel::InitOptions(){

}

void DatalogChannelsPanel::AddDatalogSession(int id){

	INFO(FMT("Adding Datalog Session ID %d", id));

	wxGrid *grid = new wxGrid(m_datalogSessions, ID_DATALOG_CHANNELS_GRID);

	wxString name;
	wxString notes;
	int timeOffset;
	m_datalogStore->ReadDatalogInfo(id,timeOffset,name,notes);

	INFO(FMT("Read Datalog Info %s",name.ToAscii()));
	m_datalogSessions->AddPage(grid,name);

	grid->CreateGrid(0,GRID_ROWS);

	grid->SetColLabelValue(0,"Channel");
	grid->SetColLabelValue(1,"Value");
	grid->SetColLabelValue(2,"Units");
	grid->SetColLabelValue(3,"Min");
	grid->SetColLabelValue(4,"Max");

	wxArrayString channelNames;
	m_datalogStore->GetChannelNames(channelNames);

	ReloadChannels(channelNames,grid);

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

	wxArrayInt datalogIds;
	m_datalogStore->ReadDatalogIds(datalogIds);

	m_datalogSessions->DeleteAllPages();

	size_t ids = datalogIds.size();
	for (size_t i = 0; i < ids; i++){
		AddDatalogSession(datalogIds[i]);
	}
}

void DatalogChannelsPanel::ReloadChannels(wxArrayString &names, wxGrid *grid){

	if (NULL == m_datalogStore){
		return;
	}

	int existingRows = grid->GetNumberRows();
	if (existingRows > 0) grid->DeleteRows(0,existingRows);
	size_t nameSize = names.size();
	grid->AppendRows(nameSize);
	for (size_t i = 0; i < nameSize; i++){
		grid->SetCellValue(i,0,names[i]);
	}
	UpdateRuntimeValues();
}


void DatalogChannelsPanel::SetDatalogStore(DatalogStore * datalogStore){
	m_datalogStore = datalogStore;
}

BEGIN_EVENT_TABLE ( DatalogChannelsPanel, wxPanel )
END_EVENT_TABLE()
