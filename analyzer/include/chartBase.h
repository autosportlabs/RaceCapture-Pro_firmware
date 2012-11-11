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

DECLARE_EVENT_TYPE ( REQUEST_DATALOG_DATA_EVENT, -1 )


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

class ViewDataHistory {

public:

	ViewDataHistory(wxString channelName, ChartValues values): channelName(channelName), values(values) {}

	wxString channelName;
	ChartValues values;
};

WX_DECLARE_OBJARRAY(ViewDataHistory, ViewDataHistoryArray);

class HistoricalView{
public:
	virtual void SetBufferSize(wxArrayString &channels, size_t size) = 0;
	virtual void UpdateValueRange(ViewDataHistoryArray &historyArray, size_t fromIndex, size_t toIndex) = 0;
};

class RaceAnalyzerChannelView{

public:
	virtual void UpdateValue(wxString &channelName, size_t index, double value) = 0;
	virtual void SetChartParams(ChartParams params) = 0;
};

WX_DECLARE_OBJARRAY(RaceAnalyzerChannelView *,RaceAnalyzerChannelViews);

class RequestDatalogRangeParams {
public:

	RequestDatalogRangeParams(): view(NULL), fromIndex(0), toIndex(0){}
	RequestDatalogRangeParams(HistoricalView *view, wxArrayString channelNames, size_t fromIndex, size_t toIndex): view(view), channelNames(channelNames), fromIndex(fromIndex), toIndex(toIndex){}

	HistoricalView *view;
	wxArrayString channelNames;
	size_t fromIndex;
	size_t toIndex;
};

enum{
   ID_LOGVIEWER_SCROLL			= wxID_HIGHEST + 1,
   ID_REQUEST_DATALOG_DATA
};


#endif /* CHARTBASE_H_ */
