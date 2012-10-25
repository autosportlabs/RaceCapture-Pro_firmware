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
	DatalogPlayer();
	~DatalogPlayer();
	void Play(int datalogId);
	void Requery(int datalogId);
	void Pause();
	void FastForwardBeginning();
	void FastForwardEnd();
	void SetPlaybackMultiplier(int multiplier);
	void UpdateDataHistory(HistoricalView *view, wxString &channel, size_t fromIndex, size_t toIndex);

	void Create(DatalogStore *datalogStore, RaceAnalyzerChannelViews *views);
	void * Entry();

private:
	void Tick(size_t offset);

	int m_datalogId;
	size_t m_offset;
	int m_multiplier;

	DatalogStore *m_datalogStore;
	DatalogChannels m_datalogChannels;
	DatalogStoreRows  m_datalogData;
	RaceAnalyzerChannelViews * m_views;
	wxSemaphore * m_shouldPlay;

};


#endif /* RUNTIMEREADER_H_ */
