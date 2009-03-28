
#include "raceAnalyzer.h"


#include "optionsDialog.h"
#include "fileIO.h"

#include "filenew.xpm"
#include "fileopen.xpm"
#include "filesave.xpm"
#include "filesaveas.xpm"
#include "getconfig.xpm"
#include "writeconfig.xpm"
#include "commitflash.xpm"
#include "line-chart.xpm"
#include "runtime.xpm"
#include "advancetable.xpm"
#include "tuning.xpm"
#include "mjlj_logo_small.xpm"

#include "mjlj_icon.xpm"
#include "mjlj_icon_med.xpm"
#include "mjlj_icon_large.xpm"
#include "mjlj_icon_xlarge.xpm"

//wxAUI string definitions
#define PANE_CONFIGURATION 		"config"
#define PANE_RUNTIME			"runtime"
#define PANE_TUNING				"tuning"


#define CAPTION_TUNING			"Tuning"
#define CAPTION_RUNTIME 		"Runtime"
#define CAPTION_CONFIG			"Configuration"

IMPLEMENT_APP(RaceAnalyzerApp);


bool RaceAnalyzerApp::OnInit()
{
	// Create an instance of our frame, or window
	_mainFrame = new MainFrame(MJLJ_WINDOW_TITLE, wxPoint(1, 1), wxSize(300,300));

	wxIconBundle iconBundle;
	iconBundle.AddIcon(wxIcon(mjlj_icon_xlarge_xpm));
	iconBundle.AddIcon(wxIcon(mjlj_icon_large_xpm));
	iconBundle.AddIcon(wxIcon(mjlj_icon_med_xpm));
	iconBundle.AddIcon(wxIcon(mjlj_icon_xpm));
	_mainFrame->SetIcons(iconBundle);

	_mainFrame->Show(TRUE); // show the window
	SetTopWindow(_mainFrame); // and finally, set it as the main window

  return wxApp::OnInit();
}

void RaceAnalyzerApp::OnInitCmdLine(wxCmdLineParser& parser){
   parser.AddParam("",wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
   parser.Parse(false);
   if (parser.GetParamCount() > 0){
   		wxString mjljConfigFile = parser.GetParam(0);
   		_mainFrame->LoadConfigurationFile(mjljConfigFile);
   }
}

bool RaceAnalyzerApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
   return true;
}


MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize
  &size) : wxFrame((wxFrame*)NULL,  - 1, title, pos, size)
{
	_appTerminated = false;
	ShowSplashScreen();

	_currentConfigFileName = NULL;
	_activeConfig = -1;
	_appOptions.LoadAppOptions();
	_appPrefs.LoadAppPrefs();

	InitializeFrame();
	SetSize(wxSize(800,640));
	Center();

	m_raceAnalyzerComm.SetSerialPort(_appOptions.GetSerialPort());

	try{
	 LoadInitialConfig();
	}
	catch(...){}

}

MainFrame::~MainFrame(){
	TerminateApp();
	_frameManager.UnInit();
}


void MainFrame::LoadInitialConfig(){


}

void MainFrame::ShowSplashScreen(){

	wxBitmap splashBitmap(mjlj_logo_small_xpm);

	wxSplashScreen* splash = new wxSplashScreen(splashBitmap,
      wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
      4000, NULL, -1, wxDefaultPosition, wxDefaultSize,
      wxSIMPLE_BORDER|wxSTAY_ON_TOP);
	wxYield();
}

void MainFrame::InitializeFrame(){

	_frameManager.SetFrame(this);
	_frameManager.SetFlags(_frameManager.GetFlags() ^ wxAUI_MGR_TRANSPARENT_DRAG);

	CreateStatusBar(3);

	InitializeComponents();

	_frameManager.Update();

	if (0 == _appPrefs.GetPerspectives().Count()){
		CreateDefaultPerspectives();
		_appPrefs.SetActivePerspective(_appPrefs.GetPerspectives().Count() - 1);
		SwitchToPerspective(0);
	}
	else{
		SwitchToPerspective(_appPrefs.GetActivePerspective());
	}

	InitializeMenus();
}


