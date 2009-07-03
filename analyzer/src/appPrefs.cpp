
#include "appPrefs.h"
#include "wx/config.h"
#include "wx/stdpaths.h"

AppPrefs::AppPrefs()
{
}


void AppPrefs::SaveAppPrefs(){
	wxConfig config(RACE_ANALYZER_APP_NAME);

	config.Write(CONFIG_KEY_LAST_CONFIG_DIRECTORY,_lastConfigFileDirectory);
	config.Write(CONFIG_KEY_LAST_DATALOG_DIRECTORY, _lastDatalogDirectory);

	int count = _perspectives.Count();
	config.Write(CONFIG_KEY_PERSPECTIVE_COUNT, count);
	config.Write(CONFIG_KEY_ACTIVE_PERSPECTIVE, _activePerspective);
	for (int i = 0; i < count; i++){
		config.Write(wxString::Format("%s%d",CONFIG_KEY_PERSPECTIVE,i), _perspectives[i]);
		config.Write(wxString::Format("%s%d",CONFIG_KEY_PERSPECTIVE_NAME,i), _perspectiveNames[i]);
	}
}

void AppPrefs::LoadAppPrefs(){
	wxConfig config(RACE_ANALYZER_APP_NAME);

	wxStandardPaths standardPaths;

	_lastConfigFileDirectory = config.Read(CONFIG_KEY_LAST_CONFIG_DIRECTORY, standardPaths.GetUserConfigDir());
	_lastDatalogDirectory = config.Read(CONFIG_KEY_LAST_DATALOG_DIRECTORY, standardPaths.GetUserConfigDir());

	int count;
	config.Read(CONFIG_KEY_PERSPECTIVE_COUNT, &count,0);
	config.Read(CONFIG_KEY_ACTIVE_PERSPECTIVE, &_activePerspective,0);
	//if we can't load all perspectives correctly we fail-back to defaults
	for (int i = 0; i < count; i++){
		wxString perspectiveConfig;
		if (config.Read(wxString::Format("%s%d",CONFIG_KEY_PERSPECTIVE,i), &perspectiveConfig)){
			_perspectives.Add(perspectiveConfig);
		}
		else{
			ResetDefaults();
			break;
		}
		wxString perspectiveName;
		if (config.Read(wxString::Format("%s%d", CONFIG_KEY_PERSPECTIVE_NAME,i), &perspectiveName)){
			_perspectiveNames.Add(perspectiveName);
		}
		else{
			ResetDefaults();
			break;
		}
	}
}

void AppPrefs::ResetDefaults(){
	_perspectives.Empty();
	_activePerspective = 0;
	SaveAppPrefs();
}

wxArrayString & AppPrefs::GetPerspectives(){
	return _perspectives;
}

wxArrayString & AppPrefs::GetPerspectiveNames(){
	return _perspectiveNames;
}

int & AppPrefs::GetActivePerspective(){
	return _activePerspective;
}

void AppPrefs::SetActivePerspective(int activePerspective){
	_activePerspective = activePerspective;
}

void AppPrefs::SetLastConfigFileDirectory(wxString lastConfigFileDirectory){
	_lastConfigFileDirectory = lastConfigFileDirectory;
}

wxString AppPrefs::GetLastConfigFileDirectory(){
	return _lastConfigFileDirectory;
}

void AppPrefs::SetLastDatalogDirectory(wxString lastDatalogDirectory){
	_lastDatalogDirectory = lastDatalogDirectory;
}

wxString AppPrefs::GetLastDatalogDirectory(){
	return _lastDatalogDirectory;
}
