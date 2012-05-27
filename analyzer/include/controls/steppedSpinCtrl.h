
#ifndef STEPPEDSPINCTRL_H_
#define STEPPEDSPINCTRL_H_


#include <wx/wx.h>
#include <wx/spinctrl.h>

class SteppedSpinCtrl : public wxSpinCtrl{

	public:
	    SteppedSpinCtrl() : wxSpinCtrl() { }

	    SteppedSpinCtrl(wxWindow *parent,
	               wxWindowID id = wxID_ANY,
	               const wxString& value = wxEmptyString,
	               const wxPoint& pos = wxDefaultPosition,
	               const wxSize& size = wxDefaultSize,
	               long style = wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB,
	               int min = 0, int max = 100, int initial = 0,
	               const wxString& name = _T("RPMSpinCtrl"))
	        : wxSpinCtrl(parent, id, value, pos, size, style, min, max, initial, name),
	        _stepValue(1) { }

	    bool Create(wxWindow *parent,
	                wxWindowID id = wxID_ANY,
	                const wxString& value = wxEmptyString,
	                const wxPoint& pos = wxDefaultPosition,
	                const wxSize& size = wxDefaultSize,
	                long style = wxSP_ARROW_KEYS,
	                int min = 0,
	                int max = 100,
	                int initial = 0,
	                const wxString& name = _T("RPMSpinCtrl")){

			         return wxSpinCtrl::Create(parent,
			         					id,
			         					value,
			         					pos,
			         					size,
			         					style,
			         					min,
			         					max,
			         					initial,
			         					name);
	                }
            void SetStepValue(int stepValue);
            int	GetStepValue();


		protected:
		    // the handler for wxSpinButton events
    		void OnSpinChange(wxSpinEvent& event);
    		void OnFocusChange(wxFocusEvent& event);
			void OnTextChange(wxCommandEvent &event);
		private:
			int _stepValue;
			void CheckRange();
		    DECLARE_EVENT_TABLE()


};

#endif /* STEPPEDSPINCTRL_H_ */