void MainFrame::CreateDefaultPerspectives(){

	CreateDefaultConfigPerspective();
	CreateDefaultRuntimePerspective();
	CreateDefaultTuningPerspective();
	_appPrefs.SaveAppPrefs();
}

void MainFrame::CreateDefaultConfigPerspective(){

	_frameManager.GetPane(wxT(PANE_RUNTIME)).Show(false);
	_frameManager.GetPane(wxT(PANE_TUNING)).Show(false);
	_frameManager.GetPane(wxT(PANE_CONFIGURATION)).Show(true);
	_frameManager.Update();

	wxString perspective = _frameManager.SavePerspective();

	_appPrefs.GetPerspectives().Add(perspective);
	_appPrefs.GetPerspectiveNames().Add(PERSPECTIVE_CONFIG);
}

void MainFrame::CreateDefaultRuntimePerspective(){

	_frameManager.GetPane(wxT(PANE_TUNING)).Show(false);
	_frameManager.GetPane(wxT(PANE_CONFIGURATION)).Show(false);
	_frameManager.GetPane(wxT(PANE_RUNTIME)).Show(true);
	_frameManager.Update();

	wxString perspective = _frameManager.SavePerspective();

	_appPrefs.GetPerspectives().Add(perspective);
	_appPrefs.GetPerspectiveNames().Add(PERSPECTIVE_RUNTIME);
}

void MainFrame::CreateDefaultTuningPerspective(){

	_frameManager.GetPane(wxT(PANE_RUNTIME)).Show(false);
	_frameManager.GetPane(wxT(PANE_CONFIGURATION)).Show(false);
	_frameManager.GetPane(wxT(PANE_TUNING)).Show(true);
	_frameManager.Update();

	wxString perspective = _frameManager.SavePerspective();

	_appPrefs.GetPerspectives().Add(perspective);
	_appPrefs.GetPerspectiveNames().Add(PERSPECTIVE_TUNING);

}


void MainFrame::SaveCurrentPerspective(){

	//save the current perspective
	int currentPerspective = _appPrefs.GetActivePerspective();
	_appPrefs.GetPerspectives()[currentPerspective] = _frameManager.SavePerspective();

}

void MainFrame::SwitchToPerspective(int id){

	//load the new perspective
	_frameManager.LoadPerspective(_appPrefs.GetPerspectives()[id],true);
	_appPrefs.SetActivePerspective(id);
	_frameManager.Update();
}

void MainFrame::OnConfigPerspective(wxCommandEvent& event){
	SaveCurrentPerspective();
	SwitchToPerspective(0);
}

void MainFrame::OnRuntimePerspective(wxCommandEvent& event){
	SaveCurrentPerspective();
	SwitchToPerspective(1);
}

void MainFrame::OnTuningPerspective(wxCommandEvent& event){
	SaveCurrentPerspective();
	SwitchToPerspective(2);
}

