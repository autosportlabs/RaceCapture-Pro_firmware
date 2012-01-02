/*
 * addChannelWizard.h
 *
 *  Created on: Oct 3, 2011
 *      Author: brent
 */

#ifndef ADDCHANNELWIZARD_H_
#define ADDCHANNELWIZARD_H_
#include "wx/wxprec.h"
#include "wx/wizard.h"
#include "wx/grid.h"
#include "appPrefs.h"
#include "appOptions.h"
#include "datalogStore.h"
#include "newChannelProcessor.h"
#include "wx/dynarray.h"



class ChannelProcessorDescription{

public:
	ChannelProcessorDescription(wxString name, wxString description, NewChannelProcessor processor, NewChannelOptionsPageFactory * wizardPageFactory);

	wxString m_name;
	wxString m_description;
	NewChannelProcessor m_processor;
	NewChannelOptionsPageFactory * m_wizardPageFactory;
};

WX_DECLARE_OBJARRAY(ChannelProcessorDescription,ChannelProcessorDescriptions);

class AddChannelWizardParams{

public:
	AddChannelWizardParams() :
		appPrefs(NULL),
		appOptions(NULL),
		datalogStore(NULL)

		{}

	AddChannelWizardParams(
			AppPrefs *prefs,
			AppOptions *options,
			DatalogStore *store
			):
		appPrefs(prefs),
		appOptions(options),
		datalogStore(store)
		{

		}

		AppPrefs 			*appPrefs;
		AppOptions			*appOptions;
		DatalogStore 		*datalogStore;
};

class NewChannelFinishPage : public wxWizardPageSimple{

public:
	NewChannelFinishPage(wxWizard *parent);

};

class NewChannelProgressPage : public wxWizardPageSimple{

public:
	NewChannelProgressPage(wxWizard *parent, AddChannelWizardParams *params);
	void OnWizardPageFinished(wxWizardEvent &event);
	void OnStartNewChannel(wxCommandEvent &event);
	void OnNewChannelProgress(wxCommandEvent &event);
	void OnNewChannelResult(wxCommandEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageChanged(wxWizardEvent &event);
	DECLARE_EVENT_TABLE()

private:
	AddChannelWizardParams *m_params;
};



class SelectNewChannelTypePage : public wxWizardPageSimple{

public:
	SelectNewChannelTypePage(wxWizard *parent, AddChannelWizardParams *params, ChannelProcessorDescriptions *processorDescriptions);
	wxGrid * CreateNewChannelsGrid();
	void UpdateUIState();
	void OnGridSelected(wxGridEvent &event);
	void AppendNewChannelType(wxGrid *grid, ChannelProcessorDescription &channelProcessor);
	void PopulateAvailableNewChannels(wxWizard *wizardParent, wxGrid *grid, ChannelProcessorDescriptions *processorDescriptions);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageChanged(wxWizardEvent &event);
    DECLARE_EVENT_TABLE()

private:
	AddChannelWizardParams	*m_params;
	int selectedChannelIndex;
	ChannelProcessorDescriptions *m_processorDescriptions;
	wxWizard *m_wizardParent;
};


class AddChannelWizard : public wxWizard{

public:

    AddChannelWizard(wxWindow *frame, AddChannelWizardParams params);
    wxWizardPage *GetFirstPage() const { return m_startPage; }
    void SetAddChannelOptionsPage(wxWizardPageSimple *page);

private:
    void initChannelProcessors();

    wxWizardPageSimple 	*m_startPage;
    wxWizardPageSimple *m_progressPage;
    AddChannelWizardParams	m_params;

    ChannelProcessorDescriptions m_channelProcessors;
};

DEFINE_EVENT_TYPE ( ADD_CHANNEL_PROGRESS_EVENT )
DEFINE_EVENT_TYPE ( ADD_CHANNEL_RESULT_EVENT )

#endif /* ADDCHANNELWIZARD_H_ */
