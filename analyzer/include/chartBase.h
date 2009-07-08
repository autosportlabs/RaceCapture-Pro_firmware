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
#

#ifndef CHARTBASE_H_
#define CHARTBASE_H_

class ChartParams{

public:
	ChartParams() :
		appPrefs(NULL),
		appOptions(NULL),
		datalogStore(NULL)
		{}

	ChartParams(
			AppPrefs *prefs,
			AppOptions *options,
			DatalogStore *store ):
		appPrefs(prefs),
		appOptions(options),
		datalogStore(store)
		{

		}

	AppPrefs 			*appPrefs;
	AppOptions			*appOptions;
	DatalogStore 		*datalogStore;
};


class RaceAnalyzerChart{

public:
	virtual void SetChartParams(ChartParams params) = 0;

};

WX_DECLARE_OBJARRAY(RaceAnalyzerChart *,RaceAnalyzerCharts);

#endif /* CHARTBASE_H_ */