void MainFrame::InitializeMenus(){


	//initialize main menu
	wxMenuBar* menuBar = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(wxID_NEW, wxT("New\tCtrl+N"), wxT("Create a new Ignition Configuration"));
	fileMenu->Append(wxID_OPEN, wxT("Open\tCtrl+O"), wxT("Open a saved Ignition Configuration"));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_SAVE, wxT("Save\tCtrl+S"), wxT("Save the current Ignition Configuration"));
	fileMenu->Append(wxID_SAVEAS, wxT("Save As\tCtrl+Shift+S"), wxT("Save the current Ignition Configuration under a new file"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_QUICKLOGGING, wxT("Quick Datalog\tF5"));
	_startDatalogMenuItem = fileMenu->Append(ID_STARTLOG, wxT("Start Datalog\tF6"));
	_stopDatalogMenuItem = fileMenu->Append(ID_STOPLOG, wxT("Stop Datalog\tESC"));

	fileMenu->Append(ID_OPENDATALOG,wxT("Open Datalog"));

	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, wxT("Exit"), wxT("Exit the program"));
	menuBar->Append(fileMenu, wxT("File"));

	wxMenu* editMenu = new wxMenu();
	menuBar->Append(editMenu, wxT("Edit"));

	wxMenu* toolsMenu = new wxMenu();
	toolsMenu->Append(ID_OPTIONS, wxT("Configurator Options"));

	wxMenu* viewMenu = new wxMenu();
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_RESTORE_DEFAULT_VIEWS, "Restore Default View");

	menuBar->Append(viewMenu, "View");

	wxMenu* perspectiveMenu = new wxMenu();
	int perspectiveCount = _appPrefs.GetPerspectiveNames().Count();

	for (int i = 0; i < perspectiveCount; i++){
		perspectiveMenu->Append(ID_PERSPECTIVES + i, wxString::Format("%s\tF%s",_appPrefs.GetPerspectiveNames()[i].ToAscii(), wxString::Format("%d",2+i).ToAscii()));
	}

	menuBar->Append(perspectiveMenu, wxT("Perspective"));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(ID_HELP_ABOUT, wxT("About MJLJ Configurator"));
	menuBar->Append(helpMenu, wxT("Help"));

	SetMenuBar(menuBar);

	//initialize tool bar
	wxToolBar* toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER);
	wxBitmap bmpNew(filenew_xpm);
	wxBitmap bmpOpen(fileopen_xpm);
	wxBitmap bmpSave(filesave_xpm);
	wxBitmap bmpSaveAs(filesaveas_xpm);
	wxBitmap bmpGetConfig(getconfig_xpm);
	wxBitmap bmpWriteConfig(writeconfig_xpm);
	wxBitmap bmpCommitFlash(commitflash_xpm);
	wxBitmap bmpChart(line_chart_xpm);
	wxBitmap bmpRuntime(runtime_xpm);
	wxBitmap bmpAdvanceTable(advancetable_xpm);
	wxBitmap bmpTuning(tuning_xpm);

	toolBar->AddTool(wxID_NEW, bmpNew, wxT("New Ignition Configuration"));
	toolBar->AddTool(wxID_OPEN, bmpOpen, wxT("Open"));
	toolBar->AddSeparator();
	toolBar->AddTool(wxID_SAVE, bmpSave, wxT("Save"));
	toolBar->AddTool(wxID_SAVEAS, bmpSaveAs, wxT("Save As"));
	toolBar->AddSeparator();
	toolBar->AddTool(ID_CONFIG_MODE, bmpAdvanceTable, wxT("Edit IgnitionConfiguration"));
	toolBar->AddTool(ID_RUNTIME_MODE, bmpChart, wxT("Charting/Runtime View"));
	toolBar->AddTool(ID_TUNING_MODE, bmpTuning, wxT("Tuning"));

	toolBar->Realize();
	SetToolBar(toolBar);
}

void MainFrame::InitializeComponents(){


	m_runtimePanel = new wxPanel(this);
	_frameManager.AddPane(m_runtimePanel, wxAuiPaneInfo().Name(wxT(PANE_RUNTIME)).Caption(wxT(CAPTION_RUNTIME)).Center().Hide());

	m_tuningPanel = new wxPanel(this);
	_frameManager.AddPane(m_tuningPanel, wxAuiPaneInfo().Name(wxT(PANE_TUNING)).Caption(wxT(CAPTION_TUNING)).Center().Show(true));

	m_configPanel = new wxPanel(this);
	_frameManager.AddPane(m_configPanel, wxAuiPaneInfo().Name(wxT(PANE_CONFIGURATION)).Caption(wxT(CAPTION_CONFIG)).Center());
}


void MainFrame::OnHelpAbout(wxCommandEvent &event){

	wxString msg = wxString::Format("MegaJolt Lite Jr. Configurator %s\n\nhttp://www.autosportlabs.net\n\nCopyright © 2004-2008 Autosport Labs\n\n",MJLJ_CONFIG_VERSION);
	wxMessageDialog dlg(this,msg, "About", wxOK);
	dlg.ShowModal();
}

