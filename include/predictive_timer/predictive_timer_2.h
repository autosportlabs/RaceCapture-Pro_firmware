/*
 * predective_timer_2.h
 *
 *  Created on: Jan 20, 2014
 *      Author: stieg
 */

#ifndef PREDICTIVE_TIMER_2_H_
#define PREDICTIVE_TIMER_2_H_

#include "dateTime.h"
#include "geopoint.h"

#include <stdbool.h>

/**
 * Method invoked whenever we detect that we have crossed the start finish line
 * @param point The location of the start/finish line.
 * @param time The time (millis) which the sample was taken.
 */
void startFinishCrossed(GeoPoint point, tiny_millis_t time);

/**
 * Adds a new GPS sample to our record if the algorithm determines its time for one.
 * @param point The point to add.
 * @param time The time (millis) which the sample was taken.
 * @return TRUE if it was added, FALSE otherwise.
 */
bool addGpsSample(GeoPoint point, tiny_millis_t time);

/**
 * Calculates the split of your current time against the fast lap time at the position given.
 * @param point The position you are currently at.
 * @param time The time (millis) which the sample was taken.
 * @return The split between your current time and the fast lap time.  Positive indicates you are
 * going faster than your fast lap, negative indicates slower.
 */
tiny_millis_t getSplitAgainstFastLap(GeoPoint point, tiny_millis_t time);

/**
 * Figures out the predicted lap time.  Call as much as you like... it will only do
 * calculations when new data is actually available.  This minimizes inaccuracies and
 * allows for drivers to better see how their most recent driving affected their predicted
 * lap time.
 * @param point The current location of the car.
 * @param time The time (millis) which the sample was taken.
 * @return The predicted lap time.
 */
tiny_millis_t getPredictedTime(GeoPoint point, tiny_millis_t time);

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

#endif /* PREDICTIVE_TIMER_2_H_ */
