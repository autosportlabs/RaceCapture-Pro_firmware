/*
 * predictive_timer_2.c
 *
 *  Created on: Jan 22, 2014
 *      Author: stieg
 */

#include "predictive_timer_2.h"

#include "debug.h"
#include "geopoint.h"
#include "mod_string.h"

/**
 * These settings control critical values that will affect performance.  Understand these values
 * before altering them.
 */
/**
 * # of Buffer slots per buffer.  Each slot is 12 bytes.  I want 1K RAM usage in per buffer.
 * So 1024/12 = 85.3 -> 85 slots.  Wheee math!
 */
#define MAX_TIMELOC_SAMPLES 85

/**
 * How frequently to initially take in GPS data.  To small and we overflow.  To large and we don't
 * update very frequently.
 */
#define INITIAL_POLL_INTERVAL 5.0

/**
 * The absolute minimum predicted time.  This fixes issues related to predictive timing around the
 * Start/Finish Line.
 */
#define MIN_PREDICTED_TIME 10.0

/**
 * A simple Time and Location sample.
 */
typedef struct _TimeLoc {
	GeoPoint point;
	float time;
} TimeLoc;

static TimeLoc buff1[MAX_TIMELOC_SAMPLES];
static TimeLoc buff2[MAX_TIMELOC_SAMPLES];

// Our pointers that maintain the fast lap and current lap buffers.
static TimeLoc *currLap = buff1;
static TimeLoc *fastLap = buff2;

// Index to track current slot in buffer
static int buffIndex;

// Index to track high slot in fastLapTimer.  Its points to the next open slot there.
static int fastLapIndex;

// Time of the fast lap.
static float fastLapTime;

// Time (in seconds) current lap started.
static float currLapStartTime;

// Holds the lastPredictedTime.  Used for when we don't have good data to give yet.
static float lastPredictedTime;

// Holds the last predicted Delta.  Used like lastPredictedTime.
static float lastPredictedDelta;

// Interval between polls in seconds.
static float pollInterval = INITIAL_POLL_INTERVAL;

// Indicates the current status of the recording code.  DISABLED until we start the first lap.
static enum Status {
	DISABLED, RECORDING, FULL,
} status = DISABLED;

/**
 * Gets the current lap time in seconds.
 * @param currentTime The current UTC time.
 * @return The current time of this lap in seconds.
 */
static float getCurrentLapTime(float utcTime) {
	return utcTime - currLapStartTime;
}

/**
 * Creates a timeLoc sample and places it in the currBuff.  Increments counter as needed.
 * @return true if the insert succeeded, false otherwise.
 */
static bool insertTimeLocSample(GeoPoint point, float utcTime) {
	if (buffIndex >= MAX_TIMELOC_SAMPLES)
		return false;

	TimeLoc *timeLoc = currLap + buffIndex;
	timeLoc->point = point;
	timeLoc->time = getCurrentLapTime(utcTime);

	if (++buffIndex >= MAX_TIMELOC_SAMPLES)
		DEBUG("Buffer now Full!\n");

	return true;
}

/**
 * Handles all the work done if a new hot Lap is set.
 * @param lapTime The time it took to complete the lap.
 */
static void setNewFastLap(float lapTime) {
	DEBUG("Setting new fast lap time to %f\n", lapTime);
	fastLapTime = lapTime;

	// Swap out our buffers.
	fastLapIndex = buffIndex;
	fastLap = currLap;
	currLap = currLap == buff1 ? buff2 : buff1;
}

bool isPredictiveTimeAvailable() {
	return fastLapIndex != 0;
}

/**
 * Adjusts the poll interval so that we can effectively use our buffer.  The more full it
 * gets the better timing accuracy we can give.
 */
static float adjustPollInterval(float lapTime) {
	// If no hotLap is set there no data to work with.
	if (!isPredictiveTimeAvailable())
		return pollInterval;

	// Target 90% buffer use +- 10%.
	float slots = (float) MAX_TIMELOC_SAMPLES;
	float percentUsed = buffIndex / slots;
	DEBUG("Recorded %d samples.  Targeting ~ %f samples.\n", buffIndex, MAX_TIMELOC_SAMPLES * 0.9);

	if (percentUsed > 0.8 && status != FULL) {
		DEBUG("Within target range.  Not adjusting sample rate.\n");
		return pollInterval;
	}


	pollInterval = lapTime / slots / 0.9;
	DEBUG("Setting poll interval to %f\n", pollInterval);

	return pollInterval;
}

/**
 * Handles adding a sample at the end of the lap.  This is needed so we always get an accurate
 * reading, even if we run out of buffer space.
 * @param point The point we are at when we cross the start finish line.
 * @param utcTime Duh!
 */
static void finishLap(GeoPoint point, float utcTime) {
	// Drop last entry if necessary to record end of lap.
	if (buffIndex >= MAX_TIMELOC_SAMPLES)
		buffIndex = MAX_TIMELOC_SAMPLES - 1;

	insertTimeLocSample(point, utcTime);
}

/**
 * Resets the state in preparation for the next lap.
 */
