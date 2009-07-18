#ifndef __RACE_ANALYZER_H_
#define __RACE_ANALYZER_H_

#include "wx/wxprec.h"
#include <wx/thread.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/splash.h>
#include <wx/cmdline.h>
#include <wx/wizard.h>

#include "datalogStore.h"
#include "lineChartPane.h"
#include "datalogChannelsPanel.h"
#include "raceAnalyzerConfigBase.h"
#include "LCDWindow.h"
#include "AngularMeter.h"
#include "scriptPanel.h"
#include "comm.h"
#include "configData.h"
#include "appOptions.h"
#include "appPrefs.h"
#include "commonEvents.h"
#include "digitalRPMPanel.h"
#include "chartBase.h"

#include <wx/aui/aui.h>
#include <wx/aui/framemanager.h>


#define  OPEN_RACE_EVENT_FILTER 	"RaceAnalyzer Event Files (*.radb)|*.radb"
#define	RACEANALYZER_WINDOW_TITLE 	"RaceAnalyzer V%s"

#define PERSPECTIVE_CONFIG	"Configuration"
#define PERSPECTIVE_RUNTIME	"Runtime"
#define PERSPECTIVE_ANALYZE	"Analysis"
#define PERSPECTIVE_SCRIPT	"Scripting"
#define MAX_PERSPECTIVES	20


class MainFrame;

class RaceAnalyzerApp : public wxApp{
	  public:
	    virtual bool OnInit();
		bool OnCmdLineParsed(wxCmdLineParser& parser);
	    void OnInitCmdLine(wxCmdLineParser& parser);
	  private:
	  	MainFrame *_mainFrame;
  };


class MainFrame : public wxFrame
  {

  private:

	void InitComms();
  	void NotifyConfigChanged();
  	void UpdateConfigFileStatus();
	void InitializeComponents();
	void InitializeFrame();
	void InitializeMenus();
	void SetActivityMessage(const wxString& message);
	void SetStatusMessage(const wxString& message);
	void ClearActivityMessage();
	void ClearStatusMessage();
	void UpdateCommControls();
	void UpdateAnalyzerView();
	void UpdateDatalogSessions();
	void UpdateAnalysis();
	void ConfigModified();
	void TerminateApp();
	bool QuerySaveModifications();
	bool QueryFileOverwrite();
	void SwitchToPerspective(int id);
	void SaveCurrentPerspective();
	void CreateDefaultPerspectives();
	void CreateDefaultConfigPerspective();
	void CreateDefaultRuntimePerspective();
	void CreateDefaultAnalyzePerspective();
	void CreateDefaultScriptPerspective();
	void ShowSplashScreen();
	void SyncControls();

	void NewRaceEvent(wxString fileName);
	void CloseRaceEvent();

	void AddNewLineChart(DatalogChannelSelectionSet *selectionSet);

	//Main Panels

  	//events
  	void OnHelpAbout(wxCommandEvent &event);

  	void OnConfigChanged(wxCommandEvent &event);
  	void OnFileExit(wxCommandEvent &event);
  	void OnExit(wxCloseEvent& WXUNUSED(event));
  	void RaceEventLoaded();
  	void OnNewRaceEvent(wxCommandEvent &event);
  	void OnOpenRaceEvent(wxCommandEvent &event);

	void OnImportDatalog(wxCommandEvent &event);
	void OnAppOptions(wxCommandEvent &event);
    void OnSwitchView(wxCommandEvent &event);
    void OnConfigPerspective(wxCommandEvent &event);
    void OnRuntimePerspective(wxCommandEvent &event);
    void OnAnalyzePerspective(wxCommandEvent &event);
	void OnRestoreDefaultView(wxCommandEvent &event);

	void OnImportWizardFinished(wxWizardEvent &event);

	//chart events
	void OnAddLineChart(wxCommandEvent &event);

 public:


	void OpenRaceEvent(wxString fileName);
    MainFrame( const wxString &title, const wxPoint &pos, const wxSize &size );
	~MainFrame();

	//Data

	ConfigData 			m_currentConfigData;
	wxString 			m_currentConfigFileName;

	bool				m_appTerminated;

	int					m_activeConfig;

	RaceAnalyzerComm	m_raceAnalyzerComm;
	DatalogStore		m_datalogStore;

	DatalogStoreRows	m_datalogData;

	wxPanel				*m_configPanel;
	ScriptPanel			*m_scriptPanel;
	wxPanel				*m_analyzePanel;
	DatalogChannelsPanel *m_channelsPanel;


	RaceAnalyzerCharts	m_charts;

	//App Options and settings
	AppOptions			m_appOptions;

	AppPrefs			_appPrefs;

	//Frame Manager
	wxAuiManager		_frameManager;

	DECLARE_EVENT_TABLE()

  };


  enum{


	ID_OPTIONS = wxID_HIGHEST + 1,
	ID_GET_CONFIG,
	ID_WRITE_CONFIG,

	ID_CONFIG_MODE,
	ID_RUNTIME_MODE,
	ID_ANALYZE_MODE,

	ID_HELP_ABOUT,
	ID_IMPORT_DATALOG,

	ID_ADD_LINE_CHART,

	ID_RESTORE_DEFAULT_VIEWS,
	ID_PERSPECTIVES //this must be last

  };


#endif
