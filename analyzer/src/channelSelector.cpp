/*
 * channelSelector.cpp
 *
 *  Created on: 2009-07-08
 *      Author: brent
 */
#include "channelSelector.h"

#define SCROLLBAR_RANGE 10000
#define	SCROLLBAR_THUMBSIZE 100
#define SCROLLBAR_PAGESIZE 100


ChannelSelectorPanel::ChannelSelectorPanel() : wxPanel()
{
	InitComponents();
}

ChannelSelectorPanel::ChannelSelectorPanel(wxWindow *parent,
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
						name)
{
	InitComponents();
}

ChannelSelectorPanel::~ChannelSelectorPanel(){

}

void ChannelSelectorPanel::InitComponents(){

	wxFlexGridSizer *sizer = new wxFlexGridSizer(1,1,0,0);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);

	m_channelGrid = new wxGrid(this,-1);
	m_channelGrid->CreateGrid(0,2);

	sizer->Add(m_channelGrid,1,wxEXPAND);
	this->SetSizer(sizer);

}

void ChannelSelectorPanel::SetDatalogStore(DatalogStore *store){
	m_datalogStore = store;
}

void ChannelSelectorPanel::RefreshChannels(){
	DatalogChannels channels;
	DatalogChannelTypes channelTypes;

	m_datalogStore->ReadChannels(channels);
	m_datalogStore->ReadChannelTypes(channelTypes);

	int existingRows = m_channelGrid->GetRows();
	if (existingRows > 0){
		m_channelGrid->DeleteRows(0,existingRows);
	}

	size_t channelCount = channels.Count();
	m_channelGrid->InsertRows(0,channelCount);

	for (size_t i = 0; i < channelCount; i++){
		DatalogChannel &channel = channels[i];
		m_channelGrid->SetCellValue(i,0,channel.name);
		DatalogChannelType &type = channelTypes[channel.typeId];
		m_channelGrid->SetCellValue(i,1,type.name);
	}
}

BEGIN_EVENT_TABLE ( ChannelSelectorPanel, wxPanel )

END_EVENT_TABLE()




