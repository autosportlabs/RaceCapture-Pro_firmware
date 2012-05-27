
#ifndef BASECHANNELCONFIGPANEL_H_
#define BASECHANNELCONFIGPANEL_H_

#include "wx/wxprec.h"
#include "raceAnalyzerConfigBase.h"
#include "raceCapture/raceCaptureConfig.h"
#include "configuration/baseConfigPanel.h"
#include "configuration/channelConfigPanel.h"
#include "commonEvents.h"
#include "comm.h"

class BaseChannelConfigPanel : public BaseConfigPanel{

	public:
		BaseChannelConfigPanel();
		BaseChannelConfigPanel(wxWindow *parent,
					wxWindowID id = -1,
					RaceCaptureConfig *config = NULL,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "baseChannelConfigPanel"
					);

		~BaseChannelConfigPanel();

		void SetComm(RaceAnalyzerComm *comm);

		void OnConfigUpdated();
		virtual wxPanel * GetTopInnerPanel() = 0;
		virtual wxPanel * GetBottomInnerPanel() = 0;
		virtual wxString GetChannelListTitle() = 0;
		virtual wxString GetChannelLabel(int index) = 0;
		virtual int ChannelCount() = 0;
		virtual ChannelConfigExtraFields CreateExtendedChannelFields(int i) = 0;
		virtual void UpdatedExtendedFields() = 0;
		virtual void UpdateExtendedChannelFields(int i) = 0;
		virtual wxString GetChannelConfigPanelName(int i) = 0;
		virtual ChannelConfig * GetChannelConfig(int i) = 0;

		wxTextCtrl * FindTextCtrl(wxString name);
		wxComboBox * FindComboBoxCtrl(wxString name);
		wxCheckBox * FindCheckBoxCtrl(wxString name);
		void SetTextCtrlValue(const wxString &name, float value);
		void SetCheckBoxValue(const wxString &name, bool value);

		void InitComponents();
		//event handlers

	protected:

	private:
		ChannelConfigPanel * FindChannelConfigPanel(int index);
		ChannelConfigPanel * CreateNewChannelConfigPanel(int index,bool showHeaders, ChannelConfigExtraFields &extraFields, ChannelConfig *channelConfig);

		RaceAnalyzerComm *m_comm;

	DECLARE_EVENT_TABLE()
};




#endif /* BASECHANNELCONFIGPANEL_H_ */
