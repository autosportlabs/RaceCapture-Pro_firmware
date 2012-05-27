#include "configuration/configPanel.h"
#include "configuration/loggerOutputConfigPanel.h"


LoggerOutputConfigPanel::LoggerOutputConfigPanel() : BaseConfigPanel()
{
}

LoggerOutputConfigPanel::LoggerOutputConfigPanel(wxWindow *parent,
			wxWindowID id,
			RaceCaptureConfig *config,
			const wxPoint &pos,
			const wxSize &size,
			long style,
			const wxString &name
			)
			: BaseConfigPanel(	parent,
						id,
						config,
						pos,
						size,
						style,
						name)
{
}

LoggerOutputConfigPanel::~LoggerOutputConfigPanel(){

}

void LoggerOutputConfigPanel::OnConfigUpdated(){
	LoggerOutputConfig &cfg = (m_raceCaptureConfig->loggerOutputConfig);
	m_sdLoggingModeCombo->Select(cfg.loggingMode);
	m_telemetryModeCombo->Select(cfg.telemetryMode);
	m_p2pAddressHighTextCtrl->SetValue(wxString::Format("%u",cfg.p2pDestinationAddrHigh));
	m_p2pAddressLowTextCtrl->SetValue(wxString::Format("%u",cfg.p2pDestinationAddrLow));
}


void LoggerOutputConfigPanel::InitComponents(){

 	wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outerSizer);

	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		wxStaticBoxSizer *loggingModeSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"SD Logging Mode"),wxVERTICAL);

		loggingModeSizer->Add(new wxStaticText(this,wxID_ANY,"Specify the logging mode used for logging to the SD Memory card.\n\nSelect CSV for easy import to analysis programs & spreadsheets;\nSelect Binary format for maximum performance"));
		loggingModeSizer->AddSpacer(10);

		wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(1,2,6,6);
		optionsSizer->Add(new wxStaticText(this,wxID_ANY,"Logging Mode"),0,wxALIGN_LEFT);

		m_sdLoggingModeCombo = new wxComboBox(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize);
		m_sdLoggingModeCombo->Append("SD Logging Disabled");
		m_sdLoggingModeCombo->Append("Comma Separated Values (CSV)");
		m_sdLoggingModeCombo->Append("Binary (not yet supported)");
		m_sdLoggingModeCombo->Select(0);
		optionsSizer->Add(m_sdLoggingModeCombo,0,wxALIGN_LEFT);
		m_sdLoggingModeCombo->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(LoggerOutputConfigPanel::OnLoggingModeChanged),NULL,this);

		loggingModeSizer->Add(optionsSizer,0,wxEXPAND);
		outerSizer->Add(loggingModeSizer, 1, wxEXPAND | wxALL,1);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		wxStaticBoxSizer *telemetryModeSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Telemetry Mode"),wxVERTICAL);

		telemetryModeSizer->Add(new wxStaticText(this,wxID_ANY,"Specify the telemetry module attached to RaceCapture/Pro"));
		telemetryModeSizer->AddSpacer(10);

		wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(1,2,6,6);
		optionsSizer->Add(new wxStaticText(this,wxID_ANY,"Telemetry Mode"),1,wxALIGN_LEFT);

		m_telemetryModeCombo = new wxComboBox(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize);
		m_telemetryModeCombo->Append("Telemetry Disabled");
		m_telemetryModeCombo->Append("Peer to Peer Mesh Network");
		m_telemetryModeCombo->Append("Cellular Module");
		m_telemetryModeCombo->Select(0);
		optionsSizer->Add(m_telemetryModeCombo,0,wxALIGN_LEFT);
		m_telemetryModeCombo->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(LoggerOutputConfigPanel::OnTelemetryModeChanged),NULL,this);
		telemetryModeSizer->Add(optionsSizer,0);
		telemetryModeSizer->AddSpacer(20);

		wxStaticBoxSizer *p2pAddrSizer = new wxStaticBoxSizer(new wxStaticBox(this,-1,"Peer to Peer Base Station Address"),wxVERTICAL);

		p2pAddrSizer->Add(new wxStaticText(this,wxID_ANY,"Specify the 2-part address of the Base Station Node"));
		p2pAddrSizer->AddSpacer(10);

		wxFlexGridSizer* p2pOptionsSizer = new wxFlexGridSizer(2,3,6,6);

		p2pOptionsSizer->AddStretchSpacer(1);
		p2pOptionsSizer->Add(new wxStaticText(this,wxID_ANY,"High"),1,wxALIGN_LEFT);
		p2pOptionsSizer->Add(new wxStaticText(this,wxID_ANY,"Low"),1,wxALIGN_LEFT);
		p2pOptionsSizer->Add(new wxStaticText(this,wxID_ANY,"Base Station Address (Decimal)"),1,wxALIGN_LEFT);

		m_p2pAddressHighTextCtrl =new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,wxTextValidator(wxFILTER_NUMERIC));
		m_p2pAddressHighTextCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(LoggerOutputConfigPanel::OnP2PAddressHighChanged),NULL,this);
		p2pOptionsSizer->Add(m_p2pAddressHighTextCtrl);

		m_p2pAddressLowTextCtrl =new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,wxTextValidator(wxFILTER_NUMERIC));
		m_p2pAddressLowTextCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(LoggerOutputConfigPanel::OnP2PAddressLowChanged),NULL,this);
		p2pOptionsSizer->Add(m_p2pAddressLowTextCtrl);

		p2pAddrSizer->Add(p2pOptionsSizer,1,wxEXPAND);
		telemetryModeSizer->Add(p2pAddrSizer,1,wxEXPAND);
		outerSizer->Add(telemetryModeSizer,1,wxEXPAND | wxALL,1);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
}

void LoggerOutputConfigPanel::OnP2PAddressHighChanged(wxCommandEvent &event){
	wxTextCtrl *c = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != c) {
		LoggerOutputConfig &cfg = (m_raceCaptureConfig->loggerOutputConfig);
		cfg.p2pDestinationAddrHigh = atoi(c->GetValue());
	}
}

void LoggerOutputConfigPanel::OnP2PAddressLowChanged(wxCommandEvent &event){
	wxTextCtrl *c = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (NULL != c) {
		LoggerOutputConfig &cfg = (m_raceCaptureConfig->loggerOutputConfig);
		cfg.p2pDestinationAddrLow = atoi(c->GetValue());
	}
}

void LoggerOutputConfigPanel::OnTelemetryModeChanged(wxCommandEvent &event){
	wxComboBox *c = dynamic_cast<wxComboBox*>(event.GetEventObject());
	if (NULL != c) {
		LoggerOutputConfig &cfg = (m_raceCaptureConfig->loggerOutputConfig);
		cfg.telemetryMode = (telemetry_mode_t)c->GetSelection();
	}
}

void LoggerOutputConfigPanel::OnLoggingModeChanged(wxCommandEvent &event){
	wxComboBox *c = dynamic_cast<wxComboBox*>(event.GetEventObject());
	if (NULL != c) {
		LoggerOutputConfig &cfg = (m_raceCaptureConfig->loggerOutputConfig);
		cfg.loggingMode = (logging_mode_t)c->GetSelection();
	}
}

BEGIN_EVENT_TABLE ( LoggerOutputConfigPanel, wxPanel )
END_EVENT_TABLE()
