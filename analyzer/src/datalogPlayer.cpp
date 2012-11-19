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

void DatalogPlayer::PlayFwd(int datalogId){
	if (m_datalogId != datalogId) Requery(datalogId);
	SetPlaybackMultiplier(1);
	m_shouldPlay->Post();
}

void DatalogPlayer::PlayRev(int datalogId){
	if (m_datalogId != datalogId) Requery(datalogId);
	SetPlaybackMultiplier(-1);
	m_shouldPlay->Post();
}

void DatalogPlayer::Pause(){
	m_shouldPlay->TryWait();
}

void DatalogPlayer::SkipFwd(){
	m_offset = 0;
	Tick(m_offset);
}

void DatalogPlayer::SkipRev(){
	m_offset = m_datalogData.Count() - 1;
	Tick(m_offset);
}

void DatalogPlayer::SeekFwd(){
	int playbackMultiplier = GetPlaybackMultiplier();
	if (playbackMultiplier < MAX_PLAYBACK_MULTIPLIER){
		SetPlaybackMultiplier(playbackMultiplier + 1);
		m_shouldPlay->Post();
	}
}

void DatalogPlayer::SeekRev(){
	int playbackMultiplier = GetPlaybackMultiplier();
	if (playbackMultiplier > -MAX_PLAYBACK_MULTIPLIER){
		SetPlaybackMultiplier(playbackMultiplier - 1);
		m_shouldPlay->Post();
	}
}

void DatalogPlayer::SetPlaybackMultiplier(int multiplier)
{
	m_multiplier = multiplier;
}

int DatalogPlayer::GetPlaybackMultiplier(){
	return m_multiplier;
}

void DatalogPlayer::Create(DatalogStore *datalogStore, RaceAnalyzerChannelViews *views )  {
	m_datalogStore = datalogStore;
	m_views = views;
  	wxThread::Create();
}

void DatalogPlayer::StopPlayback(){
	SetPlaybackMultiplier(0);
	Pause();
}

void * DatalogPlayer::Entry(){
	while (! TestDestroy()){
		m_shouldPlay->Wait();
		Tick(m_offset);
		m_shouldPlay->Post();
		wxThread::Sleep(33);

		m_offset += m_multiplier;

		int ubound = m_datalogData.Count() - 1;
		if (m_offset < 0 ){
			StopPlayback();
			m_offset = 0;
		}
		if (m_offset > ubound ){
			StopPlayback();
			m_offset = ubound;
		}
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
		InitView((*m_views)[i], datalogLength);
	}
}

void DatalogPlayer::AddView(RaceAnalyzerChannelView *view){
	InitView(view, m_datalogData.Count());
}

void DatalogPlayer::InitView(RaceAnalyzerChannelView *view, size_t datalogLength){

	wxArrayString channels;
	for (size_t ii = 0; ii < m_datalogChannels.Count(); ii++){
		wxString channel = m_datalogChannels[ii].name;
		channels.Add(channel);
	}

	HistoricalView *hv = dynamic_cast<HistoricalView *>(view);
	if (NULL != hv) hv->SetBufferSize(channels, datalogLength);

}

void DatalogPlayer::UpdateDataHistory(HistoricalView *view, wxArrayString &channels, size_t fromIndex, size_t toIndex){

	ViewDataHistoryArray viewDataHistoryArray;

	for (wxArrayString::iterator it = channels.begin(); it != channels.end(); ++it){

		ChartValues values;
		values.Alloc(toIndex - fromIndex);
		int channelId = DatalogChannelUtil::FindChannelIdByName(m_datalogChannels, *it);
		if ( channelId >= 0){
			for (size_t i = fromIndex; i < toIndex; i++){
				DatastoreRow row = m_datalogData[i - fromIndex];
				double value = row.values[channelId];
				values.Add(value);
			}
			ViewDataHistory history(*it, values);
			viewDataHistoryArray.Add(history);
		}
	}
	if (NULL != view) view->UpdateValueRange(viewDataHistoryArray, fromIndex, toIndex);
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




