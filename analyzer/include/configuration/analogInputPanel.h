
#ifndef ANALOGINPUTPANEL_H_
#define ANALOGINPUTPANEL_H_

#include "wx/wxprec.h"
#include "wx/treebook.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/analogInputPanel.h"
#include "configuration/channelConfigPanel.h"
#include "configuration/baseChannelConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"

class AnalogInputPanel :  public BaseChannelConfigPanel{

	public:
		AnalogInputPanel();
		AnalogInputPanel(wxWindow *parent,
					wxWindowID id = -1,
					RaceCaptureConfig *config = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "analogInputPanel"
					);

		~AnalogInputPanel();
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

		//event handlers
	private:
		void OnPrecisionChanged(wxCommandEvent &event);
		void OnAdvancedSettings(wxCommandEvent &event);

		SteppedSpinCtrl *m_loggingPrecisionSpinner[CONFIG_ANALOG_CHANNELS];
	DECLARE_EVENT_TABLE()
};




#endif /* ANALOGINPUTPANEL_H_ */
