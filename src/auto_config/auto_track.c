/**
 * RCP
 */

#include "auto_track.h"
#include "geopoint.h"
#include "loggerConfig.h"
#include "tracks.h"
#include "printk.h"


const Track * auto_configure_track(Track *defaultCfg, GeoPoint gp) {
	pr_info("Configuring start/finish...");

   if (isStartPointValid(defaultCfg) && isFinishPointValid(defaultCfg)) {
      pr_info("using fixed config\r\n");
      // Then this has been configured and we don't touch it.
      return defaultCfg;
   }

   const Tracks *tracks = get_tracks();
   if (!tracks || tracks->count <= 0) {
      // Well shit!
      pr_info("error!\r\n");
      return defaultCfg;
   }

   float best_track_dist = MAX_DIST_FROM_SF;
   const Track *foundTrack = NULL;

   for (unsigned i = 0; i < tracks->count; ++i) {
      const Track *track = &(tracks->tracks[i]);

      // XXX: inaccurate but fast.  Good enough for now.
      GeoPoint startPoint = getStartPoint(track);
      float track_distance = distPythag(&startPoint, &gp);

      if (track_distance >= best_track_dist)
         continue;

      // If here then we have a new best.  Set it accordingly
      best_track_dist = track_distance;
      foundTrack = track;
   }

   // if no track found, fall back to default
   if (! foundTrack){
	   pr_info("no ");
	   foundTrack = defaultCfg;
   }

   pr_info("track found\r\n");
   return foundTrack;
}
