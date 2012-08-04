/*
 * runtimeReader.cpp
 *
 *  Created on: Jun 23, 2012
 *      Author: brent
 */
#include "runtimeReader.h"

void RuntimeReader::Create(RaceAnalyzerComm *comm, RuntimeListener *listener){
	m_comm = comm;
	m_listener = listener;
}

void * RuntimeReader::Entry(){
	while (! TestDestroy()){
		ReadRuntimeValues();
	}
	return NULL;
}

void RuntimeReader::ReadRuntimeValues(){
	RuntimeValues values;

	//Read the runtime values here
	NotifyRuntimeValues(values);
}

void RuntimeReader::NotifyRuntimeValues(RuntimeValues &values){
	RuntimeValues::iterator it;
	for (it = values.begin(); it != values.end(); ++it ){
		m_listener->OnRuntimeValueUpdated(it->first,it->second);
	}

}




