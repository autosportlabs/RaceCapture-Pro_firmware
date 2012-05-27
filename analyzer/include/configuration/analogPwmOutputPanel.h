
#ifndef ANALOGPULSEOUTPUTPANEL_H_
#define ANALOGPULSEOUTPUTPANEL_H_

#include "wx/wxprec.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/channelConfigPanel.h"
#include "configuration/baseChannelConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"


class AnalogPulseOutputPanel : public BaseChannelConfigPanel{

	public:
		AnalogPulseOutputPanel();
		AnalogPulseOutputPanel(wxWindow *parent,
					wxWindowID id = -1,
					RaceCaptureConfig *config = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "analogPulseOutputPanel"
					);

		~AnalogPulseOutputPanel();
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
	private:
		//event handlers
		void OnPrecisionChanged(wxCommandEvent &event);
		void OnModeChanged(wxCommandEvent &event);
		void OnAdvancedSettings(wxCommandEvent &event);

		void InitModeCombo(wxComboBox *modeCombo);

		SteppedSpinCtrl *m_loggingPrecisionSpinner[CONFIG_ANALOG_PULSE_CHANNELS];
		wxComboBox *m_modeCombo[CONFIG_ANALOG_PULSE_CHANNELS];
	DECLARE_EVENT_TABLE()
};




#endif /* ANALOGPULSEOUTPUTPANEL_H_ */
