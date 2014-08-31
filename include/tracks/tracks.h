#ifndef TRACKS_H_
#define TRACKS_H_

#include <stdint.h>
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

enum TrackType {
   TRACK_TYPE_CIRCUIT = 0,
   TRACK_TYPE_STAGE = 1,
};

#define MAGIC_NUMBER_TRACKS_INIT	0xDECAFBAD

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
	enum TrackType track_type;
	union{
		GeoPoint allSectors[SECTOR_COUNT];
		Stage stage;
		Circuit circuit;
		GeoPoint startLine;
	};
} Track;

typedef struct _Tracks{
	uint32_t magicInit;
	size_t count;
	Track tracks[MAX_TRACK_COUNT];
} Tracks;

void initialize_tracks();
int flash_tracks(const Tracks *source, size_t rawSize);
int add_track(const Track *track, size_t index, int mode);
int flash_default_tracks(void);
const Tracks * get_tracks();

/**
 * Returns the finish line of the track, regardless if its a stage or a circuit.
 * @return The GeoPoint representing the finish line.
 */
GeoPoint getFinishLine(const Track *t);

/**
 * Tells the caller if the finish line of the given track is valid.
 * @return true if its a real value, false otherwise.
 */
int isFinishLineValid(const Track *t);

#endif /* TRACKS_H_ */
