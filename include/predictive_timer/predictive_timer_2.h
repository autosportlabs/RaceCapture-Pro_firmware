/*
 * predective_timer_2.h
 *
 *  Created on: Jan 20, 2014
 *      Author: stieg
 */

#ifndef PREDICTIVE_TIMER_2_H_
#define PREDICTIVE_TIMER_2_H_

/* #ifdef __cplusplus */
/* extern "C" { */
/* #endif */

#include "dateTime.h"
#include "geopoint.h"

#include <stdbool.h>

/**
 * Called when we finish a lap.  Adds the final sample and adjusts sample rates
 * in preperation for the next lap.  Must be called after #startLap is called.
 * @param gpsSnapshot The GPS state when the lap was completed.
 */
void finishLap(const GpsSnapshot *gpsSnapshot);

/**
 * Called when we start a lap.  This sets the appropriate timers and starts recording
 * of the times.  This must be called before either #addGpsSample or #finishLap are
 * called.
 * @param point The position when the lap started.
 * @param time The time (millis) when the lap started.
 */
void startLap(const GeoPoint *point, const tiny_millis_t time);

/**
 * Adds a new GPS sample to our record if the algorithm determines its time for one.
 * Must be invoked after the invocation of #startLap but before the invocation of
 * #finishLap
 * @param gpsSnapshot The GPS state when the lap was started.
 * @return TRUE if it was added, FALSE otherwise.
 */
bool addGpsSample(const GpsSnapshot *gpsSnapshot);

/**
 * Calculates the split of your current time against the fast lap time at the position given.
 * @param point The position you are currently at.
 * @param time The time (millis) which the sample was taken.
 * @return The split between your current time and the fast lap time.  Positive indicates you are
 * going faster than your fast lap, negative indicates slower.
 */
tiny_millis_t getSplitAgainstFastLap(const GeoPoint *  point, tiny_millis_t time);

/**
 * Figures out the predicted lap time.  Call as much as you like... it will only do
 * calculations when new data is actually available.  This minimizes inaccuracies and
 * allows for drivers to better see how their most recent driving affected their predicted
 * lap time.
 * @param point The current location of the car.
 * @param time The time (millis) which the sample was taken.
 * @return The predicted lap time.
 */
tiny_millis_t getPredictedTime(const GeoPoint * point, tiny_millis_t time);

/**
 * Like #getPredictedTime but returns the value in minutes.  Useful for logging compatibility.
 */
float getPredictedTimeInMinutes();

/**
 * Tells the caller if a predictive time is ready to be had.
 * @return True if it is, false otherwise.
 */
bool isPredictiveTimeAvailable();

/**
 * Resets the predictive timer logic to the initial state.
 */
void resetPredictiveTimer();

/**
 * Finds the percentage that currPt is between startPt and endPt.as a
 * projection of point c onto the vector formed between points s and e.
 * This method requires that point m be between the points s and e on
 * the earth's surface.  If that requirement is met this method should
 * return a value between 0 and 1.  Otherwise the value will be undefined.
 * those bounds then
 * @param s The start point.
 * @param e The end point.
 * @param m The middle point.
 * @return The percentage that projected point m lies between startPt
 * and endPt if the method
 * requirements were met. < 0 or > 1 otherwise.
 */
float distPctBtwnTwoPoints(const GeoPoint *s, const GeoPoint *e, const GeoPoint *m);


/* #ifdef __cplusplus */
/* } */
/* #endif */

#endif /* PREDICTIVE_TIMER_2_H_ */
