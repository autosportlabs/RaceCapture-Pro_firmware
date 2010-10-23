/*
 * scriptPanel.h
 *
 *  Created on: Apr 30, 2009
 *      Author: brent
 */

#ifndef SCRIPTPANEL_H_
#define SCRIPTPANEL_H_

#include "wx/wxprec.h"
#include "raceAnalyzerConfigBase.h"
#include "commonEvents.h"
#include "comm.h"

class ScriptPanel : public wxPanel{

	public:
		ScriptPanel();
		ScriptPanel(wxWindow *parent,
					wxWindowID id = -1,
					const wxPoint &pos = wxDefaultPosition,
					const wxSize &size = wxDefaultSize,
					long style = wxTAB_TRAVERSAL,
					const wxString &name = "panel"
					);


		~ScriptPanel();

		void SetComm(RaceAnalyzerComm *comm);

		//event handlers
	private:

		void OnReadScript(wxCommandEvent &event);
		void OnWriteScript(wxCommandEvent &event);
		void OnRunScript(wxCommandEvent &event);
		void InitComponents();
		void InitOptions();

		wxTextCtrl *m_scriptCtrl;

		RaceAnalyzerComm *m_comm;

		enum{
			ID_SCRIPT_WINDOW = wxID_HIGHEST + 10000,
			ID_BUTTON_READ,
			ID_BUTTON_WRITE,
			ID_BUTTON_RUN
		};

	DECLARE_EVENT_TABLE()
};




#endif /* SCRIPTPANEL_H_ */
