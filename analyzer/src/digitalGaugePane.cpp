/*
 * analogGaugePane.cpp
 *
 *  Created on: 2009-07-20
 *      Author: brent
 */

#define MAX_VALUE_PRECISION 4
static const char * VALUE_FORMAT[MAX_VALUE_PRECISION + 1] = {"%4.0f","%4.1f","%4.2f","%4.3f","%4.4f"};


#include "digitalGaugePane.h"
DigitalGaugePane::DigitalGaugePane() : wxPanel()
{
	InitComponents();
}

DigitalGaugePane::DigitalGaugePane(wxWindow *parent,
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
	m_valuePrecision = 0;
	InitComponents();
}

DigitalGaugePane::~DigitalGaugePane(){

}

void DigitalGaugePane::CreateGauge(int datalogId, wxString channelName){

	DatalogStore *store = m_chartParams.datalogStore;

	DatalogChannelTypes channelTypes;
	store->GetChannelTypes(channelTypes);

	DatalogChannel channel;
	store->GetChannel(datalogId,channelName,channel);
	DatalogChannelType &type = channelTypes[channel.typeId];

	AppOptions *options = m_chartParams.appOptions;

	DigitalGaugeTypes &gaugeTypes = options->GetDigitalGaugeTypes();

	if (gaugeTypes.find(type.name) != gaugeTypes.end()){
		DigitalGaugeType &gaugeType = gaugeTypes[type.name];
		m_lcdDisplay->SetNumberDigits(gaugeType.valueDigits + gaugeType.valuePrecision + (type.minValue < 0 ? 1 : 0));
		m_lcdDisplay->SetGrayColour(gaugeType.digitOffColor);
		m_lcdDisplay->SetLightColour(gaugeType.digitOnColor);
		m_valuePrecision = gaugeType.valuePrecision;
	}

	wxArrayString channelNames;
	channelNames.Add(channelName);
	store->ReadDatalog(m_channelData,datalogId,channelNames,0);

	SetOffset(0);
}


void DigitalGaugePane::SetOffset(int offset){
	m_dataOffset = offset;
	RefreshGaugeValue();
}

void DigitalGaugePane::RefreshGaugeValue(){

	DatastoreRow &row = m_channelData[m_dataOffset];
	double value = row.values[0];
	m_lcdDisplay->SetValue(wxString::Format(VALUE_FORMAT[m_valuePrecision],value));
}

void DigitalGaugePane::SetChartParams(ChartParams params){
	m_chartParams = params;
}

void DigitalGaugePane::UpdateValue(wxString &name, float value){

}

void DigitalGaugePane::InitComponents(){

	wxFlexGridSizer *sizer = new wxFlexGridSizer(1,1,0,0);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);

	m_lcdDisplay = new LCDDisplay(this);
	m_lcdDisplay->SetMinSize(wxSize(150,50));
	sizer->Add(m_lcdDisplay,1,wxEXPAND);
	this->SetSizer(sizer);
}


BEGIN_EVENT_TABLE ( DigitalGaugePane, wxPanel )
END_EVENT_TABLE()
