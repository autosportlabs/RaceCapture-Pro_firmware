/*
 * scriptPanel.cpp
 *
 *  Created on: Apr 30, 2009
 *      Author: brent
 */
#include "scriptPanel.h"

ScriptPanel::ScriptPanel() : wxPanel()
{
	InitComponents();
}

ScriptPanel::ScriptPanel(wxWindow *parent,
			wxWindowID id,
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
	InitComponents();
}

ScriptPanel::~ScriptPanel(){

}

void ScriptPanel::InitComponents(){


	wxFlexGridSizer *sizer = new wxFlexGridSizer(2,1,3,3);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);



	m_scriptCtrl = new wxTextCtrl(this,ID_SCRIPT_WINDOW,"", wxDefaultPosition, wxDefaultSize,wxTE_MULTILINE | wxTE_PROCESS_TAB);

	sizer->Add(m_scriptCtrl,1,wxEXPAND);

	wxButton *readButton = new wxButton(this,ID_BUTTON_READ,"Read");
	wxButton *writeButton = new wxButton(this, ID_BUTTON_WRITE,"Write");
	wxButton *runButton = new wxButton(this, ID_BUTTON_RUN,"Run");

	wxGridSizer *buttonBarSizer = new wxGridSizer(1,2,3,3);

	wxBoxSizer *leftButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *rightButtonSizer = new wxBoxSizer(wxHORIZONTAL);

	leftButtonSizer->Add(readButton,0,wxALIGN_LEFT | wxALL);
	leftButtonSizer->Add(writeButton,0,wxALIGN_RIGHT | wxALL);
	rightButtonSizer->AddStretchSpacer(1);
	rightButtonSizer->Add(runButton,0,wxALIGN_RIGHT | wxALL);

	buttonBarSizer->Add(leftButtonSizer,1,wxEXPAND | wxALL);
	buttonBarSizer->Add(rightButtonSizer,1,wxEXPAND | wxALL);

	sizer->Add(buttonBarSizer,1,wxEXPAND);
	this->SetSizer(sizer);
}


void ScriptPanel::InitOptions(){

}

void ScriptPanel::SetComm(RaceAnalyzerComm *comm){
	m_comm = comm;
}

void ScriptPanel::OnReadScript(wxCommandEvent &event){
	try{
		wxString script = m_comm->readScript();
		m_scriptCtrl->SetValue(script);
	}
	catch(CommException e){
		wxLogMessage("Error reading script: %s", e.GetErrorMessage().ToAscii());
	}
}

void ScriptPanel::OnWriteScript(wxCommandEvent &event){

	try{
		wxString script = m_scriptCtrl->GetValue();
		m_comm->writeScript(script);
	}
	catch(CommException e){
		wxLogMessage("Error writing script: %s",e.GetErrorMessage().ToAscii());
	}
}

void ScriptPanel::OnRunScript(wxCommandEvent &event){
	m_comm->reloadScript();
}


BEGIN_EVENT_TABLE ( ScriptPanel, wxPanel )
	EVT_BUTTON(ID_BUTTON_READ,ScriptPanel::OnReadScript)
	EVT_BUTTON(ID_BUTTON_WRITE,ScriptPanel::OnWriteScript)
	EVT_BUTTON(ID_BUTTON_RUN,ScriptPanel::OnRunScript)

END_EVENT_TABLE()
