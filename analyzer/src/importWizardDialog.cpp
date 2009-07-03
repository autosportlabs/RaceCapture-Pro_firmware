/*
 * importWizardDialog.cpp
 *
 *  Created on: Jun 22, 2009
 *      Author: brent
 */
#import "importWizardDialog.h"
#import "wx/file.h"

#define LOGGING_FILE_FILTER	"RaceAnalyzer Datalog File (*.log)|*.log|Generic CSV File (*.csv)|*.csv"

#define ERROR_BACKGROUND_COLOR wxColor(0xff,0xe0,0xe0)

DatalogFileSelectPage::DatalogFileSelectPage(wxWizard *parent, ImportWizardParams *params) :
	wxWizardPageSimple(parent), m_params(params){

	wxFlexGridSizer *pathSizer = new wxFlexGridSizer(3,3,3);
	pathSizer->AddGrowableCol(1);
	pathSizer->Add(new wxStaticText(this,-1,"File Location"),1,wxALIGN_CENTER_VERTICAL);
	m_filePath = new wxTextCtrl(this, ID_IMPORT_WIZ_PATH_TEXT_CTRL, m_params->datalogFilePath);
	m_defaultColor = m_filePath->GetBackgroundColour();

	pathSizer->Add(m_filePath,1,wxEXPAND);
	pathSizer->Add(new wxButton(this,ID_IMPORT_WIZ_BROWSE_BUTTON,"..."),1);

	pathSizer->Add(30,30,1);
	pathSizer->Add(400,30,1);
	pathSizer->Add(30,30,1);

	pathSizer->AddSpacer(10);
	m_helpText = new wxStaticText(this,ID_IMPORT_WIZ_DATALOG_FILE_HELP_TEXT,"");
	wxFont f= m_helpText->GetFont();
	f.SetStyle(wxFONTSTYLE_ITALIC);
	f.SetNoAntiAliasing(false);
	m_helpText->SetFont(f);
	pathSizer->Add(m_helpText,1,wxEXPAND);
	pathSizer->AddSpacer(10);

	pathSizer->Add(30,30,1);
	pathSizer->Add(400,30,1);
	pathSizer->Add(30,30,1);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add( pathSizer, 1, wxALL | wxEXPAND, 1 );

    SetSizer(mainSizer);
    mainSizer->Fit(this);
    UpdateUIState();
}

void DatalogFileSelectPage::OnBrowse(wxCommandEvent &evt){

	ImportWizardParams *params = m_params;
	AppPrefs *prefs = params->appPrefs;
	wxFileDialog fileDialog(this, "Import Datalog", prefs->GetLastConfigFileDirectory(), "", LOGGING_FILE_FILTER, wxOPEN);

	int result = fileDialog.ShowModal();
	if (wxID_OK != result) return;
	prefs->SetLastDatalogDirectory(fileDialog.GetDirectory());

	wxString fileName = fileDialog.GetPath();
	SetDatalogFilePath(fileName);

	UpdateUIState();
}

bool DatalogFileSelectPage::IsFileValid(wxString filePath){
	return wxFile::Exists(filePath.ToAscii());
}

void DatalogFileSelectPage::UpdateUIState(){

	wxWindow *nextButton = GetParent()->FindWindow(wxID_FORWARD);
	if (IsFileValid(GetDatalogFilePath())){
		nextButton->Enable();
		m_helpText->SetLabel("Press Next to continue...");
		m_filePath->SetBackgroundColour(m_defaultColor);
	}
	else{
		nextButton->Disable();
		m_helpText->SetLabel("Select a Log File to import");
		m_filePath->SetBackgroundColour(ERROR_BACKGROUND_COLOR);
	}
}


wxString DatalogFileSelectPage::GetDatalogFilePath(){
	return m_filePath->GetValue();
}

void DatalogFileSelectPage::SetDatalogFilePath(wxString filePath){
	m_filePath->SetValue(filePath);
}