void MainFrame::UpdateLoggingStatus(){

//	bool isLogging = _datalogger.IsLogging();
	bool isLogging = false;
	_startDatalogMenuItem->Enable(! isLogging);
	_stopDatalogMenuItem->Enable(isLogging);
	SetActivityMessage(isLogging ? "Datalogging" : "");
}


void MainFrame::OnStartLogging(wxCommandEvent &event){

	wxString defaultDir = _appPrefs.GetLastDatalogDirectory();
	wxString defaultFile = "";
	wxFileDialog fileDialog(this, "Datalog to File", defaultDir, defaultFile, LOGGING_FILE_FILTER,wxSAVE);

	int result = fileDialog.ShowModal();

	if (wxID_OK == result){
		const wxString fileName = fileDialog.GetPath();
		if (!wxFile::Exists(fileName) || (wxFile::Exists(fileName) && QueryFileOverwrite())){

			//start logging here

			_appPrefs.SetLastDatalogDirectory(fileDialog.GetDirectory());
		}
	}
	UpdateLoggingStatus();
}

void MainFrame::OnOpenDatalog(wxCommandEvent &event){

	wxString defaultDir = _appPrefs.GetLastDatalogDirectory();
	wxString defaultFile = "";
	wxFileDialog fileDialog(this, "Open Datalog", defaultDir, defaultFile, LOGGING_FILE_FILTER, wxOPEN);
	int result = fileDialog.ShowModal();

	if (wxID_OK == result){
		const wxString fileName = fileDialog.GetPath();
		_appPrefs.SetLastDatalogDirectory(fileDialog.GetDirectory());

	}
}

void MainFrame::OnQuickLogging(wxCommandEvent &event){

//	if (_datalogger.IsLogging()) _datalogger.StopLogging();

	wxDateTime now = wxDateTime::Now();

	wxString filename = "MJLJ_Log_" + now.Format("%Y-%m-%d_%H.%M.%S") + ".csv";
	wxString lastDirectory = _appPrefs.GetLastDatalogDirectory();

	if ( "" != lastDirectory){
		filename = lastDirectory + wxFileName::GetPathSeparator() + filename;
	}
	//_datalogger.StartLogging(filename);
	UpdateLoggingStatus();
}

void MainFrame::OnStopLogging(wxCommandEvent &event){

//	if (_datalogger.IsLogging()) _datalogger.StopLogging();
	UpdateLoggingStatus();
}

void MainFrame::NotifyConfigChanged(){

	wxCommandEvent event( CONFIG_STALE_EVENT, CONFIG_STALE );
	event.SetEventObject(this);
	m_tuningPanel->AddPendingEvent(event);
	m_configPanel->AddPendingEvent(event);
	m_runtimePanel->AddPendingEvent(event);
}


void MainFrame::OnConfigChanged(wxCommandEvent &event){

	event.StopPropagation();
	wxObject *obj = event.GetEventObject();
	if (obj == NULL){
		wxLogMessage("null event object");
		return;
	}

	event.Skip(false);
	ConfigModified();
	NotifyConfigChanged();
}

void MainFrame::ConfigModified(){
	SyncControls();
}

void MainFrame::SyncControls(){

}


void MainFrame::OnSwitchView(wxCommandEvent &event){

	SaveCurrentPerspective();
	SwitchToPerspective(event.GetId() - ID_PERSPECTIVES);
}

void MainFrame::SetActivityMessage(const wxString& message){
	SetStatusText(message,0);
}

void MainFrame::SetStatusMessage(const wxString& message){
	SetStatusText(message,1);
}

void MainFrame::ClearStatusMessage(){
	SetStatusText("",1);
}

void MainFrame::ClearActivityMessage(){
	SetStatusText("",0);
}


