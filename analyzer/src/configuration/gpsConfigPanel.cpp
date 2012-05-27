#include "configuration/configPanel.h"
#include "configuration/gpsConfigPanel.h"
#include "BitmapWindow.h"

#include "finishLine.xpm"

const wxString GpsConfigPanel::GPS_INSTALLED_CHECKBOX_NAME = "gpsInstalled";
const wxString GpsConfigPanel::GPS_STARTFINISH_LATITUDE_NAME = "gpsStartFinishLatitude";
const wxString GpsConfigPanel::GPS_STARTFINISH_LONGITUDE_NAME = "gpsStartFinishLongitude";
const wxString GpsConfigPanel::GPS_STARTFINISH_TARGET_RADIUS_NAME  = "gpsStartFinish";

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
	SetCheckBoxValue(GPS_INSTALLED_CHECKBOX_NAME,gpsConfig.gpsInstalled == 1);
	SetTextCtrlValue(GPS_STARTFINISH_LATITUDE_NAME,gpsConfig.startFinishLatitude);
	SetTextCtrlValue(GPS_STARTFINISH_LONGITUDE_NAME,gpsConfig.startFinishLongitude);
	SetTextCtrlValue(GPS_STARTFINISH_TARGET_RADIUS_NAME,gpsConfig.startFinishRadius);
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

	wxCheckBox *installedCheckBox = new wxCheckBox(m_gpsOptionsPanel,-1,"GPS Installed");
	installedCheckBox->SetName(GPS_INSTALLED_CHECKBOX_NAME);

	sizer->Add(installedCheckBox,1,wxEXPAND | wxALIGN_CENTER_VERTICAL);

	wxStaticBoxSizer *startFinishSizer = new wxStaticBoxSizer(new wxStaticBox(m_gpsOptionsPanel,-1,"Start/Finish Line Target"),wxHORIZONTAL);

	wxFlexGridSizer *innerStartFinishSizer = new wxFlexGridSizer(3,3,8,8);
	innerStartFinishSizer->AddGrowableCol(2);

	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,-1,"Target Latitude"),1,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
	innerStartFinishSizer->Add(new wxTextCtrl(m_gpsOptionsPanel,-1,"",wxDefaultPosition,wxDefaultSize,0,wxDefaultValidator,GPS_STARTFINISH_LATITUDE_NAME),1);
	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,-1,"Degrees"),1,wxALIGN_CENTER_VERTICAL);

	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,-1,"Target Longitude"),1,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
	innerStartFinishSizer->Add(new wxTextCtrl(m_gpsOptionsPanel,-1,"",wxDefaultPosition,wxDefaultSize,0,wxDefaultValidator,GPS_STARTFINISH_LONGITUDE_NAME),1);
	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,-1,"Degrees"),1,wxALIGN_CENTER_VERTICAL);

	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,-1,"Target Radius"),1,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
	innerStartFinishSizer->Add(new wxTextCtrl(m_gpsOptionsPanel,-1,"",wxDefaultPosition,wxDefaultSize,0,wxDefaultValidator,GPS_STARTFINISH_TARGET_RADIUS_NAME),1);
	innerStartFinishSizer->Add(new wxStaticText(m_gpsOptionsPanel,-1,"Degrees"),1,wxALIGN_CENTER_VERTICAL);

	startFinishSizer->Add(innerStartFinishSizer);
	startFinishSizer->Add(new BitmapWindow(m_gpsOptionsPanel, -1,finishLine_xpm),1, wxALIGN_CENTER_HORIZONTAL);
	sizer->Add(startFinishSizer,1,wxEXPAND);
	m_gpsOptionsPanel->SetSizer(sizer);

	return m_gpsOptionsPanel;
}

wxPanel * GpsConfigPanel::GetBottomInnerPanel(){
	return NULL;
}

BEGIN_EVENT_TABLE ( GpsConfigPanel, wxPanel )
END_EVENT_TABLE()
