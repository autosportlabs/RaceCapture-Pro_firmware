/**
 * RCP
 */

#include "auto_track.h"
#include "geopoint.h"
#include "loggerConfig.h"
#include "tracks.h"


AutoTrackStatus auto_configure_track(GPSTargetConfig *cfg, GeoPoint gp) {
   if (cfg->latitude || cfg->longitude) {
      // Then this has been configured and we don't touch it.
      return ATS_ALREADY_CONFIGURED;
   }

   const Tracks *tracks = get_tracks();
   if (!tracks || tracks->count <= 0) {
      // Well shit!
      return ATS_NO_DATA;
   }

   float best_track_dist = MAX_DIST_FROM_SF;
   int i;
   for (i = 0; i < tracks->count; ++i) {
      const Track *track = &(tracks->tracks[i]);

      // XXX: inaccurate but fast.  Good enough for now.
      float track_distance = distPythag(&track->startFinish, &gp);

      if (track_distance >= best_track_dist)
         continue;

      // If here then we have a new best.  Set it accordingly
      best_track_dist = track_distance;
      cfg->latitude = track->startFinish.latitude;
      cfg->longitude = track->startFinish.longitude;
   }

   // If latitude is not 0, then we have auto-detcted a track.
   return cfg->latitude ? ATS_TRACK_FOUND : ATS_NO_TRACK_FOUND;
}
