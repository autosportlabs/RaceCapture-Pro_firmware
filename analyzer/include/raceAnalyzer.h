#ifndef __RACE_ANALYZER_H_
#define __RACE_ANALYZER_H_

#include <wx/wx.h>
#include <wx/thread.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/splash.h>
#include <wx/cmdline.h>

#include "raceAnalyzerConfigBase.h"
#include "LCDWindow.h"
#include "AngularMeter.h"
#include "StripChart.h"
#include "comm.h"
#include "configData.h"
#include "appOptions.h"
#include "appPrefs.h"
#include "commonEvents.h"
#include "digitalRPMPanel.h"

#include <wx/aui/aui.h>
#include <wx/aui/framemanager.h>


  #define  CONFIG_FILE_FILTER 	"RaceAnalyzer Configuration files (*.raz;)|*.raz"
  #define	MJLJ_WINDOW_TITLE 	"RaceAnalyzer V%s"

  #define PERSPECTIVE_CONFIG 	"Configuration"
  #define PERSPECTIVE_RUNTIME 	"Runtime"
  #define PERSPECTIVE_TUNING	"Tuning"
  #define MAX_PERSPECTIVES		20

  #define LOGGING_FILE_FILTER	"RaceAnalyzer Datalog File (*.csv)|*.csv"

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
  	void NotifyConfigChanged();
  	void LoadInitialConfig();
  	void UpdateConfigFileStatus();
	void InitializeComponents();
	void InitializeFrame();
	void InitializeMenus();
	void SetActivityMessage(const wxString& message);
	void SetStatusMessage(const wxString& message);
	void ClearActivityMessage();
	void ClearStatusMessage();
	void UpdateCommControls();
	void ConfigModified();
	void TerminateApp();
	bool QuerySaveModifications();
	bool QueryFileOverwrite();
	void SwitchToPerspective(int id);
	void SaveCurrentPerspective();
	void CreateDefaultPerspectives();
	void CreateDefaultConfigPerspective();
	void CreateDefaultRuntimePerspective();
	void CreateDefaultTuningPerspective();
	void ShowSplashScreen();
	void UpdateLoggingStatus();
	void SyncControls();
	//Data

	ConfigData 			_currentConfigData;
	wxString 			*_currentConfigFileName;
	bool				_configModified;
	bool				_staleFlashMemory;
	bool				_staleRAMMemory;

	bool				_appTerminated;

	int					_activeConfig;
	int					_cylinderCount;

	RaceAnalyzerComm			m_raceAnalyzerComm;
	wxMenuItem			*_startDatalogMenuItem;
	wxMenuItem			*_stopDatalogMenuItem;

	wxPanel				*m_runtimePanel;
	wxPanel				*m_tuningPanel;
	wxPanel				*m_configPanel;

	//App Options and settings
	AppOptions		_appOptions;

	AppPrefs		_appPrefs;

	//Frame Manager
	wxAuiManager		_frameManager;

	//Main Panels

  	//events
  	void OnHelpAbout(wxCommandEvent &event);
  	void OnQuickLogging(wxCommandEvent &event);
  	void OnStopLogging(wxCommandEvent &event);
  	void OnStartLogging(wxCommandEvent &event);
  	void OnOpenDatalog(wxCommandEvent &event);

  	void OnConfigChanged(wxCommandEvent &event);
  	void OnFileExit(wxCommandEvent &event);
  	void OnExit(wxCloseEvent& WXUNUSED(event));

  	void OnNewConfig(wxCommandEvent& event);
	void OnSaveCurrentConfig(wxCommandEvent& event);
	void OnSaveAsCurrentConfig(wxCommandEvent& event);
	void OnOpenConfig(wxCommandEvent& event);
	void OnAppOptions(wxCommandEvent& event);
    void OnSwitchView(wxCommandEvent& event);
    void OnConfigPerspective(wxCommandEvent& event);
    void OnRuntimePerspective(wxCommandEvent& event);
    void OnTuningPerspective(wxCommandEvent& event);
	void OnRestoreDefaultView(wxCommandEvent &event);


 public:

	void LoadConfigurationFile(const wxString fileName);
	void SaveCurrentConfig();
	void SaveAsCurrentConfig();

    MainFrame( const wxString &title, const wxPoint &pos, const wxSize &size );
	~MainFrame();

    DECLARE_EVENT_TABLE()

  };


  enum{

    ID_PERSPECTIVES			= wxID_HIGHEST + 1,

	ID_OPTIONS,
	ID_GET_CONFIG,
	ID_WRITE_CONFIG,

	ID_CONFIG_MODE,
	ID_RUNTIME_MODE,
	ID_TUNING_MODE,

	ID_HELP_ABOUT,

	ID_RESTORE_DEFAULT_VIEWS,


	ID_QUICKLOGGING,
	ID_STARTLOG,
	ID_STOPLOG,
	ID_OPENDATALOG
  };


#endif
