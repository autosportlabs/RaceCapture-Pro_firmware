/**
 * RCP
 */

#ifndef __AUTO_TRACK_H__
#define __AUTO_TRACK_H__

#include "geopoint.h"
#include "loggerConfig.h"
#include "tracks.h"

/**
 * Using 1K Meters as max distance from start finish as if you are farther than
 * that from start/finish then just wow.
 */
#define MAX_DIST_FROM_SF 1000

/**
 * Automatically picks the best track (if available) and updates the config to use this
 * track.
 * @param cfg Pointer to the config that will be used for the config info.
 * @param gp The GeoPoint that we got when we picked up our first good fix.
 * @return A status indicator that tells the caller the result of the call.
 */
const Track * auto_configure_track(Track *defaultCfg, GeoPoint gp);

#endif // __AUTO_TRACK_H__
