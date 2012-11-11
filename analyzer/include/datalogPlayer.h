/*
 * runtimeReader.h
 *
 *  Created on: Jun 23, 2012
 *      Author: brent
 */

#ifndef DATALOGPLAYER_H_
#define DATALOGPLAYER_H_

#include "wx/wxprec.h"
#include "chartBase.h"
#include "datalogData.h"

class DatalogPlayer : public wxThread {

public:
	const static int MAX_PLAYBACK_MULTIPLIER = 25;
	DatalogPlayer();
	~DatalogPlayer();
	void PlayFwd(int datalogId);
	void PlayRev(int datalogId);
	void Requery(int datalogId);
	void AddView(RaceAnalyzerChannelView *view);
	void InitView(RaceAnalyzerChannelView *view, size_t datalogLength);
	void Pause();
	void SkipFwd();
	void SkipRev();
	void SeekFwd();
	void SeekRev();
	void StopPlayback();
	void SetPlaybackMultiplier(int multiplier);
	int GetPlaybackMultiplier();
	void UpdateDataHistory(HistoricalView *view, wxArrayString &channels, size_t fromIndex, size_t toIndex);

	void Create(DatalogStore *datalogStore, RaceAnalyzerChannelViews *views);
	void * Entry();

private:
	void Tick(size_t offset);

	int m_datalogId;
	int m_offset;
	int m_multiplier;

	DatalogStore *m_datalogStore;
	DatalogChannels m_datalogChannels;
	DatalogStoreRows  m_datalogData;
	RaceAnalyzerChannelViews * m_views;
	wxSemaphore * m_shouldPlay;


};


#endif /* RUNTIMEREADER_H_ */