static void startNewLap(GeoPoint point, float utcTime) {
	currLapStartTime = utcTime;
	lastPredictedDelta = 0.0;
	lastPredictedTime = 0.0;
	buffIndex = 0;
	status = RECORDING;

	DEBUG("Starting new lap.  Status %d, buffIndex = %d, startTime = %f\n",
			status, buffIndex, utcTime);
	insertTimeLocSample(point, utcTime);
}

/**
 * @param utcTime The time the sample was taken
 * @return The difference in seconds between our most recent sample a this new one.
 */
static float getTimeSinceLastSample(float utcTime) {
	return utcTime - currLapStartTime - currLap[buffIndex - 1].time;
}

/**
 * Method invoked whenever we detect that we have crossed the start finish line.  Invoke this method
 * instead of invoking the addGpsSample method.  This has special state to handle corner cases.
 * @param point The location of the start/finish line.
 * @param utcTime The current UTC time when we crossed.
 */
void startFinishCrossed(GeoPoint point, float utcTime) {
	INFO("Start/Finish Crossed.\n");
	finishLap(point, utcTime);

	if (status != DISABLED) {
		float lapTime = getCurrentLapTime(utcTime);
		INFO("Last lap time was %f seconds\n", lapTime);

		if (fastLapTime <= 0.0 || lapTime <= fastLapTime)
			setNewFastLap(lapTime);

		adjustPollInterval(lapTime);
	}

	startNewLap(point, utcTime);
}

/**
 * Adds a new GPS sample to our record if the algorithm determines its time for one.  Use this when
 * we are not crossing the start/finish line.
 * @param point The point to add.
 * @param utcTime The time which the sample was taken.
 * @return true if it was added, false otherwise.
 */
bool addGpsSample(GeoPoint point, float utcTime) {
	DEVEL("Add GPS Sample called\n");

	if (status != RECORDING) {
		DEVEL("DROPPING - State is %d\n", status);
		return false;
	}

	// Check if enough time has elapsed between sample periods.
	if (getTimeSinceLastSample(utcTime) < pollInterval) {
		DEVEL("DROPPING - elapsed < pollInterval\n");
		return false;
	}

	if (!insertTimeLocSample(point, utcTime)) {
		status = FULL;
		DEVEL("DROPPING - Buffer full\n");
		return false;
	}

	DEBUG("Adding sample  %f/%f @ %f\n", point.latitude, point.longitude, utcTime);
	return true;
}

/**
 * Finds the percentage that currPt is between startPt and endPt.as a projection of point c onto the
 * vector formed between points s and e.  This method requires that point m be between the points s
 * and e on the earth's surface.  If that requirement is met this method should return a value
 * between 0 and 1.  Otherwise the value will be undefined.
 * those bounds then
 * @param s The start point.
 * @param e The end point.
 * @param m The middle point.
 * @return The percentage that projected point m lies between startPt and endPt if the method
 * requirements were met. < 0 or > 1 otherwise.
 */
static float distPctBtwnTwoPoints(GeoPoint *s, GeoPoint *e, GeoPoint *m) {
	float distSM = distPythag(s, m); // A
	float distME = distPythag(m, e); // B
	float distSE = distPythag(s, e); // C

	// projDist = (A^2 + C^2 - B^2) / 2 * C
	float projDistFromS = ((distSM * distSM) + (distSE * distSE)
			- (distME * distME)) / (2 * distSE);

	DEVEL("distSE = %f, distSM = %f, distME = %f, projDist = %f\n", distSE,
			distSM, distME, projDistFromS);

	return projDistFromS / distSE;
}

static bool inBounds(float v) {
	return v >= 0 && v <= 1;
}

/**
 * Finds the  closest point to the given point in the fastLap buffer.  Orders the output buffer
 * such that the lower time is always first.
 * @param currPoint The current point of measurement.
 * @return The index of the closest point in the fastLap buffer to the current point, or -1 if
 * no closest point is available.
 */
static int findClosestPt(GeoPoint *currPoint) {
	if (!isPredictiveTimeAvailable())
		return -1;

	// First find the closest point.  Start with index 0 as your best.
	int bestIndex = 0;
	GeoPoint *fastLapPoint = &(fastLap[bestIndex].point);
	float lowestDistance = distPythag(currPoint, fastLapPoint);

	for (int i = 1; i < fastLapIndex; ++i) {
		fastLapPoint = &(fastLap[i].point);
		float distance = distPythag(currPoint, fastLapPoint);

		if (distance < lowestDistance) {
			lowestDistance = distance;
			bestIndex = i;
		}
	}

	DEVEL("Smallest distance is %f from point %d\n", lowestDistance, bestIndex);
	return bestIndex;
}

/**
 * Finds the two points closest to the given point in the fastLap buffer.  Orders the output buffer
 * such that the lower time is always first.
 * @param currPoint The current point of measurement.
 * @param tlPts Output buffer where the two closest points will go.  Lower time point first.
 * Undefined values if method returns false.
 * @return true if a fast lap is set and the points are next to each other in the fastLap buffer
 * and the given point is between the two points, false otherwise.
 */
