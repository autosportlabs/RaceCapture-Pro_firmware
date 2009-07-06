/*
 * importWizardDialog.cpp
 *
 *  Created on: Jun 22, 2009
 *      Author: brent
 */
#import "importWizardDialog.h"
#import "wx/file.h"
#include "logging.h"

#define LOGGING_FILE_FILTER	"RaceAnalyzer Datalog File (*.log)|*.log|Generic CSV File (*.csv)|*.csv"

#define ERROR_BACKGROUND_COLOR wxColor(0xff,0xe0,0xe0)

wxString ImportWizardParams::GetDefaultDatalogName(){
	return "Datalog " + wxDateTime::Now().FormatDate();
}

ImporterThread::ImporterThread() : wxThread()
{ }

void ImporterThread::SetParams(ImportWizardParams *params, wxWindow *owner){
	m_params = params;
	m_owner = owner;
}

wxThread::ExitCode ImporterThread::Entry(){

	DatalogStore *store = m_params->datalogStore;

	wxCommandEvent resultEvent( IMPORT_RESULT_EVENT, ID_IMPORT_WIZ_IMPORT_RESULT );
	try{

		DatalogChannels existingChannels;
		DatalogChannelTypes existingChannelTypes;

		store->GetChannels(existingChannels);
		store->GetChannelTypes(existingChannelTypes);

		DatalogChannels &importChannels = m_params->datalogChannels;
		DatalogChannelTypes &importChannelTypes = m_params->datalogChannelTypes;

		//blend channels together
		size_t count = importChannels.Count();
		for (size_t i = 0; i < count; i++){

			DatalogChannel importChannel = importChannels[i];

			int existingId = DatalogChannelUtil::FindChannelIdByName(existingChannels,importChannel.name);

			if (existingId < 0){
				//need to add the channel
				int typeId = importChannel.typeId;

				if (typeId >= 0){
					DatalogChannelType importType = importChannelTypes[typeId];

					int existingTypeId = DatalogChannelUtil::FindChannelTypeIdByName(existingChannelTypes, importType.name);

					if (existingTypeId < 0){
						existingChannelTypes.Add(importType);
						importChannel.typeId = existingChannelTypes.Count() - 1;
					}
					else{
						importChannel.typeId = existingTypeId;
					}
				}
				existingChannels.Add(importChannel);
			}
		}

		store->ClearChannelTypes();
		store->ClearChannels();

		store->ImportChannelTypes(existingChannelTypes);
		store->ImportChannels(existingChannels);

		store->ImportDatalog(m_params->datalogFilePath,
				m_params->datalogName,
				m_params->datalogDesc,
				existingChannels,
				existingChannelTypes,
				this);
		resultEvent.SetInt(1);
	}
	catch(DatastoreException e){
		resultEvent.SetInt(0);
		resultEvent.SetString(e.GetMessage());
	}
	catch(FileAccessException e){
		resultEvent.SetInt(0);
		resultEvent.SetString(e.GetMessage());
	}
	catch(...){
		resultEvent.SetInt(0);
		resultEvent.SetString("An unspecified error occurred");
	}
	m_owner->AddPendingEvent(resultEvent);
	return NULL;
}

void ImporterThread::UpdateProgress(int completePercent){
	wxCommandEvent event( IMPORT_PROGRESS_EVENT, ID_IMPORT_WIZ_IMPORT_PROGRESS );
	event.SetInt(completePercent);
	m_owner->AddPendingEvent(event);
}

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

	wxString filePath = fileDialog.GetPath();

	if (GetDatalogFilePath() != filePath){
		m_params->datalogChannels.Clear();
		m_params->datalogChannelTypes.Clear();
	}
	SetDatalogFilePath(filePath);

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
	m_name = new wxTextCtrl(this,ID_IMPORT_WIZ_DATALOG_NAME_TEXT,"");
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

