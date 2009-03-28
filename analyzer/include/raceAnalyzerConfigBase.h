#ifndef RACEANALYZERCONFIGBASE_H_
#define RACEANALYZERCONFIGBASE_H_

#define		MJLJ_CONFIG_VERSION			"0.0.1"
#define 	MJLJ_CONFIG_APP_NAME 	"Autosport_Labs_RaceAnalyzer_0.0.1"


class InvalidStateException{
	

	public:
		InvalidStateException(){}
		InvalidStateException(wxString message){
			_message = message;
		}
		
		wxString GetMessage(){
			return _message;
		}
	
		wxString _message;
};
#endif
