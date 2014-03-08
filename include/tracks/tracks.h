#ifndef TRACKS_H_
#define TRACKS_H_

#include "stddef.h"
#include "default_tracks.h"

#define TRACKS_COUNT			100
#define SECTOR_COUNT			10

typedef struct _GPSPoint{
	float latitude;
	float longitude;
} GPSPoint;

typedef struct _Track{
	GPSPoint startFinish;
	GPSPoint sectors[SECTOR_COUNT];
} Track;

typedef struct _Tracks{
	size_t count;
	Track tracks[TRACKS_COUNT];
} Tracks;

int flash_tracks(const Tracks *source, size_t rawSize);
int update_track(const Track *track, size_t index, int mode);
int flash_default_tracks(void);
Track * find_track(float latitude, float longitude);
const Tracks * get_tracks();



#endif /* TRACKS_H_ */
