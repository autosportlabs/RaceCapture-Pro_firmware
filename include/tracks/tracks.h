#ifndef TRACKS_H_
#define TRACKS_H_

#include "stddef.h"
#include "default_tracks.h"
#include "geopoint.h"

#define TRACKS_COUNT			40
#define SECTOR_COUNT			20


#define TRACK_TYPE_CIRCUIT 		0
#define TRACK_TYPE_SINGLE		1

typedef struct _Track{
	float radius;
	char track_type;
	union{
		GeoPoint sectors[SECTOR_COUNT];
		GeoPoint startFinish;
	};
} Track;

typedef struct _Tracks{
	size_t count;
	Track tracks[TRACKS_COUNT];
} Tracks;

int flash_tracks(const Tracks *source, size_t rawSize);
int update_track(const Track *track, size_t index, int mode);
int flash_default_tracks(void);
const Tracks * get_tracks();



#endif /* TRACKS_H_ */
