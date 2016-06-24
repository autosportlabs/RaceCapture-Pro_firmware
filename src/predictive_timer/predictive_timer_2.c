/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "capabilities.h"
#include "dateTime.h"
#include "debug.h"
#include "geopoint.h"
#include "gps.h"
#include <string.h>
#include "predictive_timer_2.h"

/**
 * How frequently to initially take in GPS data.  To small and we overflow.  To large and we don't
 * update very frequently.
 */
#define INITIAL_POLL_INTERVAL 5000

/**
 * The absolute minimum predicted time.  This fixes issues related to predictive timing around the
 * Start/Finish Line.
 */
#define MIN_PREDICTED_TIME 10000

// A smaller TimeLoc value for space savings
struct PtTimeLoc {
    GeoPoint point;
    tiny_millis_t time;
};

/*
 * What is the maximum number of samples available per predictive time
 * buffer.  More samples == better resolution. Each slot is 12 bytes.
 * This is defined in capabilities.h
 */
static struct PtTimeLoc buff1[PREDICTIVE_TIME_MAX_SAMPLES];
static struct PtTimeLoc buff2[PREDICTIVE_TIME_MAX_SAMPLES];

// Our pointers that maintain the fast lap and current lap buffers.
static struct PtTimeLoc *currLap = buff1;
static struct PtTimeLoc *fastLap = buff2;

// Index to track current slot in buffer
static int buffIndex;

// Index to track high slot in fastLapTimer.  Its points to the next open slot there.
static int fastLapIndex;

// Time of the fast lap.
static tiny_millis_t fastLapTime;

// Time current lap started.
static tiny_millis_t currLapStartTime;

// Holds the lastPredictedTime.  Used for when we don't have good data to give yet.
static tiny_millis_t lastPredictedTime;

// Holds the last predicted Delta.  Used like lastPredictedTime.
static tiny_millis_t lastPredictedDelta;

// Interval between polls in milliseconds.
static tiny_millis_t pollInterval = INITIAL_POLL_INTERVAL;

// Indicates the current status of the recording code.  DISABLED until we start the first lap.
static enum Status {
    DISABLED, RECORDING, FULL,
} status = DISABLED;

/**
 * Gets the current lap time in seconds.
 * @param currentTime The current time in millis.
 * @return The current time of this lap in milliseconds.
 */
static tiny_millis_t getCurrentLapTime(tiny_millis_t time)
{
    return time - currLapStartTime;
}

/**
 * Creates a timeLoc sample and places it in the currBuff.  Increments counter as needed.
 * @return true if the insert succeeded, false otherwise.
 */
static bool insertTimeLocSample(const GeoPoint * point, tiny_millis_t time)
{
    if (buffIndex >= PREDICTIVE_TIME_MAX_SAMPLES)
        return false;

    struct PtTimeLoc *timeLoc = currLap + buffIndex;
    timeLoc->point = *point;
    timeLoc->time = getCurrentLapTime(time);

    if (++buffIndex >= PREDICTIVE_TIME_MAX_SAMPLES) {
        DEBUG("Buffer now Full!\n");
    }

    return true;
}

/**
 * Handles all the work done if a new hot Lap is set.
 * @param lapTime The time it took to complete the lap.
 */
static void setNewFastLap(tiny_millis_t lapTime)
{
    DEBUG("Setting new fast lap time to %f\n", lapTime);
    fastLapTime = lapTime;

    // Swap out our buffers.
    fastLapIndex = buffIndex;
    fastLap = currLap;
    currLap = currLap == buff1 ? buff2 : buff1;
}

bool isPredictiveTimeAvailable()
{
    return fastLapIndex != 0;
}

/**
 * Adjusts the poll interval so that we can effectively use our buffer.  The more full it
 * gets the better timing accuracy we can give.
 */
