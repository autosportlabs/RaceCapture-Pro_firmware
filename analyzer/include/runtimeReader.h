/*
 * runtimeReader.h
 *
 *  Created on: Jun 23, 2012
 *      Author: brent
 */

#ifndef RUNTIMEREADER_H_
#define RUNTIMEREADER_H_

#include "wx/wxprec.h"
#include "raceCapture/raceCaptureRuntime.h"
#include "comm.h"

class RuntimeListener{
public:
	virtual void OnRuntimeValueUpdated(wxString &name, float value) = 0;
};

class RuntimeReader : public wxThread {

public:
	void Create(RaceAnalyzerComm *comm, RuntimeListener *listener);
	void * Entry();

private:
	void ReadRuntimeValues();
	void NotifyRuntimeValues(RuntimeValues &values);
	RuntimeListener *m_listener;
	RaceAnalyzerComm *m_comm;


};


#endif /* RUNTIMEREADER_H_ */
