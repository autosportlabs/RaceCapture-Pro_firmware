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
#import "appPrefs.h"
#import "appOptions.h"
#import "datalogStore.h"

class ImportWizardParams{

public:
	ImportWizardParams() :
		appPrefs(NULL),
		appOptions(NULL),
		datalogStore(NULL),
		datalogFilePath(""),
		datalogName(""),
		datalogDesc("")
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
		datalogDesc(desc)
		{}

	AppPrefs 			*appPrefs;
	AppOptions			*appOptions;
	DatalogStore 		*datalogStore;
	wxString			datalogFilePath;
	wxString			datalogName;
	wxString			datalogDesc;
	DatalogChannels		datalogChannels;
	DatalogChannelTypes datalogChannelTypes;
};

class DatalogFileSelectPage : public wxWizardPageSimple{

public:
	DatalogFileSelectPage(wxWizard *parent, ImportWizardParams *params);
	void OnDatalogPathTextChange(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
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
	wxString GetDefaultDatalogName();
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
	void PopulateChannels();
	void RefreshChannelGrid();

	DECLARE_EVENT_TABLE()

private:
	ImportWizardParams 	*m_params;
	wxGrid				*m_channelMapGrid;
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
	ID_IMPORT_WIZ_CHANNEL_MAP_GRID

};


#endif /* IMPORTWIZARDDIALOG_H_ */