void DatalogInfoPage::OnWizardPageChanged(wxWizardEvent& event){

	m_name->SetValue(m_params->datalogName);
	m_description->SetValue(m_params->datalogDesc);
}

BEGIN_EVENT_TABLE ( DatalogInfoPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, DatalogInfoPage::OnWizardPageChanged)
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
	UpdateUIState();
}

void MapDatalogChannelsPage::PopulateChannels(){

	DatalogChannels &channels = m_params->datalogChannels;
	DatalogChannelTypes &channelTypes = m_params->datalogChannelTypes;

	//skip if we've already populated channels
	if (channels.Count() > 0 && channelTypes.Count() > 0) return;

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

	DatalogChannels existingChannels;
	DatalogChannelTypes existingChannelTypes;

	store->GetChannels(existingChannels);
	store->GetChannelTypes(existingChannelTypes);

	wxArrayString remainingHeaders;
	AddExistingChannels(headers,
			remainingHeaders,
			channels,
			channelTypes,
			existingChannels,
			existingChannelTypes);

	DatalogChannels &standardChannels = m_params->appOptions->GetStandardChannels();
	DatalogChannelTypes &standardChannelTypes = m_params->appOptions->GetStandardChannelTypes();

	wxArrayString unmatchedHeaders;
	AddExistingChannels(remainingHeaders,
			unmatchedHeaders,
			channels,
			channelTypes,
			standardChannels,
			standardChannelTypes);

	size_t unmatchedCount = unmatchedHeaders.Count();
	for (size_t i = 0; i < unmatchedCount; i++){
		channels.Add(DatalogChannel(unmatchedHeaders[i]));
	}

}

void MapDatalogChannelsPage::AddExistingChannels(wxArrayString &headers, wxArrayString &remainingHeaders,
		DatalogChannels &channels,
		DatalogChannelTypes &channelTypes,
		DatalogChannels &existingChannels,
		DatalogChannelTypes &existingChannelTypes
		){


	size_t count = headers.Count();

	for (size_t i = 0; i < count; i++){
		wxString header = headers[i];
		int id = DatalogChannelUtil::FindChannelIdByName(existingChannels,header);
		if (id >= 0){
			DatalogChannel channel = existingChannels[id];
			DatalogChannelType channelType = existingChannelTypes[channel.typeId];

			//did we previously add this channel type?
			int previousTypeId = DatalogChannelUtil::FindChannelTypeIdByName(channelTypes,channelType.name);
			if (previousTypeId >= 0){ //if so, re-use it for this new channel
				channel.typeId = previousTypeId;
			}
			else{ //otherwise add it
				channelTypes.Add(channelType);
				channel.typeId = channelTypes.GetCount() - 1;
			}
			channels.Add(channel);
		}
		else{
			remainingHeaders.Add(header);
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
			m_channelMapGrid->SetCellValue(i,3,wxString::Format("%.2f",channelType.minValue));
			m_channelMapGrid->SetCellValue(i,4,wxString::Format("%.2f",channelType.maxValue));
		}
	}
	m_channelMapGrid->AutoSize();
}

void MapDatalogChannelsPage::UpdateUIState(){


	GetParent()->FindWindow(wxID_BACKWARD)->Enable(true);
	GetParent()->FindWindow(wxID_FORWARD)->Enable(true);
}

BEGIN_EVENT_TABLE ( MapDatalogChannelsPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, MapDatalogChannelsPage::OnWizardPageChanged)

END_EVENT_TABLE()


