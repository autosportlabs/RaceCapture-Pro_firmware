
#include "raceAnalyzer.h"


#include "optionsDialog.h"
#include "fileIO.h"
#include "logging.h"
#include "lineChart.h"
#include "importWizardDialog.h"

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
#include "import.xpm"

#include "mjlj_icon.xpm"
#include "mjlj_icon_med.xpm"
#include "mjlj_icon_large.xpm"
#include "mjlj_icon_xlarge.xpm"

//wxAUI string definitions
#define PANE_CONFIGURATION 		"config"
#define PANE_RUNTIME			"runtime"
#define PANE_ANALYZE			"analysis"
#define PANE_SCRIPT				"script"

#define CAPTION_ANALYSIS		"Analysis"
#define CAPTION_RUNTIME 		"Runtime"
#define CAPTION_CONFIG			"Configuration"
#define CAPTION_SCRIPT			"Script"

IMPLEMENT_APP(RaceAnalyzerApp);


bool RaceAnalyzerApp::OnInit()
{
	// Create an instance of our frame, or window
	_mainFrame = new MainFrame(RACEANALYZER_WINDOW_TITLE, wxPoint(1, 1), wxSize(300,300));

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
   		_mainFrame->OpenRaceEvent(mjljConfigFile);
   }
}

bool RaceAnalyzerApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
   return true;
}


MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize
  &size) : wxFrame((wxFrame*)NULL,  - 1, title, pos, size)
{
	m_appTerminated = false;
	//ShowSplashScreen();

	m_activeConfig = -1;
	m_appOptions.LoadAppOptions();
	_appPrefs.LoadAppPrefs();

	InitializeFrame();
	SetSize(wxSize(800,640));
	Center();

	InitComms();

}

MainFrame::~MainFrame(){
	TerminateApp();
	_frameManager.UnInit();
}


void MainFrame::InitComms(){

	m_scriptPanel->SetComm(&m_raceAnalyzerComm);
	try{
		m_raceAnalyzerComm.SetSerialPort(m_appOptions.GetSerialPort());
	}
	catch(CommException e){
		SetStatusMessage(e.GetErrorMessage());
	}
}


void MainFrame::ShowSplashScreen(){

	wxBitmap splashBitmap(mjlj_logo_small_xpm);

	new wxSplashScreen(splashBitmap,
      wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
      4000, NULL, -1, wxDefaultPosition, wxDefaultSize,
      wxSIMPLE_BORDER|wxSTAY_ON_TOP);
	wxYield();
}

