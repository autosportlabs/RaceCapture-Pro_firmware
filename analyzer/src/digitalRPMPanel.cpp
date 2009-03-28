
#include "digitalRPMPanel.h"

DigitalRPMPanel::DigitalRPMPanel() : wxPanel()
{
	InitComponents();
}

DigitalRPMPanel::DigitalRPMPanel(wxWindow *parent,
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

DigitalRPMPanel::~DigitalRPMPanel(){
	
}

void DigitalRPMPanel::InitComponents(){
	
	wxFont meterFont;
	meterFont.SetPointSize(16);
	
	wxFlexGridSizer *sizer = new wxFlexGridSizer(2,1,3,3);
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(0);
	
	_rpmLCD = new LCDDisplay(this);
	_rpmLCD->SetNumberDigits(4);
	_rpmLCD->SetValue("");
	_rpmLCD->SetBestFittingSize(wxSize(300,100));
	_rpmLCD->SetMinSize(wxSize(60,25));
	_rpmLCD->SetLightColour(wxColor(255,255,0));
	_rpmLCD->SetGrayColour(wxColor(40,40,0));
	
	sizer->Add(_rpmLCD,1,wxEXPAND);
	this->SetSizer(sizer);
}

void DigitalRPMPanel::SetValue(int value){

	_rpmLCD->SetValue(wxString::Format("%i",value));
}

void DigitalRPMPanel::SetAccel(int accel){
	
	
}

void DigitalRPMPanel::InitOptions(){
	
}

BEGIN_EVENT_TABLE ( DigitalRPMPanel, wxPanel )
END_EVENT_TABLE()