
#ifndef FILE_IO_H_
#define FILE_IO_H_

#include <wx/wx.h>
#include "configData.h"

class FileIOException{
	
	
	public:
		FileIOException();
		FileIOException( const wxString &message );
		const wxString GetMessage();
	
	private:
		wxString _message;
	
};


class MJLJConfigFileIOBase{

	public:
		const wxString GetFileName();
		void SetFileName(const wxString& fileName);


	private:
		wxString _fileName; 	
	
};

class MJLJConfigFileReader : public MJLJConfigFileIOBase{
	
	public:
		void ReadConfiguration(ConfigData &config);
	
	private:
};

class MJLJConfigFileWriter : public MJLJConfigFileIOBase{
	
	public:
		void WriteConfigData(ConfigData &config);
	
	private:
};


#endif /*MJLJFILEIO_*/