static tiny_millis_t adjustPollInterval(tiny_millis_t lapTime)
{
    // If no hotLap is set there no data to work with.
    if (!isPredictiveTimeAvailable())
        return pollInterval;

    // Target 90% buffer use +- 10%.
    const float slots = (float) PREDICTIVE_TIME_MAX_SAMPLES;
    const float percentUsed = ((float) buffIndex) / slots;
    DEBUG("Recorded %d samples.  Targeting ~ %f samples.\n", buffIndex, slots * 0.9);

    if (percentUsed > 0.8 && status != FULL) {
        DEBUG("Within target range.  Not adjusting sample rate.\n");
        return pollInterval;
    }

    // Careful here of gotchas with tiny_millis_t and floats.
    pollInterval = lapTime / (slots / 0.9);
    DEBUG("Setting poll interval to %ull\n", pollInterval);

    return pollInterval;
}

/**
 * @param time The time the sample was taken
 * @return The difference in seconds between our most recent sample a this new one.
 */
static tiny_millis_t getTimeSinceLastSample(tiny_millis_t time)
{
    return time - currLapStartTime - currLap[buffIndex - 1].time;
}

/**
 * Handles adding a sample at the end of the lap.  This is needed so we always
 * get an accurate reading, even if we run out of buffer space.
 */
void finishLap(const GpsSnapshot *gpsSnapshot)
{
    if (status == DISABLED) return;

    const tiny_millis_t time = gpsSnapshot->deltaFirstFix;
    const GeoPoint *point = &gpsSnapshot->sample.point;

    // Drop last entry if necessary to record end of lap.
    if (buffIndex >= PREDICTIVE_TIME_MAX_SAMPLES)
        buffIndex = PREDICTIVE_TIME_MAX_SAMPLES - 1;

    insertTimeLocSample(point, time);

    tiny_millis_t lapTime = getCurrentLapTime(time);
    INFO("Last lap time was %f seconds\n", lapTime);

    if (fastLapTime <= 0.0 || lapTime <= fastLapTime) {
        setNewFastLap(lapTime);
    }

    adjustPollInterval(lapTime);
    status = DISABLED;
}

/**
 * Resets the state in preparation for the next lap.  Inserts first sample
 */
void startLap(const GeoPoint *point, const tiny_millis_t time)
{
    if (status != DISABLED) return;

    status = RECORDING;
    currLapStartTime = time;
    lastPredictedDelta = 0;
    lastPredictedTime = 0;
    buffIndex = 0;

    DEBUG("Starting new lap.  Status %d, buffIndex = %d, startTime = %ull\n",
          status, buffIndex, time);

    insertTimeLocSample(point, time);
}

/**
 * Adds a new GPS sample to our record if the algorithm determines its time
 * for one.  Use this when we are not crossing the start or finish line.
 * @return true if it was added, false otherwise.
 */
bool addGpsSample(const GpsSnapshot *gpsSnapshot)
{
    const tiny_millis_t time = gpsSnapshot->deltaFirstFix;
    const GeoPoint *point = &gpsSnapshot->sample.point;

    DEVEL("Add GPS Sample called\n");

    if (status != RECORDING) {
        DEVEL("DROPPING - State is %d\n", status);
        return false;
    }

    // Check if enough time has elapsed between sample periods.
    if (getTimeSinceLastSample(time) < pollInterval) {
        DEVEL("DROPPING - elapsed < pollInterval\n");
        return false;
    }

    if (!insertTimeLocSample(point, time)) {
        status = FULL;
        DEVEL("DROPPING - Buffer full\n");
        return false;
    }

    DEBUG("Added sample  %f/%f @ %f\n", point.latitude, point.longitude, time);
    return true;
}

float distPctBtwnTwoPoints(const GeoPoint *s, const GeoPoint *e, const GeoPoint *m)
{
    const float distSM = distPythag(s, m); // A
    const float distME = distPythag(m, e); // B
    const float distSE = distPythag(s, e); // C

    // projDist = (A^2 + C^2 - B^2) / 2 * C
    const float projDistFromS = ((distSM * distSM) + (distSE * distSE)
                                 - (distME * distME)) / (2 * distSE);

    DEVEL("distSE = %f, distSM = %f, distME = %f, projDist = %f\n", distSE,
          distSM, distME, projDistFromS);

    return projDistFromS / distSE;
}

