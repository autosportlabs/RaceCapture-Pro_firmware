
#ifndef GPIOPANEL_H_
#define GPIOPANEL_H_

#include "wx/wxprec.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/channelConfigPanel.h"
#include "configuration/baseChannelConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"


class GpioPanel : public BaseChannelConfigPanel{

	public:
		GpioPanel();
		GpioPanel(wxWindow *parent,
					wxWindowID id = -1,
					RaceCaptureConfig *config = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "gpioPanel"
					);

		~GpioPanel();
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
		wxString GetModeFieldName(int index);
		void InitModeCombo(wxComboBox *modeCombo);

	DECLARE_EVENT_TABLE()
};




#endif /* GPIOPANEL_H_ */
