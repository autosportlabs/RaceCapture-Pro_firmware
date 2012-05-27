
#ifndef PULSEINPUTPANEL_H_
#define PULSEINPUTPANEL_H_

#include "wx/wxprec.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/channelConfigPanel.h"
#include "configuration/baseChannelConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"


class PulseInputPanel : public BaseChannelConfigPanel{

	public:
		PulseInputPanel();
		PulseInputPanel(wxWindow *parent,
					wxWindowID id = -1,
					RaceCaptureConfig *config = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "timerInputPanel"
					);

		~PulseInputPanel();
		wxPanel * GetTopInnerPanel();
		wxPanel * GetBottomInnerPanel();
		ChannelConfig * GetChannelConfig(int i);
		int ChannelCount();
		void UpdatedExtendedFields();
		void UpdateExtendedChannelFields(int i);
		ChannelConfigExtraFields CreateExtendedChannelFields(int i);
		wxString GetChannelConfigPanelName(int index);
		wxString GetChannelLabel(int index);
		wxString GetChannelListTitle();

		void OnPrecisionChanged(wxCommandEvent &event);
		void OnAdvancedSettings(wxCommandEvent &event);

		//event handlers
	private:
		const wxString GetScalingFieldName(int i);

	DECLARE_EVENT_TABLE()
};




#endif /* PULSEINPUTPANEL_H_ */
