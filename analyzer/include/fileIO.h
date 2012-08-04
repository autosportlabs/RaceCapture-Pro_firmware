
#ifndef FILE_IO_H_
#define FILE_IO_H_

#include "wx/wxprec.h"
#include "raceCapture/raceCaptureConfig.h"
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
		void ReadConfiguration(RaceCaptureConfig &config);

	private:
};

class RaceCaptureConfigFileWriter : public RaceCaptureConfigFileIOBase{

	public:
		void WriteConfigData(RaceCaptureConfig &config);

	private:
};


#endif /*MJLJFILEIO_*/
