#ifndef APPPEFS_H_
#define APPPEFS_H_
#include "wx/wx.h"

#include "raceAnalyzerConfigBase.h"

#define CONFIG_KEY_LAST_CONFIG_DIRECTORY "lastConfigDir"
#define CONFIG_KEY_LAST_DATALOG_DIRECTORY "lastDatalogDir"
#define CONFIG_KEY_PERSPECTIVE 	"perspective_"
#define CONFIG_KEY_PERSPECTIVE_COUNT "perspectiveCount"
#define CONFIG_KEY_ACTIVE_PERSPECTIVE "activePerspective"
#define CONFIG_KEY_PERSPECTIVE_NAME "perspectiveName_"


class AppPrefs{
	
	public:
		
		
		AppPrefs(); 
		
		wxArrayString & GetPerspectives();
		int & GetActivePerspective();
		void SetActivePerspective(int activePerspective);
		wxArrayString & GetPerspectiveNames();
		wxString GetLastConfigFileDirectory();
		void SetLastConfigFileDirectory(wxString lastconfigFileDirectory);
		wxString GetLastDatalogDirectory();
		void SetLastDatalogDirectory(wxString lastDatalogDirectory);
		
		
		void SaveAppPrefs();
		void LoadAppPrefs();
		void ResetDefaults();
		
	private:
		int				_activePerspective;
		wxArrayString 	_perspectives;
		wxArrayString	_perspectiveNames;
		wxString		_lastConfigFileDirectory;
		wxString		_lastDatalogDirectory;
				
};
#endif /*APPOPTIONS_H_*/
