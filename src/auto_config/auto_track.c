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
      GPSPoint sf = track->startFinish;
      GeoPoint gp_sf = {sf.latitude, sf.longitude};

      // XXX: inaccurate but fast.  Good enough for now.
      float track_distance = distPythag(&gp_sf, &gp);

      if (track_distance >= best_track_dist)
         continue;

      cfg->latitude = gp_sf.latitude;
      cfg->longitude = gp_sf.longitude;
   }

   // If latitude is not 0, then we have auto-detcted a track.
   return cfg->latitude ? ATS_TRACK_FOUND : ATS_NO_TRACK_FOUND;
}
