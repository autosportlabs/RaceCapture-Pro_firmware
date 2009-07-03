#ifndef OPTIONSDIALOG_H_
#define OPTIONSDIALOG_H_

#include "wx/wxprec.h"
#include "wx/valgen.h"
#include "appOptions.h"

  class OptionsDialog : public wxDialog
  {
  	DECLARE_CLASS( OptionsDialog )
  	DECLARE_EVENT_TABLE();

  	public:
  		OptionsDialog();

  		OptionsDialog(wxWindow* parent,
  			wxWindowID id = wxID_ANY,
  			const wxString& caption = wxT("Options"),
  			const wxPoint& pos = wxDefaultPosition,
  			const wxSize& size = wxDefaultSize,
  			long style = wxCAPTION | wxSYSTEM_MENU );

  		bool Create (wxWindow* parent,
  			wxWindowID id = wxID_ANY,
  			const wxString& caption = wxT("Options"),
  			const wxPoint& pos = wxDefaultPosition,
  			const wxSize& size = wxDefaultSize,
  			long style = wxCAPTION | wxSYSTEM_MENU );

  		AppOptions * GetAppOptions();
  		void SetAppOptions(AppOptions *appOptions);

  		void CreateControls();

  	private:
		void UpdateControlStates();

  		AppOptions *_options;

  };


class COMPortValidator : public wxValidator{

	public:
		COMPortValidator();
		COMPortValidator(AppOptions *appOptions);
		void SetAppOptions(AppOptions *appOptions);
		wxObject* Clone() const;
		bool Validate(wxWindow* parent);
		bool TransferFromWindow();
		bool TransferToWindow();


	private:
		AppOptions* _appOptions;
};


#endif /*OPTIONSDIALOG_H_*/
