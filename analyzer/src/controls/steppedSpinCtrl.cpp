#include "controls/steppedSpinCtrl.h"

void SteppedSpinCtrl::OnSpinChange(wxSpinEvent& event){

		CheckRange();
		//wxSpinCtrl::OnSpinChange(event);
}

void SteppedSpinCtrl::OnFocusChange(wxFocusEvent& event){
	CheckRange();
	}

void SteppedSpinCtrl::SetStepValue(int stepValue){
		_stepValue = stepValue;
		CheckRange();
}

int SteppedSpinCtrl::GetStepValue(){
	return _stepValue;
}

void SteppedSpinCtrl::CheckRange(){
	int value = GetValue();
	int remainder = value % _stepValue;

	if (0 != remainder){
		if (remainder > _stepValue / 2){
			value -=remainder;
		}
		else if (remainder < _stepValue / 2){
			value += (_stepValue - remainder);
		}
	}
	SetValue(value);
}

void SteppedSpinCtrl::OnTextChange(wxCommandEvent &event){
	if (0 == GetValue() % _stepValue){
		event.Skip(true);
	}
}

BEGIN_EVENT_TABLE(SteppedSpinCtrl, wxSpinCtrl)
    EVT_SPIN(wxID_ANY, SteppedSpinCtrl::OnSpinChange)
    EVT_KILL_FOCUS(SteppedSpinCtrl::OnFocusChange)
    EVT_TEXT(-1,SteppedSpinCtrl::OnTextChange)
END_EVENT_TABLE()
