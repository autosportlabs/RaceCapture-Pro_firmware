#ifndef TRACKS_H_
#define TRACKS_H_

#include "stddef.h"
#include "default_tracks.h"
#include "geopoint.h"

#define TRACK_ADD_RESULT_OK  		1
#define TRACK_ADD_RESULT_FAIL  		0

#define TRACK_ADD_MODE_IN_PROGRESS	1
#define TRACK_ADD_MODE_COMPLETE 	2

#define MAX_TRACK_COUNT				40
#define SECTOR_COUNT				20
#define CIRCUIT_SECTOR_COUNT		SECTOR_COUNT - 1
#define STAGE_SECTOR_COUNT			SECTOR_COUNT - 2

#define DEFAULT_TRACK_TARGET_RADIUS	0.0001

#define TRACK_TYPE_CIRCUIT 			0
#define TRACK_TYPE_SINGLE			1

typedef struct _Circuit{
	GeoPoint startFinish;
	GeoPoint sectors[CIRCUIT_SECTOR_COUNT];
} Circuit;

typedef struct _Stage{
	GeoPoint start;
	GeoPoint sectors[STAGE_SECTOR_COUNT];
	GeoPoint finish;
} Stage;


typedef struct _Track{
	unsigned char track_type;
	union{
		GeoPoint allSectors[SECTOR_COUNT];
		Stage stage;
		Circuit circuit;
		GeoPoint startLine;
	};
} Track;

typedef struct _Tracks{
	size_t count;
	Track tracks[MAX_TRACK_COUNT];
} Tracks;

int flash_tracks(const Tracks *source, size_t rawSize);
int add_track(const Track *track, size_t index, int mode);
int flash_default_tracks(void);
const Tracks * get_tracks();



#endif /* TRACKS_H_ */
