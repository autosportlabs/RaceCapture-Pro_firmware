/*
 * runtimeReader.cpp
 *
 *  Created on: Jun 23, 2012
 *      Author: brent
 */
#include "datalogPlayer.h"

DatalogPlayer::DatalogPlayer() : m_datalogId(0), m_offset(0), m_multiplier(1), m_datalogStore(NULL), m_views(NULL){
	m_shouldPlay = new wxSemaphore(0,1);
}

DatalogPlayer::~DatalogPlayer(){
	delete(m_shouldPlay);
}

void DatalogPlayer::Play(int datalogId){
	if (m_datalogId != datalogId) Requery(datalogId);
	m_shouldPlay->Post();
}

void DatalogPlayer::Pause(){
	m_shouldPlay->Wait();
}

void DatalogPlayer::FastForwardBeginning(){
	m_offset = 0;
}

void DatalogPlayer::FastForwardEnd(){
	m_offset = m_datalogData.Count() - 1;
}

void DatalogPlayer::SetPlaybackMultiplier(int multiplier)
{
	m_multiplier = multiplier;
}

void DatalogPlayer::Create(DatalogStore *datalogStore, RaceAnalyzerChannelViews *views )  {
	m_datalogStore = datalogStore;
	m_views = views;
  	wxThread::Create();
}

void DatalogPlayer::UpdateDataHistory(HistoricalView *view, wxString &channel, size_t fromIndex, size_t toIndex){

	ChartValues values;
	values.Alloc(toIndex - fromIndex);
	int channelId = DatalogChannelUtil::FindChannelIdByName(m_datalogChannels, channel);
	if ( channelId >= 0){
		for (size_t i = fromIndex; i < toIndex; i++){
			DatastoreRow row = m_datalogData[i - fromIndex];
			double value = row.values[channelId];
			values.Add(value);
		}
		if (NULL != view) view->UpdateValueRange(channel, fromIndex, toIndex, values);
	}
}


void * DatalogPlayer::Entry(){
	while (! TestDestroy()){
		m_shouldPlay->Wait();
		Tick(m_offset);
		m_shouldPlay->Post();
		wxThread::Sleep(33);
		if (m_offset < m_datalogData.Count() - 1) m_offset++;
	}
	return NULL;
}

void DatalogPlayer::Requery(int datalogId){
	m_datalogChannels.Clear();
	m_datalogData.Clear();
	m_datalogStore->GetChannels(datalogId, m_datalogChannels);
	wxArrayString channelNames;
	for (size_t i = 0; i < m_datalogChannels.Count(); i++){
		channelNames.Add(m_datalogChannels[i].name);
	}
	m_datalogStore->ReadDatalog(m_datalogData, datalogId, channelNames, 0);
	m_datalogId = datalogId;

	size_t datalogLength = m_datalogData.Count();
	for (size_t i = 0; i < m_views->Count(); i++){
		for (size_t ii = 0; ii < m_datalogChannels.Count(); ii++){
			wxString channel = m_datalogChannels[ii].name;
			(*m_views)[i]->SetBufferSize(channel, datalogLength);
		}
	}
}

void DatalogPlayer::Tick(size_t index){
	DatastoreRow row = m_datalogData[index];
	RowValues &values = row.values;
	size_t valuesCount = values.Count();
	for (size_t i = 0; i < m_views->Count(); i++){
		for (size_t ii = 0; ii < valuesCount; ii++){
			wxString channel = m_datalogChannels[ii].name;
			double value = row.values[ii];
			(*m_views)[i]->UpdateValue(channel, index, value);
		}
	}
}




