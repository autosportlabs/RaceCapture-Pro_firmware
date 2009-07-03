/*
 * datalogData.h
 *
 *  Created on: Jun 10, 2009
 *      Author: brent
 */

#include "wx/wxprec.h"
#include <limits>
#include <wx/dynarray.h>

#ifndef DATALOGDATA_H_
#define DATALOGDATA_H_

WX_DEFINE_ARRAY_DOUBLE(double, RowValues);

class DatastoreRow{

public:
	const static double NULL_VALUE = -0xFFFFFFFFFFFFFFF;
	DatastoreRow() : timePoint(0)
	{}

	DatastoreRow(int tp, RowValues v) : timePoint(tp),values(v)
	{}
	~DatastoreRow(){ }

	int timePoint;
	RowValues values;
};

WX_DECLARE_OBJARRAY(DatastoreRow,DatalogStoreRows);


class DatalogChannelType{

public:
	DatalogChannelType(wxString newName, wxString newUnitsLabel, int newSmoothingLevel, double newMinValue, double newMaxValue);
	wxString name;
	wxString unitsLabel;
	int smoothingLevel;
	double minValue;
	double maxValue;
};

class DatalogChannel{

public:
	DatalogChannel(wxString newName, int newTypeId, wxString newDescription);
	DatalogChannel();

	wxString name;
	int typeId;
	wxString description;
};

WX_DECLARE_OBJARRAY(DatalogChannel,DatalogChannels);
WX_DECLARE_OBJARRAY(DatalogChannelType, DatalogChannelTypes);

class DatalogChannelUtil{

public:

	static int FindChannelIdByName(DatalogChannels &channels, wxString &name);
	static int FindChannelTypeIdByName(DatalogChannelTypes &channelTypes, wxString &name);

};
#endif /* DATALOGDATA_H_ */
