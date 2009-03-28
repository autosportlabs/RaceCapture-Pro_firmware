#include "fileIO.h"
#include <wx/textfile.h>
#include <wx/tokenzr.h>

FileIOException::FileIOException(){
	_message="";
}

FileIOException::FileIOException( const wxString &message ){
	_message = message;	
}

const wxString FileIOException::GetMessage(){
	return _message;	
}

void MJLJConfigFileIOBase::SetFileName(const wxString& fileName){
	_fileName = fileName;
}

const wxString MJLJConfigFileIOBase::GetFileName(){
	return _fileName;
}


void MJLJConfigFileWriter::WriteConfigData(ConfigData &config){
	
	wxString fileName = GetFileName();
	wxTextFile configFile(fileName);
	
	if ( configFile.Exists() ){
		configFile.Open();
		configFile.Clear();
	}
	else{
		configFile.Create();
	}
	configFile.Write();
	configFile.Close();	
}


void MJLJConfigFileReader::ReadConfiguration(ConfigData &config){
	
	wxString fileName = GetFileName();
	wxTextFile configFile(fileName);
	
	if ( ! configFile.Exists() ){
		wxString errorMsg;
		errorMsg.Printf("Config file '%s' does not exist",fileName.ToAscii());
		throw FileIOException(errorMsg);	
	}
	
	configFile.Open();
	
	configFile.Close();
	
}