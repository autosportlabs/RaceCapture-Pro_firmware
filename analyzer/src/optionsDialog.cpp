
#include "optionsDialog.h"

#define NUMBER_COM_PORTS 20
#define LOAD_TYPE_SELECTION_MAP 	0
#define LOAD_TYPE_SELECTION_TPS 	1

IMPLEMENT_CLASS (OptionsDialog, wxDialog)

OptionsDialog::OptionsDialog(){}

OptionsDialog::OptionsDialog(wxWindow* parent,
  			wxWindowID id,
  			const wxString& caption,
  			const wxPoint& pos,
  			const wxSize& size,
  			long style){
	Create(parent,id,caption,pos,size,style);
}

AppOptions * OptionsDialog::GetAppOptions(){
	return _options;
}

void OptionsDialog::SetAppOptions(AppOptions *appOptions){
	_options = appOptions;
}

bool OptionsDialog::Create (wxWindow* parent,
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
 	return true;
}
  			
void OptionsDialog::CreateControls(){
 	
 	wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outerSizer);

	wxFlexGridSizer* optionsSizer = new wxFlexGridSizer(5,2,6,6);
	optionsSizer->AddGrowableCol(1);
	wxBoxSizer* okCancelSizer = new wxBoxSizer(wxHORIZONTAL);

	outerSizer->Add(optionsSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL,1);	
	outerSizer->Add(okCancelSizer,0,wxALIGN_CENTER_HORIZONTAL | wxALL,3);
 
 
 	wxString comPorts[NUMBER_COM_PORTS];
	for (int i = 0; i < NUMBER_COM_PORTS;i++){
		comPorts[i].Printf("COM %d",i+1);
	}
	wxComboBox *comPortComboBox = new wxComboBox(this, -1, "", wxDefaultPosition, wxDefaultSize, NUMBER_COM_PORTS, comPorts,wxCB_READONLY);
	COMPortValidator cpValidator(_options);
	comPortComboBox->SetValidator(cpValidator);
	
	optionsSizer->Add(new wxStaticText(this, -1, "COM Port"));
	optionsSizer->Add(comPortComboBox);
	
	wxCheckBox *autoLoadCheckBox= new wxCheckBox(this, -1, "Automatically read config at startup");
	autoLoadCheckBox->SetValidator(wxGenericValidator(&_options->GetAutoLoadConfig()));
	optionsSizer->AddStretchSpacer();
	optionsSizer->Add(autoLoadCheckBox);
}
 
BEGIN_EVENT_TABLE( OptionsDialog, wxDialog)
END_EVENT_TABLE()


COMPortValidator::COMPortValidator() : _appOptions(NULL)
{}

COMPortValidator::COMPortValidator(AppOptions *appOptions){
	_appOptions = appOptions;	
}

void COMPortValidator::SetAppOptions(AppOptions *appOptions){
	_appOptions = appOptions;	
}

bool COMPortValidator::TransferFromWindow(){

	wxComboBox* combo = (wxComboBox*)GetWindow();
	int selectedIndex = combo->GetSelection();
	_appOptions->SetSerialPort(selectedIndex);
	return true;
}

bool COMPortValidator::TransferToWindow(){

	int selectedIndex = _appOptions->GetSerialPort();
	wxComboBox* combo = (wxComboBox*)GetWindow();
	combo->Select(selectedIndex);	
	return true;
}

wxObject* COMPortValidator::Clone() const{
	return new COMPortValidator(_appOptions);	
}

bool COMPortValidator::Validate(wxWindow* parent){
	return true;	
}
