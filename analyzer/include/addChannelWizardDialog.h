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
#include "wx/dynarray.h"
#include "raceCapture/raceCaptureConfig.h"
#include "channelViews.h"


class AddChannelWizardParams{

public:
	AddChannelWizardParams(
			AppPrefs *prefs,
			AppOptions *options,
			DatalogStore *store,
			RaceCaptureConfig *config
			):
		appPrefs(prefs),
		appOptions(options),
		datalogStore(store),
		raceCaptureConfig(config)
		{

		}

		AppPrefs 			*appPrefs;
		AppOptions			*appOptions;
		DatalogStore 		*datalogStore;
		RaceCaptureConfig	*raceCaptureConfig;
};

class NewChannelFinishPage : public wxWizardPageSimple{

public:
	NewChannelFinishPage(wxWizard *parent);
};

class ChannelSelectorPage : public wxWizardPageSimple{

public:
	ChannelSelectorPage(wxWizard *parent, AddChannelWizardParams *params);
	void OnWizardPageFinished(wxWizardEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageChanged(wxWizardEvent &event);
	wxArrayString GetSelectedChannels();
private:
	void AppendChannelItem(wxGrid *grid, ChannelConfig *channelConfig);
	void PopulateAvailableChannels(wxGrid *grid, RaceCaptureConfig *config);
	wxGrid * CreateChannelsGrid();
	DECLARE_EVENT_TABLE()

private:
	AddChannelWizardParams *m_params;
	wxGrid * m_channelsGrid;
};



class SelectChannelViewPage : public wxWizardPageSimple{

public:
	SelectChannelViewPage(wxWizard *parent, AddChannelWizardParams *params, ChannelViews *channelViews);
	wxGrid * CreateViewSelectorGrid();
	void UpdateUIState();
	void OnGridSelected(wxGridEvent &event);
	void AppendNewChannelView(wxGrid *grid, ChannelView &channelProcessor);
	void PopulateNewChannelViews(wxWizard *wizardParent, wxGrid *grid, ChannelViews *channelViews);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageChanged(wxWizardEvent &event);
	ChannelViews GetSelectedChannelViews();
    DECLARE_EVENT_TABLE()

private:
	AddChannelWizardParams	*m_params;
	int selectedChannelIndex;
	ChannelViews *m_channelViews;
	wxWizard *m_wizardParent;
	wxGrid *m_channelViewSelectorGrid;
};


class AddChannelWizard : public wxWizard{

public:

    AddChannelWizard(wxWindow *frame, AddChannelWizardParams params);
    wxWizardPage *GetFirstPage() const { return m_viewSelectorPage; }

private:
    void InitChannelViews();
	void OnWizardFinished(wxWizardEvent &event);

    SelectChannelViewPage *m_viewSelectorPage;

    ChannelSelectorPage *m_channelSelectorPage;

    NewChannelFinishPage *m_finishPage;

    AddChannelWizardParams	m_params;
    ChannelViews m_channelViews;
    DECLARE_EVENT_TABLE()
};

DEFINE_EVENT_TYPE ( ADD_CHANNEL_PROGRESS_EVENT )
DEFINE_EVENT_TYPE ( ADD_CHANNEL_RESULT_EVENT )

#endif /* ADDCHANNELWIZARD_H_ */
