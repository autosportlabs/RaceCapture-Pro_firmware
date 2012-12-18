#include "configuration/analogInputAdvancedOptions.h"
#include "raceCapture/raceCaptureConfig.h"
#include <float.h>

IMPLEMENT_CLASS (AnalogInputAdvancedDialog, wxDialog)

AnalogInputAdvancedDialog::AnalogInputAdvancedDialog()  {}

AnalogInputAdvancedDialog::AnalogInputAdvancedDialog(wxWindow* parent,
  			wxWindowID id,
  			const wxString& caption,
  			const wxPoint& pos,
  			const wxSize& size,
  			long style) {
	Create(parent,id,caption,pos,size,style);
}

bool AnalogInputAdvancedDialog::Create (wxWindow* parent,
  			wxWindowID id,
  			const wxString& caption,
  			const wxPoint& pos,
  			const wxSize& size,
  			long style){

 	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

 	if (! wxDialog::Create (parent, id, caption, pos, size, style)) return false;

 	CreateControls();

 	GetSizer()->Fit(this);
 	GetSizer()->SetSizeHints(this);
 	Center();

 	RefreshControls();
 	return true;
}

void AnalogInputAdvancedDialog::RefreshControls(){
	{
		long int selectedId = 0;
		switch(m_config.scalingMode){
			case scaling_mode_raw:
				selectedId = ANALOG_SCALING_RAW_RADIO;
				break;
			case scaling_mode_linear:
				selectedId = ANALOG_SCALING_LINEAR_RADIO;
				break;
			case scaling_mode_map:
				selectedId = ANALOG_SCALING_MAP_RADIO;
				break;
		}

		if (selectedId != 0){
			wxRadioButton * radio = dynamic_cast<wxRadioButton*>(FindWindowById(selectedId));
			if (NULL != radio) radio->SetValue(true);
		}
	}
	{
		wxTextCtrl *linearScaling = dynamic_cast<wxTextCtrl*>(FindWindowById(SIMPLE_LINEAR_SCALE_TEXTBOX));
		if (NULL != linearScaling) linearScaling->SetValue(wxString::Format("%f",m_config.linearScaling));
	}
	{
		ScalingMap &scalingMap = m_config.scalingMap;
		for (int i = 0; i < CONFIG_ANALOG_SCALING_BINS; i++){
			m_analogScalingMapGrid->SetCellValue(wxString::Format("%d",scalingMap.rawValues[i]),0,i);
		}
		for (int i = 0; i < CONFIG_ANALOG_SCALING_BINS; i++){
			m_analogScalingMapGrid->SetCellValue(wxString::Format("%f",scalingMap.scaledValue[i]),1,i);
		}
	}
	RefreshScalingChart();
}

