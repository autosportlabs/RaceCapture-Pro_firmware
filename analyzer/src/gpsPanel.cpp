/*
 * gpsPanel.cpp
 *
 *  Created on: 2009-07-22
 *      Author: brent
 */
#include "gpsPanel.h"
#include "logging.h"

GPSPane::GPSPane() : wxPanel()
{
	InitComponents();
}

GPSPane::GPSPane(wxWindow *parent,
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

GPSPane::~GPSPane(){}

void GPSPane::CreateGPSView(int datalogId, int latitudeChannelId, int longitudeChannelId){
	m_datalogId = datalogId;
	m_latitudeChannelId = latitudeChannelId;
	m_longitudeChannelId = longitudeChannelId;

	DatalogStore *store = m_chartParams.datalogStore;

	DatalogChannels channels;
	store->GetChannels(datalogId, channels);

	wxArrayString channelNames;
	channelNames.Add(channels[latitudeChannelId].name);
	channelNames.Add(channels[longitudeChannelId].name);
	store->ReadDatalog(m_channelData,datalogId,channelNames,0);

	SetOffset(0);
	ReloadGPSPoints();
}

void GPSPane::SetOffset(size_t offset){
	m_dataOffset = offset;
}

void GPSPane::ReloadGPSPoints(){

	size_t count = m_channelData.Count();
	for (size_t i = 0; i < count; i++){
		DatastoreRow &row = m_channelData[i];

		double latitude = row.values[LATITUDE_INDEX];
		double longitude = row.values[LONGITUDE_INDEX];
		if (latitude != DatastoreRow::NULL_VALUE && longitude != DatastoreRow::NULL_VALUE){
			AddGPSPoint(latitude,longitude);
		}
	}
}

void GPSPane::AddGPSPoint(double latitude, double longitude){

	double pi = 3.14159;
	// convert lat/long to radians
	latitude = pi * latitude / 180;
	longitude = pi * longitude / 180;

	// adjust position by radians
	latitude -= 1.570795765134; // subtract 90 degrees (in radians)

	double world_radius = 180;
	// and switch z and y
	double x = (world_radius) * sin(latitude) * cos(longitude);
	double z = (world_radius) * sin(latitude) * sin(longitude);
	double y = (world_radius) * cos(latitude);

	double focal_length = 150;

	double projected_x = x * focal_length / (focal_length + z);
	double projected_y = y * focal_length / (focal_length + z);

	projected_x = - projected_x;
	projected_y = - projected_y;

	m_gpsView->AddGPSPoint(GPSPoint(projected_x,projected_y,0,0,0));
}

void GPSPane::InitComponents(){

	wxFlexGridSizer *sizer = new wxFlexGridSizer(1,1,0,0);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);
	m_gpsView = new GPSView(this,-1);
	m_gpsView->SetMinSize(wxSize(400,400));
	sizer->Add(m_gpsView,1,wxEXPAND);
	this->SetSizer(sizer);
}

void GPSPane::SetChartParams(ChartParams params){
	m_chartParams = params;
}


BEGIN_EVENT_TABLE ( GPSPane , wxPanel )
END_EVENT_TABLE()

