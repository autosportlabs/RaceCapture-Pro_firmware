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
	bool operator==(const DatalogChannelType &rh){
		return (name == rh.name);
	}
	wxString name;
	wxString unitsLabel;
	int smoothingLevel;
	double minValue;
	double maxValue;
};

class DatalogChannelSystemTypes {

public:
	static DatalogChannelType GetLatitudeChannelType(){ return DatalogChannelType("Latitude", "Degrees", 0, -90.0, 90.0);}
	static DatalogChannelType GetLongitudeChannelType(){ return DatalogChannelType ("Longitude", "Degrees", 0, -180.0, 180.0);}
};


class DatalogChannel{

public:
	static const int UNDEFINED_TYPE = -1;
	DatalogChannel(wxString newName, int newTypeId = UNDEFINED_TYPE, wxString newDescription = "", bool en = true);
	DatalogChannel();

	wxString name;
	int typeId;
	wxString description;
	bool enabled;
};

WX_DECLARE_OBJARRAY(DatalogChannel,DatalogChannels);
WX_DECLARE_OBJARRAY(DatalogChannelType, DatalogChannelTypes);

class DatalogChannelUtil{

public:

	static int FindChannelIdByName(DatalogChannels &channels, wxString &name);
	static int FindChannelTypeIdByName(DatalogChannelTypes &channelTypes, wxString &name);

};
#endif /* DATALOGDATA_H_ */
