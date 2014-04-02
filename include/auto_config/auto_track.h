/**
 * RCP
 */

#ifndef __AUTO_TRACK_H__
#define __AUTO_TRACK_H__

#include "geopoint.h"
#include "loggerConfig.h"

/**
 * Using 1K Meters as max distance from start finish as if you are farther than
 * that from start/finish then just wow.
 */
#define MAX_DIST_FROM_SF 1000

typedef enum _AutoTrackStatus {
   ATS_NO_TRACK_FOUND,
   ATS_TRACK_FOUND,
   ATS_ALREADY_CONFIGURED,
   ATS_NO_DATA,
} AutoTrackStatus;

#define IS_AUTO_TRACK_STATUS_ERROR(s) ((s) > ATS_TRACK_FOUND)

/**
 * Automatically picks the best track (if available) and updates the config to use this
 * track.
 * @param cfg Pointer to the config that will be used for the config info.
 * @param gp The GeoPoint that we got when we picked up our first good fix.
 * @return A status indicator that tells the caller the result of the call.
 */
AutoTrackStatus auto_configure_track(GPSTargetConfig *cfg, GeoPoint gp);

#endif // __AUTO_TRACK_H__