void MainFrame::SaveCurrentConfig(){
	if ( _currentConfigFileName ){
		MJLJConfigFileWriter writer;
		const wxString fileName(*_currentConfigFileName);
		writer.SetFileName(fileName);

		try{
			writer.WriteConfigData(_currentConfigData);
			_configModified = false;
			SetStatusMessage("Ignition Configuration Saved");
		}
		catch (FileIOException e){
			wxMessageDialog dlg(this, wxString::Format("Failed to save Ignition Configuration:\n\n%s", e.GetMessage().ToAscii()), "Error saving", wxOK | wxICON_HAND);
			dlg.ShowModal();
			return;
		}
	}
	else{
		SaveAsCurrentConfig();
	}
	UpdateConfigFileStatus();
}

bool MainFrame::QueryFileOverwrite(){

	wxMessageDialog dlg(this, "File Exists- Overwrite?","File exists", wxYES_NO | wxNO_DEFAULT);
	return ( wxID_YES == dlg.ShowModal() );
}

void MainFrame::SaveAsCurrentConfig(){

	wxString defaultDir = _appPrefs.GetLastConfigFileDirectory();
	wxString defaultFile = "";
	wxFileDialog fileDialog(this, "Save As Ignition Configuration", defaultDir, defaultFile, CONFIG_FILE_FILTER,wxSAVE);

	int result = fileDialog.ShowModal();

	if (wxID_OK == result){
		MJLJConfigFileWriter writer;

		const wxString fileName = fileDialog.GetPath();
		if (!wxFile::Exists(fileName) || (wxFile::Exists(fileName) && QueryFileOverwrite())){
			writer.SetFileName(fileName);

			try{
				writer.WriteConfigData(_currentConfigData);
				_currentConfigFileName = new wxString(fileName);
				_configModified = false;
				SetStatusMessage("Ignition Configuration Saved");
				_appPrefs.SetLastConfigFileDirectory(fileDialog.GetDirectory());
			}
			catch (FileIOException e){
				wxMessageDialog dlg(this, wxString::Format("Failed to save Ignition Configuration:\n\n%s", e.GetMessage().ToAscii()), "Error saving", wxOK | wxICON_HAND);
				dlg.ShowModal();
				return;
			}
		}
	}
	UpdateConfigFileStatus();
}

void MainFrame::OnSaveAsCurrentConfig(wxCommandEvent& event){
	SaveAsCurrentConfig();
}

void MainFrame::OnSaveCurrentConfig(wxCommandEvent& event){
	SaveCurrentConfig();
}


void MainFrame::OnAppOptions(wxCommandEvent& event){

	OptionsDialog optionsDialog;
	optionsDialog.SetAppOptions(&_appOptions);
	optionsDialog.Create(this);

	if (optionsDialog.ShowModal() == wxID_OK) {
		_appOptions.SaveAppOptions();
		m_raceAnalyzerComm.SetSerialPort(_appOptions.GetSerialPort());
	}
}


bool MainFrame::QuerySaveModifications(){
	wxMessageDialog dlg(this, "Current Configuration has not been saved. Save Changes?","Configuration Modified", wxYES_NO | wxNO_DEFAULT);
	return ( wxID_YES == dlg.ShowModal() );
}


void MainFrame::OnNewConfig(wxCommandEvent &event){

	if (_configModified){
		if (QuerySaveModifications()) SaveCurrentConfig();
	}
	//Set default values
	NotifyConfigChanged();
}


void MainFrame::OnOpenConfig(wxCommandEvent& event){

	if (_configModified){
		if (QuerySaveModifications()) SaveCurrentConfig();
	}

	wxString defaultDir = _appPrefs.GetLastConfigFileDirectory();
	wxString defaultFile = "";
	wxFileDialog fileDialog(this, "Open Ignition Configuration", defaultDir, defaultFile, CONFIG_FILE_FILTER, wxOPEN);

	int result = fileDialog.ShowModal();

	if (wxID_OK == result){
		const wxString fileName = fileDialog.GetPath();
		LoadConfigurationFile(fileName);
		_appPrefs.SetLastConfigFileDirectory(fileDialog.GetDirectory());
	}
}

