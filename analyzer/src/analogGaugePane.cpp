/*
 * analogGaugePane.cpp
 *
 *  Created on: 2009-07-20
 *      Author: brent
 */

#include "analogGaugePane.h"
AnalogGaugePane::AnalogGaugePane() : wxPanel()
{
	InitComponents();
}

AnalogGaugePane::AnalogGaugePane(wxWindow *parent,
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

AnalogGaugePane::~AnalogGaugePane(){

}

void AnalogGaugePane::CreateGauge(int datalogId, wxString channelName){

	DatalogStore *store = m_chartParams.datalogStore;

	DatalogChannel channel;
	store->GetChannel(datalogId,channelName, channel);

	DatalogChannelTypes channelTypes;
	store->GetChannelTypes(channelTypes);

	DatalogChannelType &type = channelTypes[channel.typeId];
	m_angularMeter->SetRange(type.minValue, type.maxValue);
	AppOptions *options = m_chartParams.appOptions;

	AnalogGaugeTypes &gaugeTypes = options->GetAnalogGaugeTypes();

	if (gaugeTypes.find(type.name) != gaugeTypes.end()){
		AnalogGaugeType &gaugeType = gaugeTypes[type.name];
		m_angularMeter->AddTicks(gaugeType.majorTickInterval, gaugeType.minorTickInterval);
		m_angularMeter->SetLabelFont(gaugeType.labelFont);
		m_angularMeter->SetMajorTickFont(gaugeType.majorTickFont);
		m_angularMeter->SetValueFont(gaugeType.valueFont);
		m_angularMeter->SetValuePrecision(gaugeType.valuePrecision);
		m_angularMeter->SetLabel(type.unitsLabel);
	}

	wxArrayString channelNames;
	channelNames.Add(channelName);
	store->ReadDatalog(m_channelData,datalogId,channelNames,0);

	SetOffset(0);
}


void AnalogGaugePane::SetOffset(int offset){
	m_dataOffset = offset;
	RefreshGaugeValue();
}

void AnalogGaugePane::RefreshGaugeValue(){

	DatastoreRow &row = m_channelData[m_dataOffset];
	double value = row.values[0];
	m_angularMeter->SetValue(value);
}

void AnalogGaugePane::SetChartParams(ChartParams params){
	m_chartParams = params;
}

void AnalogGaugePane::UpdateValue(wxString &channelName, float value){

}

void AnalogGaugePane::InitComponents(){

	wxFlexGridSizer *sizer = new wxFlexGridSizer(1,1,0,0);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);
	m_angularMeter = new AngularMeter(this,-1);
	m_angularMeter->SetMinSize(wxSize(150,150));
	sizer->Add(m_angularMeter,1,wxEXPAND);
	this->SetSizer(sizer);
}


BEGIN_EVENT_TABLE ( AnalogGaugePane, wxPanel )
END_EVENT_TABLE()
