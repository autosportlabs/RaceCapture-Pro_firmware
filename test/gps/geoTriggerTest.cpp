/**
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Stieg
 */

#include "geoCircle.h"
#include "geoTrigger.h"
#include "geoTriggerTest.h"
#include "geopoint.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( GeoTriggerTest );

void GeoTriggerTest::setUp() {}

void GeoTriggerTest::tearDown() {}

void GeoTriggerTest::testShouldTrigger() {
        const GeoPoint gp = { 43.074859, -89.386336 }; // 100 State
        const struct GeoCircle gc = gc_createGeoCircle(gp, 10);
        struct GeoTrigger gt = createGeoTrigger(&gc);

        CPPUNIT_ASSERT(!isGeoTriggerTripped(&gt));

        const GeoPoint gp2 = { 43.075255, -89.385590 }; // > 10 M from 100 State
        const bool status = updateGeoTrigger(&gt, &gp2);
        CPPUNIT_ASSERT(status);
        CPPUNIT_ASSERT(isGeoTriggerTripped(&gt));
}

void GeoTriggerTest::testNoTrigger() {
        const GeoPoint gp = { 43.074897, -89.386077 }; // 100 State
        const struct GeoCircle gc = gc_createGeoCircle(gp, 15);
        struct GeoTrigger gt = createGeoTrigger(&gc);

        CPPUNIT_ASSERT(!isGeoTriggerTripped(&gt));

        const GeoPoint gp2 = { 43.074918, -89.386037 }; // < 10 M from 100 State
        const bool status = updateGeoTrigger(&gt, &gp2);
        CPPUNIT_ASSERT(!status);
        CPPUNIT_ASSERT(!isGeoTriggerTripped(&gt));
}

void GeoTriggerTest::testReset() {
        const GeoPoint gp = { 43.074859, -89.386336 }; // 100 State
        const struct GeoCircle gc = gc_createGeoCircle(gp, 10);
        struct GeoTrigger gt = createGeoTrigger(&gc);

        CPPUNIT_ASSERT(!isGeoTriggerTripped(&gt));

        const GeoPoint gp2 = { 43.075255, -89.385590 }; // > 10 M from 100 State
        const bool status = updateGeoTrigger(&gt, &gp2);
        CPPUNIT_ASSERT(status);
        CPPUNIT_ASSERT(isGeoTriggerTripped(&gt));

        resetGeoTrigger(&gt);
        CPPUNIT_ASSERT(!isGeoTriggerTripped(&gt));
}
