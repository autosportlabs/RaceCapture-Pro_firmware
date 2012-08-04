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
	void Play();
	void Pause();
	void FastForwardBeginning();
	void FastForwardEnd();
	void SetPlaybackMultiplier(int multiplier);

	void Create(DatalogStoreRows *data, RaceAnalyzerChannelViews *views);
	void * Entry();

private:
	void Tick();

	int m_offset;
	int m_multiplier;
	DatalogStoreRows * m_datalogData;
	RaceAnalyzerChannelViews * m_views;
	wxSemaphore * m_shouldPlay;

};


#endif /* RUNTIMEREADER_H_ */
