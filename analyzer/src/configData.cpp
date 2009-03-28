#include "wx/wx.h"
#include "configData.h"

ConfigValueException::ConfigValueException(){
	_message = "";
}

ConfigValueException::ConfigValueException( const wxString &message){
	wxLogError("Config Value Exception: %s", message.ToAscii());
	_message = message;	
}

const wxString ConfigValueException::GetMessage(){
	return _message;
}

bool VersionData::IsDefined() const{
	return !(!_major && !_minor && !_bugfix);
}

void VersionData::SetMajor(int major){	
	_major = major;
}

int VersionData::GetMajor() const{
	return _major;
}

void VersionData::SetMinor(int minor){
	_minor = minor;
}

int VersionData::GetMinor() const{
	return _minor;
}

int VersionData::GetBugfix() const{
	return _bugfix;
}

void VersionData::SetBugfix(int bugfix){
	_bugfix = bugfix;
}	

wxString VersionData::GetNotes() const{
	return _notes;
}

void VersionData::SetNotes(wxString notes){
	_notes = notes;
}

