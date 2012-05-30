#ifndef GPSCONFIGPANEL_H_
#define GPSCONFIGPANEL_H_

#include "wx/wxprec.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/channelConfigPanel.h"
#include "configuration/baseChannelConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"



class GpsConfigPanel : public BaseChannelConfigPanel{

	public:
		GpsConfigPanel();
		GpsConfigPanel(wxWindow *parent,
					wxWindowID id = -1,
					RaceCaptureConfig *config = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "GpsConfigPanel"
					);

		~GpsConfigPanel();
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

		wxCheckBox * m_gpsInstalledCheckBox;
		wxTextCtrl * m_startFinishLatitudeTextCtrl;
		wxTextCtrl * m_startFinishLongitudeTextCtrl;
		wxTextCtrl * m_startFinishTargetRadius;

	private:
		//event handlers
		void OnGpsInstalledChanged(wxCommandEvent &event);
		void OnStartFinishLatitudeChanged(wxCommandEvent &event);
		void OnStartFinishLongitudeChanged(wxCommandEvent &event);
		void OnStartFinishRadiusChanged(wxCommandEvent &event);

		wxPanel * m_gpsOptionsPanel;

	DECLARE_EVENT_TABLE()
};


#endif /* GPSCONFIGPANEL_H_ */
