
#ifndef FILE_IO_H_
#define FILE_IO_H_

#include "wx/wxprec.h"
#include "configData.h"

class RaceCaptureConfigFileIOBase{

	public:
		const wxString GetFileName();
		void SetFileName(const wxString& fileName);


	private:
		wxString _fileName;

};

class RaceCaptureConfigFileReader : public RaceCaptureConfigFileIOBase{

	public:
		void ReadConfiguration(ConfigData &config);

	private:
};

class RaceCaptureConfigFileWriter : public RaceCaptureConfigFileIOBase{

	public:
		void WriteConfigData(ConfigData &config);

	private:
};


#endif /*MJLJFILEIO_*/