void MainFrame::InitializeFrame(){

	_frameManager.SetFrame(this);

#ifdef __WXMSW_
	_frameManager.SetFlags(_frameManager.GetFlags() ^ wxAUI_MGR_TRANSPARENT_DRAG);
#else
	//workaround for bug
	//http://trac.wxwidgets.org/ticket/4841
	_frameManager.SetFlags(wxAUI_MGR_ALLOW_FLOATING |
			wxAUI_MGR_NO_VENETIAN_BLINDS_FADE |
			wxAUI_MGR_RECTANGLE_HINT
			);
#endif

	CreateStatusBar(3);

	InitializeComponents();

	_frameManager.Update();

	if (0 == _appPrefs.GetPerspectives().Count()){
		CreateDefaultPerspectives();
		int perspectiveCount = _appPrefs.GetPerspectives().Count();
		_appPrefs.SetActivePerspective(perspectiveCount - 1);
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
	CreateDefaultAnalyzePerspective();
	CreateDefaultScriptPerspective();
	_appPrefs.SaveAppPrefs();
}

void MainFrame::CreateDefaultConfigPerspective(){

	_frameManager.GetPane(wxT(PANE_CONFIGURATION)).Show(true);
	_frameManager.GetPane(wxT(PANE_ANALYZE)).Show(false);
	_frameManager.GetPane(wxT(PANE_RUNTIME)).Show(false);
	_frameManager.GetPane(wxT(PANE_SCRIPT)).Show(false);
	_frameManager.Update();

	wxString perspective = _frameManager.SavePerspective();

	_appPrefs.GetPerspectives().Add(perspective);
	_appPrefs.GetPerspectiveNames().Add(PERSPECTIVE_CONFIG);
}

void MainFrame::CreateDefaultScriptPerspective(){

	_frameManager.GetPane(wxT(PANE_CONFIGURATION)).Show(false);
	_frameManager.GetPane(wxT(PANE_ANALYZE)).Show(false);
	_frameManager.GetPane(wxT(PANE_RUNTIME)).Show(false);
	_frameManager.GetPane(wxT(PANE_SCRIPT)).Show(true);
	_frameManager.Update();

	wxString perspective = _frameManager.SavePerspective();

	_appPrefs.GetPerspectives().Add(perspective);
	_appPrefs.GetPerspectiveNames().Add(PERSPECTIVE_SCRIPT);
}

void MainFrame::CreateDefaultRuntimePerspective(){

	_frameManager.GetPane(wxT(PANE_CONFIGURATION)).Show(false);
	_frameManager.GetPane(wxT(PANE_ANALYZE)).Show(false);
	_frameManager.GetPane(wxT(PANE_RUNTIME)).Show(true);
	_frameManager.GetPane(wxT(PANE_SCRIPT)).Show(false);
	_frameManager.Update();

	wxString perspective = _frameManager.SavePerspective();

	_appPrefs.GetPerspectives().Add(perspective);
	_appPrefs.GetPerspectiveNames().Add(PERSPECTIVE_RUNTIME);
}

void MainFrame::CreateDefaultAnalyzePerspective(){

	_frameManager.GetPane(wxT(PANE_CONFIGURATION)).Show(false);
	_frameManager.GetPane(wxT(PANE_ANALYZE)).Show(true);
	_frameManager.GetPane(wxT(PANE_RUNTIME)).Show(false);
	_frameManager.GetPane(wxT(PANE_SCRIPT)).Show(false);
	_frameManager.Update();

	wxString perspective = _frameManager.SavePerspective();

	_appPrefs.GetPerspectives().Add(perspective);
	_appPrefs.GetPerspectiveNames().Add(PERSPECTIVE_ANALYZE);

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

void MainFrame::OnAnalyzePerspective(wxCommandEvent& event){
	SaveCurrentPerspective();
	SwitchToPerspective(2);
}

void MainFrame::InitializeMenus(){

	//initialize main menu
	wxMenuBar* menuBar = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(wxID_NEW, wxT("New\tCtrl+N"), wxT("Create a new Race Event"));
	fileMenu->Append(wxID_OPEN, wxT("Open\tCtrl+O"), wxT("Open a Race Event"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_IMPORT_DATALOG, wxT("Import\tCtrl-I"),wxT("Import Datalog"));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, wxT("Exit"), wxT("Exit the program"));
	menuBar->Append(fileMenu, wxT("File"));

	wxMenu* editMenu = new wxMenu();
	menuBar->Append(editMenu, wxT("Edit"));

	wxMenu* toolsMenu = new wxMenu();
	toolsMenu->Append(ID_OPTIONS, wxT("Configurator Options"));
	menuBar->Append(toolsMenu, "Tools");

	wxMenu* viewMenu = new wxMenu();
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_RESTORE_DEFAULT_VIEWS, "Restore Default View");

	menuBar->Append(viewMenu, "View");

	wxMenu *chartsMenu = new wxMenu();
	chartsMenu->Append(ID_ADD_LINE_CHART,wxT("Line Chart"));
	menuBar->Append(chartsMenu, "Charts");

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
	wxBitmap bmpGetConfig(getconfig_xpm);
	wxBitmap bmpWriteConfig(writeconfig_xpm);
	wxBitmap bmpCommitFlash(commitflash_xpm);
	wxBitmap bmpChart(line_chart_xpm);
	wxBitmap bmpRuntime(runtime_xpm);
	wxBitmap bmpAdvanceTable(advancetable_xpm);
	wxBitmap bmpAnalyze(tuning_xpm);
	wxBitmap bmpImport(import_xpm);

	toolBar->AddTool(wxID_NEW, bmpNew, wxT("Create new Race Event"));
	toolBar->AddTool(wxID_OPEN, bmpOpen, wxT("Open a Race Event"));
	toolBar->AddTool(ID_IMPORT_DATALOG,bmpImport,wxT("ImportDatalog"));

	toolBar->AddSeparator();
	toolBar->AddTool(ID_CONFIG_MODE, bmpAdvanceTable, wxT("Edit Configuration"));
	toolBar->AddTool(ID_RUNTIME_MODE, bmpChart, wxT("Monitor Runtime Channels"));
	toolBar->AddTool(ID_ANALYZE_MODE, bmpAnalyze, wxT("Analysis Mode"));

	toolBar->Realize();
	SetToolBar(toolBar);
}

void MainFrame::InitializeComponents(){

	m_channelsPanel = new DatalogChannelsPanel(this);
	m_channelsPanel->SetDatalogStore(&m_datalogStore);
	m_channelsPanel->SetAppOptions(&m_appOptions);
	m_channelsPanel->SetAppPrefs(&_appPrefs);

	_frameManager.AddPane(m_channelsPanel, wxAuiPaneInfo().Name(wxT(PANE_RUNTIME)).Caption(wxT(CAPTION_RUNTIME)).Center().Hide());

	m_analyzePanel = new wxPanel(this);
	_frameManager.AddPane(m_analyzePanel, wxAuiPaneInfo().Name(wxT(PANE_ANALYZE)).Caption(wxT(CAPTION_ANALYSIS)).Center().Hide());

	m_configPanel = new wxPanel(this);
	_frameManager.AddPane(m_configPanel, wxAuiPaneInfo().Name(wxT(PANE_CONFIGURATION)).Caption(wxT(CAPTION_CONFIG)).Center().Hide());

	m_scriptPanel = new ScriptPanel(this);
	_frameManager.AddPane(m_scriptPanel, wxAuiPaneInfo().Name(wxT(PANE_SCRIPT)).Caption(wxT(CAPTION_SCRIPT)).Center());

}


void MainFrame::OnHelpAbout(wxCommandEvent &event){

	wxString msg = wxString::Format("MegaJolt Lite Jr. Configurator %s\n\nhttp://www.autosportlabs.net\n\nCopyright � 2004-2008 Autosport Labs\n\n",RACE_ANALYZER_VERSION);
	wxMessageDialog dlg(this,msg, "About", wxOK);
	dlg.ShowModal();
}



void MainFrame::NotifyConfigChanged(){

	wxCommandEvent event( CONFIG_STALE_EVENT, CONFIG_STALE );
	event.SetEventObject(this);
	m_analyzePanel->AddPendingEvent(event);
	m_configPanel->AddPendingEvent(event);
	m_channelsPanel->AddPendingEvent(event);
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



bool MainFrame::QueryFileOverwrite(){

	wxMessageDialog dlg(this, "File Exists- Overwrite?","File exists", wxYES_NO | wxNO_DEFAULT);
	return ( wxID_YES == dlg.ShowModal() );
}


void MainFrame::OnAppOptions(wxCommandEvent& event){

	OptionsDialog optionsDialog;
	optionsDialog.SetAppOptions(&m_appOptions);
	optionsDialog.Create(this);

	if (optionsDialog.ShowModal() == wxID_OK) {
		m_appOptions.SaveAppOptions();
		m_raceAnalyzerComm.SetSerialPort(m_appOptions.GetSerialPort());
	}
}


bool MainFrame::QuerySaveModifications(){
	wxMessageDialog dlg(this, "Current Configuration has not been saved. Save Changes?","Configuration Modified", wxYES_NO | wxNO_DEFAULT);
	return ( wxID_YES == dlg.ShowModal() );
}


void MainFrame::OnNewRaceEvent(wxCommandEvent &event){

	wxString defaultDir = _appPrefs.GetLastConfigFileDirectory();
	wxString defaultFile = "";
	wxFileDialog fileDialog(this, "New Race Event", defaultDir, defaultFile, OPEN_RACE_EVENT_FILTER, wxSAVE);

	int result = fileDialog.ShowModal();

	if (wxID_OK == result){
		try{
			const wxString fileName = fileDialog.GetPath();
			if (wxFile::Exists(fileName)){
				wxMessageDialog dlg(this, wxString::Format("Race Event file already exists. Open this Event?"),"Open Existing Race Event", wxYES_DEFAULT | wxYES_NO);
				int result = dlg.ShowModal();
				if (result == wxYES){
					OpenRaceEvent(fileName);
				}
			}else{
				NewRaceEvent(fileName);
			}

			RaceEventLoaded();
		}
		catch(DatastoreException e){
			wxMessageDialog dlg(this, wxString::Format("Failed to Create Race Event:\n\n%s", e.GetMessage().ToAscii()), "Error Creating Race Event", wxOK | wxICON_HAND);
		}
		_appPrefs.SetLastConfigFileDirectory(fileDialog.GetDirectory());
	}
	//Set default values
	NotifyConfigChanged();
}

void MainFrame::OnOpenRaceEvent(wxCommandEvent& event){


	wxString defaultDir = _appPrefs.GetLastConfigFileDirectory();
	wxString defaultFile = "";
	wxFileDialog fileDialog(this, "Open Race Event", defaultDir, defaultFile, OPEN_RACE_EVENT_FILTER, wxOPEN | wxFILE_MUST_EXIST);

	int result = fileDialog.ShowModal();

	if (wxID_OK == result){
		try{
			const wxString fileName = fileDialog.GetPath();
			OpenRaceEvent(fileName);
			RaceEventLoaded();
		}
		catch(DatastoreException e){
			wxMessageDialog dlg(this, wxString::Format("Failed to open Race Event:\n\n%s", e.GetMessage().ToAscii()), "Error Opening", wxOK | wxICON_HAND);
			dlg.ShowModal();
			return;
		}
		_appPrefs.SetLastConfigFileDirectory(fileDialog.GetDirectory());
	}
}


void MainFrame::NewRaceEvent(wxString fileName){

	if (m_datalogStore.IsOpen()) m_datalogStore.Close();
	m_datalogStore.CreateNew(fileName);
}

void MainFrame::CloseRaceEvent(){

	if (m_datalogStore.IsOpen()){
		m_datalogStore.Close();
	}
}

void MainFrame::OpenRaceEvent(wxString fileName){

	m_datalogStore.Open(fileName);
	UpdateAnalyzerView();
}

void MainFrame::OnImportDatalog(wxCommandEvent& event){

	ImportDatalogWizard *wiz = new ImportDatalogWizard(this,ImportWizardParams(&_appPrefs,&m_appOptions,&m_datalogStore));

	wiz->ShowPage(wiz->GetFirstPage());
	wiz->Show(true);

/*	wxString defaultDir = _appPrefs.GetLastConfigFileDirectory();
	wxFileDialog fileDialog(this, "Import Datalog", defaultDir, "", LOGGING_FILE_FILTER, wxOPEN);

	int result = fileDialog.ShowModal();

	if (wxID_OK != result) return;


	try{
		const wxString fileName = fileDialog.GetPath();
		wxString name("Session");
		wxString notes("Notes");
		ImportDatalog(fileName, name, notes);
		UpdateAnalyzerView();
	}
	catch(DatastoreException e){
		wxMessageDialog dlg(this, wxString::Format("Failed to Import Datalog File:\n\n%s", e.GetMessage().ToAscii()), "Error Importing", wxOK | wxICON_HAND);
		dlg.ShowModal();
		return;
	}
	_appPrefs.SetLastConfigFileDirectory(fileDialog.GetDirectory());
	*/
}


void MainFrame::OnImportWizardFinished(wxWizardEvent &event){
	UpdateAnalyzerView();
}

void MainFrame::UpdateAnalyzerView(){

	INFO("Updating Analyzer view");
	UpdateDatalogSessions();
	INFO("Updating Analyzer view complete");
}

void MainFrame::UpdateDatalogSessions(){
	m_channelsPanel->UpdateDatalogSessions();
}

void MainFrame::UpdateAnalysis(){

/*	m_datalogData.Clear();

	wxArrayString channels;
	channels.Add("GPSVelocity");
	channels.Add("AccelX");
	channels.Add("AccelY");
	channels.Add("AccelZ");
	channels.Add("Yaw");

	int datalogId = 1;
	m_datalogStore.ReadDatalog(m_datalogData, datalogId, channels, 0);

	Range *speedRange = new Range(0,300,"KPh");
	int rangeId = m_analyzePanel->GetLineChart()->AddRange(speedRange);
	Series *gpsSpeed = new Series(&m_datalogData,0,rangeId,0,"GPS Speed",*wxGREEN);
	m_analyzePanel->GetLineChart()->AddSeries(gpsSpeed);

	Range *gForceRange = new Range(-2.0,2.0,"G");
	int gForceRangeId= m_analyzePanel->GetLineChart()->AddRange(gForceRange);

	Series *accelX = new Series(&m_datalogData, 1, gForceRangeId,0,"Accel X",*wxWHITE);
	m_analyzePanel->GetLineChart()->AddSeries(accelX);

	Series *accelY = new Series(&m_datalogData, 2, gForceRangeId,0,"Accel Y",*wxBLUE);
	m_analyzePanel->GetLineChart()->AddSeries(accelY);

	Series *accelZ = new Series(&m_datalogData, 3, gForceRangeId,0,"Accel Z",wxColor(255,255,0));
	m_analyzePanel->GetLineChart()->AddSeries(accelZ);

	Series *yaw = new Series(&m_datalogData,4,gForceRangeId,0,"Yaw",*wxRED);
	m_analyzePanel->GetLineChart()->AddSeries(yaw);
*/
}


void MainFrame::RaceEventLoaded(){
	UpdateCommControls();
	NotifyConfigChanged();
	SyncControls();
	UpdateConfigFileStatus();
	SetStatusMessage("Race Event Loaded");
}


void MainFrame::UpdateCommControls(){


}

void MainFrame::UpdateConfigFileStatus(){

	wxString windowTitle = wxString::Format(RACEANALYZER_WINDOW_TITLE,RACE_ANALYZER_VERSION);

	wxString fileName = m_datalogStore.GetFileName();
	wxString title = wxString::Format("%s - Race Event %s", windowTitle.ToAscii(), fileName.ToAscii());
	SetTitle(title);
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

	if (m_appTerminated) return;
	m_appTerminated = true;
	//Cancel exit if there are unsaved changes and the user opts to not save

	CloseRaceEvent();
	SaveCurrentPerspective();
	_appPrefs.SaveAppPrefs();

	Destroy();
}

void MainFrame::AddNewLineChart(DatalogChannelSelectionSet *selectionSet){


	LineChartPane *logViewer = new LineChartPane(this, -1);
	logViewer->SetChartParams(ChartParams(&_appPrefs,&m_appOptions,&m_datalogStore));
	logViewer->CreateChart(selectionSet);

	m_channelViews.Add(logViewer);
	wxString name = wxString::Format("lineChart_%lu", (unsigned long)m_channelViews.Count());
	wxString caption = wxString::Format("Line Chart");

	_frameManager.AddPane(logViewer,
			wxAuiPaneInfo().
			BestSize(100,50).
			MinSize(100,50).
			Name(name).
			Caption(caption).
			Bottom().
			Layer(1).
			Position(2).
			Show(true));

	_frameManager.Update();
}

void MainFrame::AddAnalogGauges(DatalogChannelSelectionSet *selectionSet){

	size_t selectionSetCount = selectionSet->Count();
	for (size_t i = 0; i < selectionSetCount; i++){
		DatalogChannelSelection &sel = (*selectionSet)[i];
		int datalogId = sel.datalogId;
		DatalogChannels channels;
		m_datalogStore.GetChannels(datalogId,channels);

		for (size_t c = 0; c < sel.channelIds.Count(); c++){
			int channelId = sel.channelIds[c];

			AnalogGaugePane *gaugePane = new AnalogGaugePane(this, -1);
			gaugePane->SetChartParams(ChartParams(&_appPrefs,&m_appOptions,&m_datalogStore));
			gaugePane->CreateGauge(datalogId,channelId);
			m_channelViews.Add(gaugePane);

			wxString name = wxString::Format("analogGauge_%lu", (unsigned long)m_channelViews.Count());
			wxString caption = wxString::Format("%s", channels[channelId].name.ToAscii());

			_frameManager.AddPane(gaugePane,
					wxAuiPaneInfo().
					BestSize(150,150).
					MinSize(150,150).
					Name(name).
					Caption(caption).
					Bottom().
					Layer(1).
					Position(2).
					Show(true));

			_frameManager.Update();
		}

	}
}

void MainFrame::AddDigitalGauges(DatalogChannelSelectionSet *selectionSet){

	size_t selectionSetCount = selectionSet->Count();
	for (size_t i = 0; i < selectionSetCount; i++){
		DatalogChannelSelection &sel = (*selectionSet)[i];
		int datalogId = sel.datalogId;
		DatalogChannels channels;
		m_datalogStore.GetChannels(datalogId,channels);

		for (size_t c = 0; c < sel.channelIds.Count(); c++){
			int channelId = sel.channelIds[c];

			DigitalGaugePane *gaugePane = new DigitalGaugePane(this, -1);
			gaugePane->SetChartParams(ChartParams(&_appPrefs,&m_appOptions,&m_datalogStore));
			gaugePane->CreateGauge(datalogId,channelId);
			m_channelViews.Add(gaugePane);

			wxString name = wxString::Format("digitalGauge_%lu", (unsigned long)m_channelViews.Count());
			wxString caption = wxString::Format("%s", channels[channelId].name.ToAscii());

			_frameManager.AddPane(gaugePane,
					wxAuiPaneInfo().
					BestSize(150,50).
					MinSize(150,50).
					Name(name).
					Caption(caption).
					Bottom().
					Layer(1).
					Position(2).
					Show(true));

			_frameManager.Update();
		}

	}
}

void MainFrame::AddGPSView(DatalogChannelSelectionSet *selectionSet){
	size_t selectionSetCount = selectionSet->Count();
	for (size_t i = 0; i < selectionSetCount; i++){
		DatalogChannelSelection &sel = (*selectionSet)[i];
		int datalogId = sel.datalogId;
		DatalogChannels channels;
		m_datalogStore.GetChannels(datalogId,channels);

		DatalogChannelTypes channelTypes;
		m_datalogStore.GetChannelTypes(channelTypes);

		int longitudeChannelId = -1;
		int latitudeChannelId = -1;

		wxArrayInt &channelIds = sel.channelIds;
		size_t selectedChannelsCount = channelIds.Count();
		for (size_t ci = 0; ci < selectedChannelsCount; ci++){
			int channelId = channelIds[ci];
			DatalogChannel &c = channels[channelId];
			DatalogChannelType &ct = channelTypes[c.typeId];
			if (ct == DatalogChannelSystemTypes::GetLongitudeChannelType()) longitudeChannelId = channelId;
			if (ct == DatalogChannelSystemTypes::GetLatitudeChannelType()) latitudeChannelId = channelId;
		}

		if (longitudeChannelId >= 0 && latitudeChannelId >= 0){

			GPSPane *gpsPane = new GPSPane(this, -1);

			gpsPane->SetChartParams(ChartParams(&_appPrefs,&m_appOptions,&m_datalogStore));
			gpsPane->CreateGPSView(datalogId,latitudeChannelId,longitudeChannelId);
			wxString name = wxString::Format("GPS_%lu", (unsigned long)m_channelViews.Count());
			wxString caption = wxString::Format("GPS %d", datalogId);

			_frameManager.AddPane(gpsPane,
					wxAuiPaneInfo().
					BestSize(400,400).
					MinSize(200,200).
					Name(name).
					Caption(caption).
					Bottom().
					Layer(1).
					Position(2).
					Show(true));

			_frameManager.Update();
		}
		else{
			wxMessageDialog dlg(this,"Please Select the Channels containing the GPS Latitude and Longitude information", "Error", wxOK);
			dlg.ShowModal();
		}
	}
}

void MainFrame::OnAddLineChart(wxCommandEvent &event){

	DatalogChannelSelectionSet *addData = (DatalogChannelSelectionSet *)event.GetClientData();
	AddNewLineChart(addData);
	delete addData;
}

void MainFrame::OnAddAnalogGauge(wxCommandEvent &event){
	DatalogChannelSelectionSet *addData = (DatalogChannelSelectionSet *)event.GetClientData();
	AddAnalogGauges(addData);
	delete addData;
}

void MainFrame::OnAddDigitalGauge(wxCommandEvent &event){
	DatalogChannelSelectionSet *addData = (DatalogChannelSelectionSet *)event.GetClientData();
	AddDigitalGauges(addData);
	delete addData;
}

void MainFrame::OnAddGPSView(wxCommandEvent &event){
	DatalogChannelSelectionSet *addData = (DatalogChannelSelectionSet *)event.GetClientData();
	AddGPSView(addData);
	delete addData;

}


BEGIN_EVENT_TABLE ( MainFrame, wxFrame )
  	EVT_CLOSE (MainFrame::OnExit)

    EVT_MENU(wxID_EXIT, MainFrame::OnFileExit)
    EVT_MENU(ID_OPTIONS, MainFrame::OnAppOptions)


    EVT_MENU(wxID_NEW, MainFrame::OnNewRaceEvent)
    EVT_MENU(wxID_OPEN,MainFrame::OnOpenRaceEvent)
    EVT_MENU(ID_IMPORT_DATALOG,MainFrame::OnImportDatalog)

    EVT_COMMAND( CONFIG_CHANGED, CONFIG_CHANGED_EVENT, MainFrame::OnConfigChanged )

	EVT_MENU( ID_CONFIG_MODE, MainFrame::OnConfigPerspective)
	EVT_MENU( ID_RUNTIME_MODE, MainFrame::OnRuntimePerspective)
	EVT_MENU( ID_ANALYZE_MODE, MainFrame::OnAnalyzePerspective)

	EVT_MENU( ID_HELP_ABOUT, MainFrame::OnHelpAbout)

	EVT_WIZARD_FINISHED(wxID_ANY, MainFrame::OnImportWizardFinished)


	EVT_MENU(ID_ADD_LINE_CHART, MainFrame::OnAddLineChart)

	EVT_COMMAND(ADD_NEW_LINE_CHART, ADD_NEW_LINE_CHART_EVENT, MainFrame::OnAddLineChart)
	EVT_COMMAND(ADD_NEW_ANALOG_GAUGE, ADD_NEW_ANALOG_GAUGE_EVENT, MainFrame::OnAddAnalogGauge)
	EVT_COMMAND(ADD_NEW_DIGITAL_GAUGE, ADD_NEW_DIGITAL_GAUGE_EVENT, MainFrame::OnAddDigitalGauge)
	EVT_COMMAND(ADD_NEW_GPS_VIEW, ADD_NEW_GPS_VIEW_EVENT, MainFrame::OnAddGPSView)


	//this must always be last
	EVT_MENU_RANGE(ID_PERSPECTIVES, ID_PERSPECTIVES + MAX_PERSPECTIVES, MainFrame::OnSwitchView)

END_EVENT_TABLE()