void DatalogFileSelectPage::OnDatalogPathTextChange(wxCommandEvent &event){
	UpdateUIState();
}

void DatalogFileSelectPage::OnWizardPageChanging(wxWizardEvent& event)
{
	if (event.GetDirection() &&  ! wxFile::Exists(GetDatalogFilePath().ToAscii())){
		event.Veto();
	}
	m_params->datalogFilePath = m_filePath->GetValue();
}

BEGIN_EVENT_TABLE ( DatalogFileSelectPage, wxWizardPageSimple )

	EVT_BUTTON(ID_IMPORT_WIZ_BROWSE_BUTTON, DatalogFileSelectPage::OnBrowse)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, DatalogFileSelectPage::OnWizardPageChanging)
	EVT_TEXT(ID_IMPORT_WIZ_PATH_TEXT_CTRL,DatalogFileSelectPage::OnDatalogPathTextChange)

END_EVENT_TABLE()




DatalogInfoPage::DatalogInfoPage(wxWizard *parent, ImportWizardParams *params) :
	wxWizardPageSimple(parent),m_params(params){

	wxFlexGridSizer *innerSizer = new wxFlexGridSizer(2,2,3,3);
	innerSizer->AddGrowableCol(1);
	innerSizer->AddGrowableRow(1);

	innerSizer->Add(new wxStaticText(this, -1, "Name"));
	m_name = new wxTextCtrl(this,ID_IMPORT_WIZ_DATALOG_NAME_TEXT,GetDefaultDatalogName());
	innerSizer->Add(m_name, 1, wxEXPAND);
	m_defaultColor = m_name->GetBackgroundColour();

	innerSizer->Add(new wxStaticText(this, -1, "Description"));
	m_description = new wxTextCtrl(this,-1,"",wxDefaultPosition, wxDefaultSize,wxTE_MULTILINE);
	innerSizer->Add(m_description, 1, wxEXPAND);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(innerSizer,1,wxALL | wxEXPAND,1 );

    SetSizer(mainSizer);
    mainSizer->Fit(this);
    UpdateUIState();

}

void DatalogInfoPage::UpdateUIState(){

	wxWindow *nextButton = GetParent()->FindWindow(wxID_FORWARD);
	if (m_name->GetValue().Length() == 0){
		nextButton->Enable(false);
		m_name->SetBackgroundColour(ERROR_BACKGROUND_COLOR);
	}
	else{
		nextButton->Enable(true);
		m_name->SetBackgroundColour(m_defaultColor);
	}
}

wxString DatalogInfoPage::GetDefaultDatalogName(){
	return "Datalog " + wxDateTime::Now().FormatDate();
}

void DatalogInfoPage::OnNameChange(wxCommandEvent &event){
	UpdateUIState();
}

void DatalogInfoPage::OnWizardPageChanging(wxWizardEvent& event)
{
	if (event.GetDirection() && m_name->GetValue().Length() == 0){
		event.Veto();
	}
	m_params->datalogName = m_name->GetValue();
	m_params->datalogDesc = m_description->GetValue();
}

BEGIN_EVENT_TABLE ( DatalogInfoPage, wxWizardPageSimple )


	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, DatalogInfoPage::OnWizardPageChanging)
	EVT_TEXT(ID_IMPORT_WIZ_DATALOG_NAME_TEXT, DatalogInfoPage::OnNameChange)

END_EVENT_TABLE()


MapDatalogChannelsPage::MapDatalogChannelsPage(wxWizard *parent,ImportWizardParams *params) :
	wxWizardPageSimple(parent), m_params(params){

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer *innerSizer = new wxFlexGridSizer(1,3,3);
    innerSizer->AddGrowableCol(0);

    m_channelMapGrid = new wxGrid(this,ID_IMPORT_WIZ_CHANNEL_MAP_GRID);
    m_channelMapGrid->CreateGrid(0,5);

    m_channelMapGrid->SetColLabelValue(0,"Channel");
    m_channelMapGrid->SetColLabelValue(1,"Type");
    m_channelMapGrid->SetColLabelValue(2,"Units");
    m_channelMapGrid->SetColLabelValue(3,"Min");
    m_channelMapGrid->SetColLabelValue(4,"Max");

    innerSizer->Add(m_channelMapGrid,1,wxEXPAND);

    mainSizer->Add(innerSizer,1,wxEXPAND);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}


