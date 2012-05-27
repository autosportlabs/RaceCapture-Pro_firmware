
#ifndef ACCELINPUTPANEL_H_
#define ACCELINPUTPANEL_H_

#include "wx/wxprec.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/channelConfigPanel.h"
#include "configuration/baseChannelConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"


class AccelInputPanel : public BaseChannelConfigPanel{

	public:
		AccelInputPanel();
		AccelInputPanel(wxWindow *parent,
					wxWindowID id = -1,
					RaceCaptureConfig *config = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "accelInputPanel"
					);

		~AccelInputPanel();
		wxString GetChannelListTitle();
		wxPanel * GetTopInnerPanel();
		wxPanel * GetBottomInnerPanel();
		ChannelConfig * GetChannelConfig(int i);
		int ChannelCount();
		void UpdatedExtendedFields();
		void UpdateExtendedChannelFields(int i);
		ChannelConfigExtraFields CreateExtendedChannelFields(int i);
		wxString GetChannelConfigPanelName(int index);
		wxString GetChannelLabel(int index);

	private:
		//event handlers
		void OnChannelMappingChanged(wxCommandEvent &event);
		void OnAccelZeroChanged(wxCommandEvent &event);
		void InitMappingCombo(wxComboBox *combo);

		wxComboBox *m_accelMapping[CONFIG_ACCEL_CHANNELS];
		SteppedSpinCtrl *m_accelZeroValue[CONFIG_ACCEL_CHANNELS];
	DECLARE_EVENT_TABLE()
};




#endif /* ACCELINPUTPANEL_H_ */
