/*
 * predective_timer_2.h
 *
 *  Created on: Jan 20, 2014
 *      Author: stieg
 */

#ifndef PREDICTIVE_TIMER_2_H_
#define PREDICTIVE_TIMER_2_H_

// HACK STUPID BOOLEAN STUFF AND MIXING C and C++.  YIKES!
#ifndef RCP_TESTING
	typedef int bool;
	#define false 0;
	#define true !false;
#endif

#include "geopoint.h"
/**
 * Method invoked whenever we detect that we have crossed the start finish line
 * @param point The location of the start/finish line.
 * @param startFinishTime The current UTC time when we crossed.
 */
void startFinishCrossed(GeoPoint point, float utcTime);

/**
 * Adds a new GPS sample to our record if the algorithm determines its time for one.
 * @param point The point to add.
 * @param utcTime The time which the sample was taken.
 * @return TRUE if it was added, FALSE otherwise.
 */
bool addGpsSample(GeoPoint point, float utcTime);

/**
 * Calculates the split of your current time against the fast lap time at the position given.
 * @param point The position you are currently at.
 * @param currentTime The current UTC wall time.
 * @return The split between your current time and the fast lap time.  Positive indicates you are
 * going faster than your fast lap, negative indicates slower.
 */
float getSplitAgainstFastLap(GeoPoint point, float currentTime);

/**
 * Figures out the predicted lap time.  Call as much as you like... it will only do
 * calculations when new data is actually available.  This minimizes inaccuracies and
 * allows for drivers to better see how their most recent driving affected their predicted
 * lap time.
 * @param point The current location of the car.
 * @param utcTime The current time of the most recent GPS fix.
 * @return The predicted lap time.
 */
float getPredictedTime(GeoPoint point, float utcTime);

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
