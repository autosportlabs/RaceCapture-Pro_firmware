/*
 * chartBase.h
 *
 *  Created on: 2009-07-07
 *      Author: brent
 */
#include "wx/wxprec.h"
#include <wx/dynarray.h>
#include "datalogStore.h"
#include "appOptions.h"
#include "appPrefs.h"
#include "commonEvents.h"
#include <wx/dynarray.h>

#ifndef CHARTBASE_H_
#define CHARTBASE_H_


WX_DEFINE_ARRAY_DOUBLE(double, ChartValues);

class ChartParams{

public:
	ChartParams() :
		appPrefs(NULL),
		appOptions(NULL)
		{}

	ChartParams(
			AppPrefs *prefs,
			AppOptions *options
			):
		appPrefs(prefs),
		appOptions(options)
		{

		}

	AppPrefs 					*appPrefs;
	AppOptions					*appOptions;
};

class HistoricalView{
public:
	virtual void UpdateValueRange(wxString &channel, size_t fromIndex, size_t toIndex, ChartValues &values) = 0;
};

class RaceAnalyzerChannelView{

public:
	virtual void SetBufferSize(wxString &channel, size_t size) = 0;
	virtual void UpdateValue(wxString &channelName, size_t index, double value) = 0;
	virtual void SetChartParams(ChartParams params) = 0;
};

WX_DECLARE_OBJARRAY(RaceAnalyzerChannelView *,RaceAnalyzerChannelViews);

class RequestDatalogRangeParams {
public:

	RequestDatalogRangeParams(): view(NULL), channelName(""), fromIndex(0), toIndex(0){}
	RequestDatalogRangeParams(HistoricalView *view, wxString channelName, size_t fromIndex, size_t toIndex): view(view), channelName(channelName), fromIndex(fromIndex), toIndex(toIndex){}

	HistoricalView *view;
	wxString channelName;
	size_t fromIndex;
	size_t toIndex;
};

#endif /* CHARTBASE_H_ */