void MainFrame::LoadConfigurationFile(const wxString fileName){

	MJLJConfigFileReader reader;

	reader.SetFileName( fileName);

	try{
		reader.ReadConfiguration(_currentConfigData);
		if (_currentConfigFileName) delete _currentConfigFileName;
		_currentConfigFileName = new wxString(fileName);
		_configModified = false;
		_staleRAMMemory = true;
		UpdateCommControls();
		NotifyConfigChanged();
		SyncControls();
		UpdateConfigFileStatus();
		SetStatusMessage("Ignition Configuration Loaded");
	}
	catch( FileIOException e ){
		wxMessageDialog dlg(this, wxString::Format("Failed to load Ignition Configuration:\n\n%s", e.GetMessage().ToAscii()), "Error loading", wxOK | wxICON_HAND);
		dlg.ShowModal();
		return;
	}
	UpdateConfigFileStatus();
}


void MainFrame::UpdateCommControls(){


}

void MainFrame::UpdateConfigFileStatus(){


	wxString modifiedLabel = _configModified ? "(Modified)" : "";
	wxString windowTitle = wxString::Format(MJLJ_WINDOW_TITLE,MJLJ_CONFIG_VERSION);

	wxString title;
	if (! _currentConfigFileName){
		title.Printf("%s - No file loaded %s",windowTitle.ToAscii(), modifiedLabel.ToAscii());
	}
	else{
		title.Printf("%s - %s %s", windowTitle.ToAscii(), _currentConfigFileName->ToAscii(), modifiedLabel.ToAscii());
	}
	SetTitle(title);

	wxToolBar *toolBar = GetToolBar();
	toolBar->EnableTool(wxID_SAVE, _configModified );
	toolBar->EnableTool(wxID_SAVEAS, _configModified );
}

void MainFrame::OnRestoreDefaultView(wxCommandEvent &event){

	CreateDefaultPerspectives();
}

void MainFrame::OnFileExit(wxCommandEvent &event){
	TerminateApp();
}

void MainFrame::OnExit(wxCloseEvent& WXUNUSED(event)){
	TerminateApp();
}


void MainFrame::TerminateApp(){

	if (_appTerminated) return;
	_appTerminated = true;
	//Cancel exit if there are unsaved changes and the user opts to not save
	if (_configModified && QuerySaveModifications()) SaveCurrentConfig();

	SaveCurrentPerspective();
	_appPrefs.SaveAppPrefs();

	Destroy();
}

BEGIN_EVENT_TABLE ( MainFrame, wxFrame )
  	EVT_CLOSE (MainFrame::OnExit)

    EVT_MENU(wxID_EXIT, MainFrame::OnFileExit)
    EVT_MENU(ID_OPTIONS, MainFrame::OnAppOptions)

    EVT_MENU_RANGE(ID_PERSPECTIVES, ID_PERSPECTIVES + MAX_PERSPECTIVES, MainFrame::OnSwitchView)

    EVT_MENU(wxID_NEW, MainFrame::OnNewConfig)
    EVT_MENU(wxID_SAVE,MainFrame::OnSaveCurrentConfig)
    EVT_MENU(wxID_OPEN,MainFrame::OnOpenConfig)
    EVT_MENU(wxID_SAVEAS,MainFrame::OnSaveAsCurrentConfig)

    EVT_COMMAND( CONFIG_CHANGED, CONFIG_CHANGED_EVENT, MainFrame::OnConfigChanged )

	EVT_MENU( ID_CONFIG_MODE, MainFrame::OnConfigPerspective)
	EVT_MENU( ID_RUNTIME_MODE, MainFrame::OnRuntimePerspective)
	EVT_MENU( ID_TUNING_MODE, MainFrame::OnTuningPerspective)


	EVT_MENU( ID_STARTLOG, MainFrame::OnStartLogging)
	EVT_MENU( ID_QUICKLOGGING, MainFrame::OnQuickLogging)
	EVT_MENU( ID_STOPLOG, MainFrame::OnStopLogging)
	EVT_MENU( ID_OPENDATALOG, MainFrame::OnOpenDatalog)

	EVT_MENU( ID_HELP_ABOUT, MainFrame::OnHelpAbout)


END_EVENT_TABLE()



