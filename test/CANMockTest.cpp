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

#include "CANMockTest.hh"
#include "CAN.h"
#include "loggerConfig.h"
#include "macros.h"
#include <cppunit/extensions/HelperMacros.h>

// extern "C" {
// #include "logger_mock/CAN_device_mock.c"
// }

CPPUNIT_TEST_SUITE_REGISTRATION( CANMockTest );

using std::string;

void CANMockTest::rxMsgEmptyQueueTest()
{
	CAN_init_port(0, DEFAULT_CAN_BAUD_RATE);

	CAN_msg rxMsg;

	CPPUNIT_ASSERT_EQUAL(CAN_rx_msg(0, &rxMsg, 0), 0);
}

void CANMockTest::txMsgRxMsgSequentialMsgQueueTest()
{
	CAN_init_port(0, DEFAULT_CAN_BAUD_RATE);

	CAN_msg txMsg;
    txMsg.addressValue = 0x7df;
    txMsg.data[0] = 2;
    txMsg.data[1] = 1;
    txMsg.data[2] = 1; //pid
    txMsg.data[3] = 0x55;
    txMsg.data[4] = 0x55;
    txMsg.data[5] = 0x55;
    txMsg.data[6] = 0x55;
    txMsg.data[7] = 0x55;
    txMsg.dataLength = 8;
    txMsg.isExtendedAddress = 0;

	CAN_msg rxMsg;

	CPPUNIT_ASSERT(CAN_tx_msg(0, &txMsg, 0) == 1);
	CPPUNIT_ASSERT(CAN_rx_msg(0, &rxMsg, 0) == 1);

	CPPUNIT_ASSERT(rxMsg.addressValue == txMsg.addressValue);
}

void CANMockTest::txMsgQueueOverflowTest()
{
	CAN_init_port(0, DEFAULT_CAN_BAUD_RATE);

	CAN_msg txMsg;
    txMsg.addressValue = 0x7df;
    txMsg.data[0] = 2;
    txMsg.data[1] = 1;
    txMsg.data[2] = 1; //pid
    txMsg.data[3] = 0x55;
    txMsg.data[4] = 0x55;
    txMsg.data[5] = 0x55;
    txMsg.data[6] = 0x55;
    txMsg.data[7] = 0x55;
    txMsg.dataLength = 8;
    txMsg.isExtendedAddress = 0;

	for (int i = 0; i < 10; i++)
	{
		CPPUNIT_ASSERT_EQUAL(CAN_tx_msg(0, &txMsg, 0), 1);
	}

	CPPUNIT_ASSERT_EQUAL(CAN_tx_msg(0, &txMsg, 0), 0);
}