DatalogImporterPage::DatalogImporterPage(wxWizard *parent, ImportWizardParams *params) :
	wxWizardPageSimple(parent), m_params(params), m_importing(false){

    wxFlexGridSizer *mainSizer = new wxFlexGridSizer(3,1,3,3);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(0);
    mainSizer->AddGrowableRow(2);

    m_progressGauge = new wxGauge(this,-1,100);
    m_progressMessage = new wxStaticText(this,-1,"",wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE | wxST_NO_AUTORESIZE);

    m_statusMessage = new wxStaticText(this,-1,"Ready To Import- Click 'Start' to Proceed",wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);

    wxButton *startImportButton = new wxButton(this,ID_IMPORT_WIZ_START_IMPORT,"Start");

    mainSizer->Add(m_statusMessage,1,wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
    mainSizer->Add(m_progressGauge,0,wxEXPAND);
    mainSizer->Add(m_progressMessage,1,wxEXPAND);
    mainSizer->Add(startImportButton,0);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->Layout();
}

void DatalogImporterPage::UpdateUIState(){

	bool alreadyImported = m_params->datalogImported;

	FindWindow(ID_IMPORT_WIZ_START_IMPORT)->Enable(! m_importing && ! alreadyImported);
	GetParent()->FindWindow(wxID_FORWARD)->Enable(! m_importing && alreadyImported);
	GetParent()->FindWindow(wxID_BACKWARD)->Enable(! m_importing && ! alreadyImported);
	GetParent()->FindWindow(wxID_CANCEL)->Enable(! m_importing && ! alreadyImported);

}

void DatalogImporterPage::OnWizardPageChanged(wxWizardEvent &event){

	m_importing = false;
	UpdateUIState();
}

void DatalogImporterPage::OnStartImport(wxCommandEvent &event){

	m_statusMessage->SetLabel("Importing Datalog Session");
	DoImportDatalog();
	m_importing = true;
	UpdateUIState();
}

void DatalogImporterPage::OnWizardPageFinished(wxWizardEvent &event){

}

void DatalogImporterPage::OnWizardPageChanging(wxWizardEvent &event){


}

void DatalogImporterPage::DoImportDatalog(){

	m_importerThread = new ImporterThread();
	m_importerThread->SetParams(m_params,this);
	m_importerThread->Create();
	m_importerThread->Run();
}

void DatalogImporterPage::OnImportProgress(wxCommandEvent &event){
	int percent = event.GetInt();
	m_progressGauge->SetValue(percent);
	m_progressMessage->SetLabel(wxString::Format("Progress: %d %%",percent));
}

void DatalogImporterPage::OnImportResult(wxCommandEvent &event){

	int result = event.GetInt();

	if (result){
		m_statusMessage->SetLabel("Import Complete");
		m_params->datalogImported = true;
	}
	else{
		wxString msg("Error Importing Datalog: \n\n");
		msg += event.GetString();
		wxMessageBox(msg ,"Error Importing",wxICON_ERROR);
	}
	m_importing = false;
	UpdateUIState();

}

BEGIN_EVENT_TABLE ( DatalogImporterPage, wxWizardPageSimple )

	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, DatalogImporterPage::OnWizardPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, DatalogImporterPage::OnWizardPageChanging)
	EVT_WIZARD_FINISHED(wxID_ANY, DatalogImporterPage::OnWizardPageFinished)
	EVT_BUTTON(ID_IMPORT_WIZ_START_IMPORT, DatalogImporterPage::OnStartImport)
	EVT_COMMAND( ID_IMPORT_WIZ_IMPORT_PROGRESS, IMPORT_PROGRESS_EVENT, DatalogImporterPage::OnImportProgress )
	EVT_COMMAND( ID_IMPORT_WIZ_IMPORT_RESULT, IMPORT_RESULT_EVENT, DatalogImporterPage::OnImportResult)

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
	wxWizardPageSimple *page4 = new DatalogImporterPage(this, &m_params);
	page1->SetNext(page2);
	page2->SetPrev(page1);
	page2->SetNext(page3);
	page3->SetPrev(page2);
	page3->SetNext(page4);
	page4->SetPrev(page3);

	m_page1 = page1;

	GetPageAreaSizer()->Add(m_page1);

}

DEFINE_EVENT_TYPE ( IMPORT_PROGRESS_EVENT )
DEFINE_EVENT_TYPE ( IMPORT_RESULT_EVENT )
