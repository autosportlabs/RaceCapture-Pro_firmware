#include "fileIO.h"
#include "exceptions.h"
#include "logging.h"
#include <wx/textfile.h>
#include <wx/tokenzr.h>


void RaceCaptureConfigFileIOBase::SetFileName(const wxString& fileName){
	_fileName = fileName;
}

const wxString RaceCaptureConfigFileIOBase::GetFileName(){
	return _fileName;
}


void RaceCaptureConfigFileWriter::WriteConfigData(ConfigData &config){

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


void RaceCaptureConfigFileReader::ReadConfiguration(ConfigData &config){

	wxString fileName = GetFileName();
	wxTextFile configFile(fileName);

	if ( ! configFile.Exists() ){
		wxString errorMsg;
		errorMsg.Printf("Config file '%s' does not exist",fileName.ToAscii());
		throw FileAccessException(errorMsg,fileName);
	}

	configFile.Open();

	configFile.Close();

}
