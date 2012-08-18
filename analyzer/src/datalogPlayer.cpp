/*
 * runtimeReader.cpp
 *
 *  Created on: Jun 23, 2012
 *      Author: brent
 */
#include "datalogPlayer.h"

DatalogPlayer::DatalogPlayer() : m_offset(0), m_multiplier(1), m_datalogData(NULL), m_views(NULL){
	m_shouldPlay = new wxSemaphore(0,1);
}

DatalogPlayer::~DatalogPlayer(){
	delete(m_shouldPlay);
}

void DatalogPlayer::Play(){
	m_shouldPlay->Post();
}

void DatalogPlayer::Pause(){
	m_shouldPlay->Wait();
}

void DatalogPlayer::FastForwardBeginning(){
	m_offset = 0;
}

void DatalogPlayer::FastForwardEnd(){
	if (NULL == m_datalogData) return;
	m_offset = m_datalogData->Count() - 1;
}

void DatalogPlayer::SetPlaybackMultiplier(int multiplier)
{
	m_multiplier = multiplier;
}

void DatalogPlayer::Create(DatalogStoreRows *data, RaceAnalyzerChannelViews *views )  {
	m_datalogData = data;
	m_views = views;
  	wxThread::Create();
}

void * DatalogPlayer::Entry(){
	while (! TestDestroy()){
		m_shouldPlay->Wait();
		Tick();
		m_shouldPlay->Post();
		wxThread::Sleep(33);
		if (m_offset < m_datalogData->Count() - 1) m_offset++;
	}
	return NULL;
}


void DatalogPlayer::Tick(){
	for (size_t i = 0; i < m_views->Count(); i++){
		(*m_views)[i]->SetOffset(m_offset);
	}
}




