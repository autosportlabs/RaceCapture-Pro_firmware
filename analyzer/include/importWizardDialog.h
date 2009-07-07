/*
 * importWizardDialog.h
 *
 *  Created on: Jun 22, 2009
 *      Author: brent
 */

#ifndef IMPORTWIZARDDIALOG_H_
#define IMPORTWIZARDDIALOG_H_
#include "wx/wxprec.h"
#include "wx/wizard.h"
#include "wx/grid.h"
#include "wx/gauge.h"
#include "wx/thread.h"
#include "appPrefs.h"
#include "appOptions.h"
#include "datalogStore.h"
#include "exceptions.h"

class ImportWizardParams{

public:
	ImportWizardParams() :
		appPrefs(NULL),
		appOptions(NULL),
		datalogStore(NULL),
		datalogFilePath(""),
		datalogName(GetDefaultDatalogName()),
		datalogDesc(""),
		datalogImported(false)
		{}

	ImportWizardParams(
			AppPrefs *prefs,
			AppOptions *options,
			DatalogStore *store,
			wxString filePath = "",
			wxString name = "",
			wxString desc = ""):
		appPrefs(prefs),
		appOptions(options),
		datalogStore(store),
		datalogFilePath(filePath),
		datalogName(name),
		datalogDesc(desc),
		datalogImported(false)
		{
				if(0 == name.Length()) datalogName = GetDefaultDatalogName();
		}

	AppPrefs 			*appPrefs;
	AppOptions			*appOptions;
	DatalogStore 		*datalogStore;
	wxString			datalogFilePath;
	wxString			datalogName;
	wxString			datalogDesc;
	DatalogChannels		datalogChannels;
	DatalogChannelTypes datalogChannelTypes;
	bool				datalogImported;

	wxString GetDefaultDatalogName();
};

class ImporterThread : public wxThread , public DatalogImportProgressListener{

public:
	ImporterThread();
	void SetParams(ImportWizardParams *params, wxWindow *owner);
	virtual wxThread::ExitCode Entry();
	virtual void UpdateProgress(int completePercent);

private:

	ImportWizardParams *m_params;
	wxWindow *m_owner;
};

class DatalogFileSelectPage : public wxWizardPageSimple{

public:
	DatalogFileSelectPage(wxWizard *parent, ImportWizardParams *params);
	void OnDatalogPathTextChange(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageChanged(wxWizardEvent &event);
	wxString GetDatalogFilePath();
    void SetDatalogFilePath(wxString filePath);
    bool IsFileValid(wxString filePath);
    void UpdateUIState();
    DECLARE_EVENT_TABLE()

private:
	ImportWizardParams 	*m_params;
	wxTextCtrl 			*m_filePath;
    wxStaticText		*m_helpText;
    wxColor				m_defaultColor;
};

class DatalogInfoPage : public wxWizardPageSimple{

public:
	DatalogInfoPage(wxWizard *parent, ImportWizardParams *params);
    void UpdateUIState();
	void OnNameChange(wxCommandEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageChanged(wxWizardEvent &event);
    DECLARE_EVENT_TABLE()

private:
	ImportWizardParams 	*m_params;
	wxTextCtrl			*m_name;
	wxTextCtrl 			*m_description;
	wxColor				m_defaultColor;

};

class MapDatalogChannelsPage : public wxWizardPageSimple{

public:
	MapDatalogChannelsPage(wxWizard *parent, ImportWizardParams *params);
	void OnWizardPageChanged(wxWizardEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
	void PopulateChannels();
	void AddExistingChannels(
			wxArrayString &headers,
			wxArrayString &remainingHeaders,
			DatalogChannels &channels,
			DatalogChannelTypes &channelTypes,
			DatalogChannels &existingChannels,
			DatalogChannelTypes &existingChannelTypes
			);

	void RefreshChannelGrid();
	void UpdateUIState();
	DECLARE_EVENT_TABLE()

private:
	ImportWizardParams 	*m_params;
	wxGrid				*m_channelMapGrid;
};


class DatalogImporterPage : public wxWizardPageSimple{

public:
	DatalogImporterPage(wxWizard *parent, ImportWizardParams *params);
	void OnWizardPageChanged(wxWizardEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageFinished(wxWizardEvent &event);
	void DoImportDatalog();
	void OnStartImport(wxCommandEvent &event);
	void OnImportProgress(wxCommandEvent &event);
	void OnImportResult(wxCommandEvent &event);
	void UpdateUIState();

	DECLARE_EVENT_TABLE()

private:
	ImportWizardParams *m_params;
	wxGauge *m_progressGauge;
	wxStaticText *m_progressMessage;
	wxStaticText *m_statusMessage;
	ImporterThread *m_importerThread;
	bool	m_importing;

};

class ImportDatalogWizard : public wxWizard{

public:
    ImportDatalogWizard(wxFrame *frame, ImportWizardParams params);

    wxWizardPage *GetFirstPage() const { return m_page1; }


private:
    wxWizardPageSimple 	*m_page1;
    ImportWizardParams	m_params;
};


enum{
	ID_IMPORT_WIZ_BROWSE_BUTTON = wxID_HIGHEST + 1,
	ID_IMPORT_WIZ_PATH_TEXT_CTRL,
	ID_IMPORT_WIZ_DATALOG_FILE_HELP_TEXT,
	ID_IMPORT_WIZ_DATALOG_NAME_TEXT,
	ID_IMPORT_WIZ_CHANNEL_MAP_GRID,
	ID_IMPORT_WIZ_IMPORT_PROGRESS,
	ID_IMPORT_WIZ_START_IMPORT,
	ID_IMPORT_WIZ_IMPORT_RESULT

};

DECLARE_EVENT_TYPE ( IMPORT_PROGRESS_EVENT, -1 )
DECLARE_EVENT_TYPE ( IMPORT_RESULT_EVENT, -1 )

#endif /* IMPORTWIZARDDIALOG_H_ */
