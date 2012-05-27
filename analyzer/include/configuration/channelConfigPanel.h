#ifndef CHANNELCONFIGPANEL_H_
#define CHANNELCONFIGPANEL_H_

#include "wx/wxprec.h"
#include "wx/dynarray.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "controls/steppedSpinCtrl.h"
#include "commonEvents.h"
#include "comm.h"

class ChannelConfigExtraField{
public:
	wxString header;
	wxWindow *control;
};

WX_DECLARE_OBJARRAY(ChannelConfigExtraField,ChannelConfigExtraFields);


class ChannelConfigPanel : public wxPanel{

	public:

		ChannelConfigPanel();
		ChannelConfigPanel(wxWindow *parent,
					ChannelConfigExtraFields extraFields,
					wxWindowID id = -1,
					bool showHeaders = false,
					ChannelConfig *channelConfig = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "channelConfigPanel"
					);


		~ChannelConfigPanel();

		void OnConfigUpdated();

	private:
		//event handlers
		void OnLabelChanged(wxCommandEvent &event);
		void OnUnitsChanged(wxCommandEvent &event);
		void OnSampleRateChanged(wxCommandEvent &event);

		//controls
		wxTextCtrl *m_channelLabel;
		wxTextCtrl *m_channelUnits;
		wxComboBox *m_channelSampleRate;

		int MapSampleRateToCombo(int sampleRate);
		sample_rate_t MapSampleRateFromCombo(int index);
		void SetChannelConfig(ChannelConfig *config);
		void InitComponents(bool showHeaders, ChannelConfigExtraFields &extraFields);
		void InitSampleRateCombo(wxComboBox *combo);

		ChannelConfig *m_channelConfig;


	DECLARE_EVENT_TABLE()
};


#endif /* CHANNELCONFIGPANEL_H_ */