static bool findTwoClosestPts(GeoPoint *currPoint, TimeLoc *tlPts[]) {
	if (!isPredictiveTimeAvailable())
		return false;

	int bestIndex = findClosestPt(currPoint);
	if (bestIndex < 0)
		return false;

	/*
	 * Next we have two neighboring points.  We want to choose the point such that point s is before
	 * our current point which is before point e.  The current point we have may be s or e, we don't
	 * know.  So how do we find this point?  Use our distPctBtwnTwoPoints method.  Values between
	 * 0 - 1 indicate a point between the two points.
	 */
	GeoPoint *gpBest = &(fastLap[bestIndex].point);

	int upIdx = bestIndex + 1;
	int dnIdx = bestIndex - 1;

	GeoPoint *gpUp = upIdx >= fastLapIndex ? NULL : &(fastLap[upIdx].point);
	GeoPoint *gpDn = dnIdx < 0 ? NULL : &(fastLap[dnIdx].point);

	float distUp = gpUp == NULL ? -1 : distPctBtwnTwoPoints(gpBest, gpUp, currPoint);
	float distDn = gpDn == NULL ? -1 : distPctBtwnTwoPoints(gpBest, gpDn, currPoint);

	if (!inBounds(distUp) && !inBounds(distDn)) {
		DEBUG("Both points not in bounds (up: %f, dn: %f).  Close to Start/Finish?\n",
				distUp, distDn);
		return false;
	}

	int secondaryIndex = inBounds(distUp) ? upIdx : dnIdx;

	tlPts[0] = &(fastLap[bestIndex]);
	tlPts[1] = &(fastLap[secondaryIndex]);

	// Swap the buffers so the lower time is always first.  Just use
	if (tlPts[1]->time < tlPts[0]->time) {
		DEVEL("Swapping buffers: %f < %f\n", tlPts[1]->time, tlPts[0]->time);
		TimeLoc *tmp = tlPts[0];
		tlPts[0] = tlPts[1];
		tlPts[1] = tmp;
	}

	return true;
}

/**
 * Calculates the split of your current time against the fast lap time at the position given.
 * @param point The position you are currently at.
 * @param currentTime The current UTC wall time.
 * @return The split between your current time and the fast lap time.  Positive indicates you are
 * going faster than your fast lap, negative indicates slower.
 */
float getSplitAgainstFastLap(GeoPoint point, float currentTime) {
	if (!isPredictiveTimeAvailable()) {
		DEBUG("No predicted time - No fast lap Set\n");
		return lastPredictedDelta;
	}

	/*
	 * Figure out the two closest points.  Order of closestPts is with lower time first.  If this
	 * fails then we can't continue.
	 */
	TimeLoc *closestPts[2];
	if (!findTwoClosestPts(&point, closestPts))
		// TODO: Perhaps return false here?  Make this better for the caller.
		return lastPredictedDelta;

	GeoPoint *pointA = &(closestPts[0]->point);
	GeoPoint *pointB = &(closestPts[1]->point);
	float percentage = distPctBtwnTwoPoints(pointA, pointB, &point);
	DEVEL("Percentage value is 0 < %f < 1\n", percentage);

	if (!inBounds(percentage)) {
		DEVEL("Current Point is not between the two closest points.\n");
		return lastPredictedDelta;
	}

	float timeDeltaBtwnPoints = closestPts[1]->time - closestPts[0]->time;
	float estFastTime = closestPts[0]->time + timeDeltaBtwnPoints  * percentage;
	DEBUG("Estimated fast lap time at this point is %f\n", estFastTime);

	lastPredictedDelta = estFastTime - getCurrentLapTime(currentTime);
	DEBUG("Time Delta is %f\n", lastPredictedDelta);
	return lastPredictedDelta;
}

/**
 * Figures out the predicted lap time.  Call as much as you like... it will only do
 * calculations when new data is actually available.  This minimizes inaccuracies and
 * allows for drivers to better see how their most recent driving affected their predicted
 * lap time.
 * @param point The current location of the car.
 * @param utcTime The current time of the most recent GPS fix.
 * @return The predicted lap time.
 */
float getPredictedTime(GeoPoint point, float utcTime) {

	float timeDelta = getSplitAgainstFastLap(point, utcTime);
	float newPredictedTime = fastLapTime - timeDelta;

	// Check for a minimum predicted time to deal with start/finish errors.
	if (newPredictedTime >= MIN_PREDICTED_TIME){
		lastPredictedTime = newPredictedTime;
	}
	else{
		newPredictedTime = lastPredictedTime;
	}

	//convert to decimal minutes
	return newPredictedTime / 60000.0f;
}

/**
 * Just reset everything.
 */
void resetPredictiveTimer() {
	DEBUG("Resetting predictive timer\n");
	status = DISABLED;
	buffIndex = 0;
	fastLapIndex = 0;
	fastLapTime = 0;
	lastPredictedTime = 0.0;
	lastPredictedDelta = 0.0;
	currLapStartTime = 0;
	pollInterval = INITIAL_POLL_INTERVAL;
}
