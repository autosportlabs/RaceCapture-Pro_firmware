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

#include "CANMapTest.hh"
#include "CAN.h"
#include "CANMap.h"
#include "mock_serial.h"

// /* Inclue the code to test here */
// extern "C" {
// #include "CAN/CANMap.c"
// }

using std::string;

CPPUNIT_TEST_SUITE_REGISTRATION( CANMapTest );

void CANMapTest::setAndGetValueTest()
{
    CANMap_set_current_map_value(1, 99);
    CPPUNIT_ASSERT_EQUAL(CANMap_get_current_map_value(1), 99);
}

void CANMapTest::setOutOfBoundsTest()
{
    CANMap_set_current_map_value(101, 99);
}

void CANMapTest::getOutOfBoundsTest()
{
    CPPUNIT_ASSERT_EQUAL(CANMap_get_current_map_value(101), 0);
}

void CANMapTest::msgProcessTest()
{
    CANMapConfig cMapConf;
        cMapConf.enabled = 1;
        cMapConf.enabledChannels = 1;

    ChannelConfig chanCfg;
        strcpy(chanCfg.label, "Test Channel");;
        strcpy(chanCfg.units, "T");
        chanCfg.min = 0.0;
        chanCfg.max = 100.0;
        chanCfg.sampleRate = 100;
        chanCfg.precision = 0;
        chanCfg.flags = 0;

    CANMapChannelConfig mapChanCfg;
        mapChanCfg.cfg = chanCfg;
        mapChanCfg.canChannel = 0;
        mapChanCfg.canIdMask = 0xFF;
        mapChanCfg.canId = 1;
        mapChanCfg.byteStart = 0;
        mapChanCfg.byteLength = 1;
        mapChanCfg.dataMask = 0xFF;
        mapChanCfg.multipler = 1;
        mapChanCfg.divisor = 1;
        mapChanCfg.adder = 0;

    cMapConf.maps[0] = mapChanCfg;

    CAN_msg msg;
        msg.isExtendedAddress = 0;
        msg.addressValue = 1;
        msg.dataLength = 8;
        msg.data[0] = 100;
        msg.data[1] = 0;
        msg.data[2] = 0;
        msg.data[3] = 0;
        msg.data[4] = 0;
        msg.data[5] = 0;
        msg.data[6] = 0;
        msg.data[7] = 0;

    int value = 0;

    CPPUNIT_ASSERT_EQUAL(CAN_process_map(&cMapConf, &msg, &value), true);
    CPPUNIT_ASSERT_EQUAL(value, 100);
    CPPUNIT_ASSERT_EQUAL(CANMap_get_current_map_value(0), 100);
}
