#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include "fileIO.h"
#include "exceptions.h"
#include "logging.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"



void RaceCaptureConfigFileIOBase::SetFileName(const wxString& fileName){
	_fileName = fileName;
}

const wxString RaceCaptureConfigFileIOBase::GetFileName(){
	return _fileName;
}


void RaceCaptureConfigFileWriter::WriteConfigData(RaceCaptureConfig &config){

	wxString fileName = GetFileName();
	wxFile configFile;

	if (!configFile.Open(fileName,wxFile::write)){
		throw FileAccessException(wxString::Format("Could not open config file '%s'", fileName.ToAscii()),fileName);
	}

	Object jsonConfig = config.ToJson();

	std::stringstream stream;
	Writer::Write(jsonConfig, stream);
	std::string configData = stream.str();
	configFile.Write(configData.c_str(), configData.size());

	configFile.Close();
}


void RaceCaptureConfigFileReader::ReadConfiguration(RaceCaptureConfig &config){

	wxString fileName = GetFileName();
	wxFile configFile;

	if ( ! configFile.Exists(fileName) ){
		wxString errorMsg;
		errorMsg.Printf("Config file '%s' does not exist",fileName.ToAscii());
		throw FileAccessException(errorMsg,fileName);
	}
	configFile.Open(fileName, wxFile::read);

	Object jsonConfig;

	std::stringstream stream;
	char byte;
	while(configFile.Read(&byte,1)){
		stream.put(byte);
	}
	Reader::Read(jsonConfig,stream);
	config.FromJson(jsonConfig);
	configFile.Close();
}
