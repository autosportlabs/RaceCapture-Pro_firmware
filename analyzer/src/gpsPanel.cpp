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

//	% WGS84 ellipsoid constants:
double a = 6378137;
double e = 8.1819190842622e-2;

//	% intermediate calculation
//	% (prime vertical radius of curvature)
double N = a / sqrt(1 - pow(e,2) * pow(sin(latitude),2));

//	% results:
	double x = (N) * cos(latitude) * cos(longitude);
	double y = (N) * cos(latitude) * sin(longitude);


	x = x / 100000;
	y = y / 100000;

//	double x = longitude*60*1852*cos(latitude);
//	double y = latitude*60*1852;
	//VERBOSE(FMT("%10.5f,%10.5f",x, y));

	m_gpsView->AddGPSPoint(GPSPoint(x,y,0,0,0));
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

