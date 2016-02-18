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

#include "CellularApiStatusKeysTest.hh"
#include "cellular_api_status_keys.h"

#include <limits.h>
#include <string.h>

/* Inclue the code to test here */
extern "C" {
#include "cellular_api_status_keys.c"
}

CPPUNIT_TEST_SUITE_REGISTRATION( CellularApiStatusKeysTest );

#define CPPUNIT_ASSERT_EQUAL_CONST_CHAR(a, b) \
        CPPUNIT_ASSERT_EQUAL(std::string(a), std::string(b))

void CellularApiStatusKeysTest::net_status_api_key_test()
{
        const char* val;

        val = cellular_net_status_api_key(CELLULAR_NETWORK_NOT_SEARCHING);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("stopped_searching", val);

        val = cellular_net_status_api_key(CELLULAR_NETWORK_SEARCHING);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("searching", val);

        val = cellular_net_status_api_key(CELLULAR_NETWORK_DENIED);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("denied", val);

        val = cellular_net_status_api_key(CELLULAR_NETWORK_REGISTERED);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("registered", val);

        val = cellular_net_status_api_key(CELLULAR_NETWORK_STATUS_UNKNOWN);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("unknown", val);

        val = cellular_net_status_api_key(
                (const enum cellular_net_status) INT_MIN);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("unknown", val);
}

void CellularApiStatusKeysTest::telemetry_status_api_key_test()
{
        const char* val;

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_IDLE);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("idle", val);

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_CONNECTED);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("connected", val);

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_CURRENT_CONNECTION_TERMINATED);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("terminated", val);

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_REJECTED_DEVICE_ID);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("rejected", val);

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_SERVER_CONNECTION_FAILED);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("connect_failed", val);

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_INTERNET_CONFIG_FAILED);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("data_failed", val);

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_CELL_REGISTRATION_FAILED);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("registration_failed", val);

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_MODEM_INIT_FAILED);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("init_failed", val);

        val = cellular_telemetry_status_api_key(
                TELEMETRY_STATUS_DATA_PLAN_NOT_AVAILABLE);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("unknown", val);

        val = cellular_telemetry_status_api_key(
                (const telemetry_status_t) INT_MIN);
        CPPUNIT_ASSERT_EQUAL_CONST_CHAR("unknown", val);
}
