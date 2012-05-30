#include "configuration/configPanel.h"
#include "configuration/gpsConfigPanel.h"
#include "BitmapWindow.h"

#include "finishLine.xpm"

GpsConfigPanel::GpsConfigPanel() : BaseChannelConfigPanel()
{
}

GpsConfigPanel::GpsConfigPanel(wxWindow *parent,
			wxWindowID id,
			RaceCaptureConfig *config,
			const wxPoint &pos,
			const wxSize &size,
			long style,
			const wxString &name
			)
			: BaseChannelConfigPanel(	parent,
						id,
						config,
						pos,
						size,
						style,
						name)
{

}

GpsConfigPanel::~GpsConfigPanel(){
}

void GpsConfigPanel::UpdatedExtendedFields(){
	GpsConfig &gpsConfig = m_raceCaptureConfig->gpsConfig;

	m_gpsInstalledCheckBox->SetValue(gpsConfig.gpsInstalled);
	m_startFinishLatitudeTextCtrl->SetValue(wxString::Format("%f",gpsConfig.startFinishLatitude));
	m_startFinishLongitudeTextCtrl->SetValue(wxString::Format("%f",gpsConfig.startFinishLongitude));
	m_startFinishTargetRadius->SetValue(wxString::Format("%f",gpsConfig.startFinishRadius));
}

void GpsConfigPanel::UpdateExtendedChannelFields(int i){
}

ChannelConfig * GpsConfigPanel::GetChannelConfig(int i){
	GpsConfig &cfg = m_raceCaptureConfig->gpsConfig;
	switch(i){
		case 0:
			return &(cfg.latitudeCfg);
		case 1:
			return &(cfg.longitudeCfg);
		case 2:
			return &(cfg.velocityCfg);
		case 3:
			return &(cfg.timeCfg);
		case 4:
			return &(cfg.qualityCfg);
		case 5:
			return &(cfg.satellitesCfg);
		case 6:
			return &(cfg.lapCountCfg);
		case 7:
			return &(cfg.lapTimeCfg);
		default:
			return NULL;
	}
}

int GpsConfigPanel::ChannelCount(){
	return CONFIG_GPS_CHANNELS;
}

ChannelConfigExtraFields GpsConfigPanel::CreateExtendedChannelFields(int i){
	ChannelConfigExtraFields extraFields;
	return extraFields;
}

wxString GpsConfigPanel::GetChannelLabel(int index){
	switch(index){
		case 0:
			return "Latitude";
		case 1:
			return "Longitude";
		case 2:
			return "Velocity";
		case 3:
			return "Time";
		case 4:
			return "GPS Quality";
		case 5:
			return "GPS Satellites";
		case 6:
			return "LapCount";
		case 7:
			return "LapTime";
		default:
			return "";
	}
}

wxString GpsConfigPanel::GetChannelListTitle(){
	return "GPS Channels";
}

wxString GpsConfigPanel::GetChannelConfigPanelName(int index){
	return wxString::Format("gps_channel_%d",index);
}

wxPanel * GpsConfigPanel::GetTopInnerPanel(){

	m_gpsOptionsPanel = new wxPanel(this);

	wxFlexGridSizer *sizer =new wxFlexGridSizer(2,1,8,8);
	sizer->AddGrowableRow(0);
	sizer->AddGrowableRow(1);
	sizer->AddGrowableCol(0);

	m_gpsInstalledCheckBox = new wxCheckBox(m_gpsOptionsPanel,wxID_ANY,"GPS Installed");
	m_gpsInstalledCheckBox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GpsConfigPanel::OnGpsInstalledChanged),NULL,this);
	sizer->Add(m_gpsInstalledCheckBox,1,wxEXPAND | wxALIGN_CENTER_VERTICAL);

	wxStaticBoxSizer *startFinishSizer = new wxStaticBoxSizer(new wxStaticBox(m_gpsOptionsPanel,-1,"Start/Finish Line Target"),wxHORIZONTAL);
	wxFlexGridSizer *innerStartFinishSizer = new wxFlexGridSizer(3,3,8,8);
	innerStartFinishSizer->AddGrowableCol(2);

	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,wxID_ANY,"Target Latitude"),1,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
	m_startFinishLatitudeTextCtrl = new wxTextCtrl(m_gpsOptionsPanel,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,wxTextValidator(wxFILTER_NUMERIC));
	m_startFinishLatitudeTextCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(GpsConfigPanel::OnStartFinishLatitudeChanged),NULL,this);	innerStartFinishSizer->Add(m_startFinishLatitudeTextCtrl,1);
	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,-1,"Degrees"),1,wxALIGN_CENTER_VERTICAL);

	m_startFinishLongitudeTextCtrl =new wxTextCtrl(m_gpsOptionsPanel,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,wxTextValidator(wxFILTER_NUMERIC));
	m_startFinishLongitudeTextCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(GpsConfigPanel::OnStartFinishLongitudeChanged),NULL,this);
	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,wxID_ANY,"Target Longitude"),1,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
	innerStartFinishSizer->Add(m_startFinishLongitudeTextCtrl,1);
	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,wxID_ANY,"Degrees"),1,wxALIGN_CENTER_VERTICAL);

	m_startFinishTargetRadius = new wxTextCtrl(m_gpsOptionsPanel,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,wxTextValidator(wxFILTER_NUMERIC));
	m_startFinishTargetRadius->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(GpsConfigPanel::OnStartFinishRadiusChanged),NULL,this);
	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,wxID_ANY,"Target Radius"),1,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
	innerStartFinishSizer->Add(m_startFinishTargetRadius,1);
	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,wxID_ANY,"Degrees"),1,wxALIGN_CENTER_VERTICAL);

	startFinishSizer->Add(innerStartFinishSizer);
	startFinishSizer->Add(new BitmapWindow(m_gpsOptionsPanel,wxID_ANY,finishLine_xpm),1, wxALIGN_CENTER_HORIZONTAL);
	sizer->Add(startFinishSizer,1,wxEXPAND);
	m_gpsOptionsPanel->SetSizer(sizer);

	return m_gpsOptionsPanel;
}

wxPanel * GpsConfigPanel::GetBottomInnerPanel(){
	return NULL;
}

void GpsConfigPanel::OnGpsInstalledChanged(wxCommandEvent &event){
	wxCheckBox *c = dynamic_cast<wxCheckBox*>(event.GetEventObject());
	if (NULL != c) m_raceCaptureConfig->gpsConfig.gpsInstalled = c->GetValue();
}

void GpsConfigPanel::OnStartFinishLatitudeChanged(wxCommandEvent &event){
	wxTextCtrl *c = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != c) m_raceCaptureConfig->gpsConfig.startFinishLatitude = atof(c->GetValue());
}

void GpsConfigPanel::OnStartFinishLongitudeChanged(wxCommandEvent &event){
	wxTextCtrl *c = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != c) m_raceCaptureConfig->gpsConfig.startFinishLongitude = atof(c->GetValue());

}

void GpsConfigPanel::OnStartFinishRadiusChanged(wxCommandEvent &event){
	wxTextCtrl *c = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != c) m_raceCaptureConfig->gpsConfig.startFinishRadius = atof(c->GetValue());
}

BEGIN_EVENT_TABLE ( GpsConfigPanel, wxPanel )
END_EVENT_TABLE()
