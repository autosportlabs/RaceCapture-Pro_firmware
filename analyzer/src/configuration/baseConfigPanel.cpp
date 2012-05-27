#include "configuration/baseConfigPanel.h"

BaseConfigPanel::BaseConfigPanel() : wxPanel()
{
}

BaseConfigPanel::BaseConfigPanel(wxWindow *parent,
			wxWindowID id,
			RaceCaptureConfig *config,
			const wxPoint &pos,
			const wxSize &size,
			long style,
			const wxString &name
			)
			: wxPanel(	parent,
						id,
						pos,
						size,
						style,
						name)
{
	m_raceCaptureConfig = config;
}

BaseConfigPanel::~BaseConfigPanel(){

}

void BaseConfigPanel::SetComm(RaceAnalyzerComm *comm){
	m_comm = comm;
}

BEGIN_EVENT_TABLE ( BaseConfigPanel, wxPanel )
END_EVENT_TABLE()