void MapDatalogChannelsPage::OnWizardPageChanged(wxWizardEvent &event){
	PopulateChannels();
	RefreshChannelGrid();
}

void MapDatalogChannelsPage::PopulateChannels(){

	DatalogStore *store = m_params->datalogStore;

	wxFFile file;
	wxString filePath = m_params->datalogFilePath;
	if (!file.Open(filePath.ToAscii())){
		wxString errMsg = wxString::Format("Could not open Datalog file\n\n%s\n\nPlease Check the file you specified",filePath.ToAscii());
		wxMessageBox(errMsg,"Error Opening File");
		return;
	}

	wxArrayString headers;
	store->GetDatalogHeaders(headers,file);

	DatalogChannels &standardChannels = m_params->appOptions->GetStandardChannels();
	DatalogChannelTypes &standardChannelTypes = m_params->appOptions->GetStandardChannelTypes();

	DatalogChannels &channels = m_params->datalogChannels;
	DatalogChannelTypes &channelTypes = m_params->datalogChannelTypes;

	size_t count = headers.Count();
	for (size_t i = 0; i < count; i++){
		int id = DatalogChannelUtil::FindChannelIdByName(standardChannels,headers[i]);
		if (id >= 0){
			DatalogChannel channel = standardChannels[id];
			DatalogChannelType channelType = standardChannelTypes[channel.typeId];

			channelTypes.Add(channelType);
			channel.typeId = channelTypes.GetCount() - 1;
			channels.Add(channel);
		}
		else{
			DatalogChannel channel;
			channel.name = headers[i];
			channels.Add(channel);
		}
	}
}

void MapDatalogChannelsPage::RefreshChannelGrid(){

	DatalogChannels &channels = m_params->datalogChannels;
	DatalogChannelTypes &channelTypes = m_params->datalogChannelTypes;

	size_t count = channels.Count();
	int existingRows = m_channelMapGrid->GetRows();

	m_channelMapGrid->InsertRows(0,count - existingRows,true);

	for (size_t i = 0; i < count; i++){
		DatalogChannel &channel = channels[i];
		m_channelMapGrid->SetCellValue(i,0,channel.name);
		int typeId = channel.typeId;
		if (typeId >=0){
			DatalogChannelType &channelType = channelTypes[typeId];
			m_channelMapGrid->SetCellValue(i,1,channelType.name);
			m_channelMapGrid->SetCellValue(i,2,channelType.unitsLabel);
			m_channelMapGrid->SetCellValue(i,3,wxString::Format("%f",channelType.minValue));
			m_channelMapGrid->SetCellValue(i,4,wxString::Format("%f",channelType.maxValue));
		}
	}
	m_channelMapGrid->AutoSize();
}

BEGIN_EVENT_TABLE ( MapDatalogChannelsPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, MapDatalogChannelsPage::OnWizardPageChanged)

END_EVENT_TABLE()


ImportDatalogWizard::ImportDatalogWizard(wxFrame *frame, ImportWizardParams params) :
	wxWizard(frame,wxID_ANY,_T("Import Datalog"),
	                 wxBitmap(),wxDefaultPosition,
	                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
	                 m_params(params)

{
	wxWizardPageSimple *page1 = new DatalogFileSelectPage(this, &m_params);
	wxWizardPageSimple *page2 = new DatalogInfoPage(this, &m_params);
	wxWizardPageSimple *page3 = new MapDatalogChannelsPage(this, &m_params);
	page1->SetNext(page2);
	page2->SetPrev(page1);
	page2->SetNext(page3);
	page3->SetPrev(page2);
	m_page1 = page1;

	GetPageAreaSizer()->Add(m_page1);

}