void AnalogInputAdvancedDialog::CreateControls(){

 	wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outerSizer);

	wxStaticBoxSizer *analogScalingOptionsSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Analog Scaling Options"),wxVERTICAL);

	analogScalingOptionsSizer->Add(new wxStaticText(this,-1,"Specify the scaling configuration for this Analog Input Channel"));
	analogScalingOptionsSizer->AddSpacer(10);

	wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(3,3,6,6);
	optionsSizer->Add(new wxRadioButton(this,ANALOG_SCALING_RAW_RADIO,"Raw Value (0 - 1023)",wxDefaultPosition,wxDefaultSize,wxRB_GROUP));
	optionsSizer->AddStretchSpacer(1);
	optionsSizer->AddStretchSpacer(1);

	optionsSizer->Add(new wxRadioButton(this,ANALOG_SCALING_LINEAR_RADIO,"Simple Linear Scaling"));
	optionsSizer->Add(new wxTextCtrl(this,SIMPLE_LINEAR_SCALE_TEXTBOX,"",wxDefaultPosition,wxDefaultSize,0,wxTextValidator(wxFILTER_NUMERIC)));

	optionsSizer->Add(new wxStaticText(this, -1,"(Multiplier)"),0,wxALIGN_LEFT);

	optionsSizer->Add(new wxRadioButton(this,ANALOG_SCALING_MAP_RADIO,"Mapped Value"));
	optionsSizer->AddStretchSpacer(1);
	optionsSizer->AddStretchSpacer(1);

	analogScalingOptionsSizer->Add(optionsSizer,1,wxEXPAND);

	outerSizer->Add(analogScalingOptionsSizer, 1, wxEXPAND | wxALL,1);

	wxStaticBoxSizer *analogScalingMapSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Analog Scaling Map"),wxVERTICAL);

	analogScalingMapSizer->Add(GetAnalogScalingMapGrid(),0 , wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALL,5);
	analogScalingMapSizer->Add(GetAnalogScalingChart(),0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL| wxALL,5);

	outerSizer->Add(analogScalingMapSizer,0 , wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALL);

 	wxBoxSizer* okCancelSizer = new wxBoxSizer(wxHORIZONTAL);

 	wxButton* ok = new wxButton(this, wxID_OK, wxT("&OK"), wxDefaultPosition, wxDefaultSize,0);
	okCancelSizer->Add(ok, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

	wxButton* cancel = new wxButton(this, wxID_CANCEL, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize,0);
	okCancelSizer->Add(cancel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

	outerSizer->Add(okCancelSizer,0, wxALIGN_CENTER_HORIZONTAL | wxALL,3);
}

void AnalogInputAdvancedDialog::RefreshScalingChart(){
	m_analogScalingChart->ClearLog();

	ScalingMap &scalingMap = m_config.scalingMap;
	double maxValue = DBL_MIN;
	double minValue = DBL_MAX;
	for (int i = 0; i < CONFIG_ANALOG_SCALING_BINS; i++){
		double val =scalingMap.scaledValue[i];
		if (val > maxValue) maxValue = val;
		if (val < minValue) minValue = val;
	}
	m_chartScale->minValue = minValue;
	m_chartScale->maxValue = maxValue;

	for (int i = MIN_ANALOG_RAW;i < MAX_ANALOG_RAW; i++){
		StripChartLogItem *logItem = new StripChartLogItem();
		double value = GetScaledValue(i);
		(*logItem)["scaled"] = value;
		m_analogScalingChart->LogData(logItem);
	}
}

double AnalogInputAdvancedDialog::GetScaledValue(int rawValue){
	unsigned short *bins;
	unsigned short bin, nextBin;

	bins = m_config.scalingMap.rawValues + CONFIG_ANALOG_SCALING_BINS - 1;
	bin = CONFIG_ANALOG_SCALING_BINS - 1;

	while (rawValue < *bins && bin > 0){
		bins--;
		bin--;
	}
	if (bin == 0 && rawValue < *bins){
		nextBin = 0;
	}
	else{
		nextBin = bin;
		if (bin < CONFIG_ANALOG_SCALING_BINS - 1) nextBin++;
	}
	double x = (double)rawValue;
	double x1 = (double)m_config.scalingMap.rawValues[bin];
	double y1 = (double)m_config.scalingMap.scaledValue[bin];
	double x2 = (double)m_config.scalingMap.rawValues[nextBin];
	double y2 = (double)m_config.scalingMap.scaledValue[nextBin];
	double scaled = LinearInterpolate(x,x1,y1,x2,y2);
	return scaled;
}

AnalogScalingMapGrid * AnalogInputAdvancedDialog::GetAnalogScalingMapGrid(){

	m_analogScalingMapGrid = new AnalogScalingMapGrid(this, ANALOG_SCALING_MAP_GRID, wxDefaultPosition, wxSize(492,70));
	m_analogScalingMapGrid->CreateGrid(2,CONFIG_ANALOG_SCALING_BINS);
	for (int i=0;i<CONFIG_ANALOG_SCALING_BINS;i++){
		m_analogScalingMapGrid->SetColSize(i,80);
		m_analogScalingMapGrid->SetCellEditor(0,i,new wxGridCellNumberEditor(MIN_ANALOG_RAW,MAX_ANALOG_RAW));
		m_analogScalingMapGrid->SetCellEditor(1,i,new wxGridCellFloatEditor(-1,4));
		m_analogScalingMapGrid->SetColLabelValue(i,wxString::Format("%d",i+1));
	}
	m_analogScalingMapGrid->SetRowLabelValue(0,"Raw");
	m_analogScalingMapGrid->SetRowLabelValue(1,"Scaled");

	m_analogScalingMapGrid->EnableDragColSize(false);
	m_analogScalingMapGrid->EnableDragRowSize(false);
	m_analogScalingMapGrid->SetSelectionMode(wxGrid::wxGridSelectCells);
	return m_analogScalingMapGrid;
}

StripChart * AnalogInputAdvancedDialog::GetAnalogScalingChart(){
	m_analogScalingChart = new StripChart(this,ANALOG_SCALING_MAP_CHART);
	m_analogScalingChart->SetMinSize(wxSize(492,200));
	m_analogScalingChart->SetBackgroundColour(*wxBLACK);
	m_analogScalingChart->SetLogBufferSize(MAX_ANALOG_RAW + 1);
	m_chartScale = new ChartScale(MIN_ANALOG_RAW,MAX_ANALOG_RAW,10, ChartScale::ORIENTATION_LEFT,"Scaled Value");
	m_chartScaleId = m_analogScalingChart->AddScale(m_chartScale);
	m_analogScalingChart->AddLogItemType(new LogItemType(m_chartScaleId, *wxGREEN, "Scaled Value", "scaled"));
	m_analogScalingChart->SetZoom(48);
	m_analogScalingChart->ShowData(false);
	m_analogScalingChart->ShowScale(false);

	return m_analogScalingChart;
}

void AnalogInputAdvancedDialog::SetConfig(AnalogConfig *config){
	if (NULL != config) m_config = *config;
}

AnalogConfig AnalogInputAdvancedDialog::GetConfig(){
	return m_config;
}

//linear interpolation routine
//            (y2 - y1)
//  y = y1 +  --------- * (x - x1)
//            (x2 - x1)
double AnalogInputAdvancedDialog::LinearInterpolate(double x, double x1, double y1, double x2, double y2){
	return y1 + (((y2 - y1))  / (x2 - x1)) * (x - x1);
}

void AnalogInputAdvancedDialog::OnScalingMapGridChanged(wxGridEvent &event){
	ScalingMap &scalingMap = m_config.scalingMap;
	for (int i = 0; i < CONFIG_ANALOG_SCALING_BINS;i++){
		scalingMap.rawValues[i] = atoi(m_analogScalingMapGrid->GetCellValue(0,i));
		scalingMap.scaledValue[i] = atof(m_analogScalingMapGrid->GetCellValue(1,i));
	}
	RefreshScalingChart();
}

void AnalogInputAdvancedDialog::OnLinearScalingButton(wxCommandEvent &event){
	m_config.scalingMode = scaling_mode_linear;
}

void AnalogInputAdvancedDialog::OnRawScalingButton(wxCommandEvent &event){
	m_config.scalingMode = scaling_mode_raw;
}

void AnalogInputAdvancedDialog::OnMapScalingButton(wxCommandEvent &event){
	m_config.scalingMode = scaling_mode_map;
}

void AnalogInputAdvancedDialog::OnLinearScaleChanged(wxCommandEvent &event){
	wxTextCtrl *w = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != w) m_config.linearScaling = atof(w->GetValue());
}

BEGIN_EVENT_TABLE( AnalogInputAdvancedDialog, wxDialog)
	EVT_TEXT(SIMPLE_LINEAR_SCALE_TEXTBOX,AnalogInputAdvancedDialog::OnLinearScaleChanged)
	EVT_RADIOBUTTON(ANALOG_SCALING_LINEAR_RADIO,AnalogInputAdvancedDialog::OnLinearScalingButton)
	EVT_RADIOBUTTON(ANALOG_SCALING_RAW_RADIO,AnalogInputAdvancedDialog::OnRawScalingButton)
	EVT_RADIOBUTTON(ANALOG_SCALING_MAP_RADIO,AnalogInputAdvancedDialog::OnMapScalingButton)
	EVT_GRID_CELL_CHANGE(AnalogInputAdvancedDialog::OnScalingMapGridChanged)
END_EVENT_TABLE()