static bool inBounds(float v)
{
    return v >= 0 && v <= 1;
}

/**
 * Finds the  closest point to the given point in the fastLap buffer.  Orders the output buffer
 * such that the lower time is always first.
 * @param currPoint The current point of measurement.
 * @return The index of the closest point in the fastLap buffer to the current point, or -1 if
 * no closest point is available.
 */
static int findClosestPt(const GeoPoint *currPoint)
{
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
static bool findTwoClosestPts(const GeoPoint *currPoint, struct PtTimeLoc *tlPts[])
{
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
        struct PtTimeLoc *tmp = tlPts[0];
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
tiny_millis_t getSplitAgainstFastLap(const GeoPoint * point, tiny_millis_t currentTime)
{
    if (!isPredictiveTimeAvailable()) {
        DEBUG("No predicted time - No fast lap Set\n");
        return lastPredictedDelta;
    }

    /*
     * Figure out the two closest points.  Order of closestPts is with lower time first.  If this
     * fails then we can't continue.
     */
    struct PtTimeLoc *closestPts[2];
    if (!findTwoClosestPts(point, closestPts))
        // TODO: Perhaps return false here?  Make this better for the caller.
        return lastPredictedDelta;

    const GeoPoint *pointA = &(closestPts[0]->point);
    const GeoPoint *pointB = &(closestPts[1]->point);
    float percentage = distPctBtwnTwoPoints(pointA, pointB, point);
    DEVEL("Percentage value is 0 < %f < 1\n", percentage);

    if (!inBounds(percentage)) {
        DEVEL("Current Point is not between the two closest points.\n");
        return lastPredictedDelta;
    }

    const tiny_millis_t timeDeltaBtwnPoints = closestPts[1]->time - closestPts[0]->time;
    const tiny_millis_t estFastTime = closestPts[0]->time + timeDeltaBtwnPoints  * percentage;
    DEBUG("Estimated fast lap time at this point is %f\n", estFastTime);

    lastPredictedDelta = estFastTime - getCurrentLapTime(currentTime);
    DEBUG("Time Delta is %ull\n", lastPredictedDelta);
    return lastPredictedDelta;
}

/**
 * Figures out the predicted lap time.  Call as much as you like... it will only do
 * calculations when new data is actually available.  This minimizes inaccuracies and
 * allows for drivers to better see how their most recent driving affected their predicted
 * lap time.
 * @param point The current location of the car.
 * @param time The current time of the most recent GPS fix.
 * @return The predicted lap time.
 */
tiny_millis_t getPredictedTime(const GeoPoint * point, tiny_millis_t time)
{
        if (DISABLED == status)
                return 0;

        tiny_millis_t timeDelta = getSplitAgainstFastLap(point, time);
        tiny_millis_t newPredictedTime = fastLapTime - timeDelta;

        // Check for a minimum predicted time to deal with start/finish errors.
        if (newPredictedTime < MIN_PREDICTED_TIME)
                return lastPredictedTime;

        return lastPredictedTime = newPredictedTime;
}

/**
 * Just reset everything.
 */
void resetPredictiveTimer()
{
    DEBUG("Resetting predictive timer\n");
    status = DISABLED;
    buffIndex = 0;
    fastLapIndex = 0;
    fastLapTime = 0;
    lastPredictedTime = 0;
    lastPredictedDelta = 0;
    currLapStartTime = 0;
    pollInterval = INITIAL_POLL_INTERVAL;
}

float getPredictedTimeInMinutes()
{
    const GeoPoint gp = getGeoPoint();
    const tiny_millis_t millis = getMillisSinceFirstFix();
    return tinyMillisToMinutes(getPredictedTime(&gp, millis));
}
