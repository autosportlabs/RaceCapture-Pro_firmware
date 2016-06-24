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

#include "channel_config.h"
#include <string.h>
#include "modp_numtoa.h"
#include "virtualChannel_test.h"
#include "virtual_channel.h"

#include <stdlib.h>

using std::string;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( VirtualChannelTest );

void VirtualChannelTest::setUp(){
	reset_virtual_channels();
}

void VirtualChannelTest::tearDown(){}

void VirtualChannelTest::testAddChannel(void){

	ChannelConfig cc = {"Blah","Units", 1.0f, 10.0f, SAMPLE_10Hz, 3};

	int id = create_virtual_channel(cc);
	VirtualChannel *vc = get_virtual_channel(id);
	CPPUNIT_ASSERT_EQUAL((string)"Blah", (string)vc->config.label);
	CPPUNIT_ASSERT_EQUAL(1.0f, vc->config.min);
	CPPUNIT_ASSERT_EQUAL(10.0f, vc->config.max);
	CPPUNIT_ASSERT_EQUAL((int)SAMPLE_10Hz, (int)vc->config.sampleRate);
	CPPUNIT_ASSERT_EQUAL((int)3, (int)vc->config.precision);
	size_t count = get_virtual_channel_count();
	CPPUNIT_ASSERT_EQUAL((size_t)1, count);
}

void VirtualChannelTest::testAddDuplicateChannel(void){
	ChannelConfig cc1 = {"Blah","Units", 1.0f, 10.0f, SAMPLE_10Hz, 3};
	ChannelConfig cc2 = {"Blah","Units", 1.0f, 10.0f, SAMPLE_10Hz, 3};
	int id1 = create_virtual_channel(cc1);
	int id2 = create_virtual_channel(cc2);
	CPPUNIT_ASSERT_EQUAL(id1, id2);  //duplicate channels with the same name should be prevented
	size_t count = get_virtual_channel_count();
	CPPUNIT_ASSERT_EQUAL((size_t)1, count);
}

void VirtualChannelTest::testAddChannelOverflow(void){

	const char *channel_prefix = "CH_";
	char channel_name_suffix[10];

	for (size_t i = 0; i < MAX_VIRTUAL_CHANNELS; i++){
		ChannelConfig cc = {"B","Units", 1.0f, 10.0f, SAMPLE_10Hz, 3};
		strcpy(cc.label, channel_prefix);
		modp_itoa10(i,channel_name_suffix);
		strcat(cc.label, channel_name_suffix);
		create_virtual_channel(cc);
	}

	//this is the overflow channel
	ChannelConfig cc = {"Overflow","Units", 1.0f, 10.0f, SAMPLE_10Hz, 3};
	int x = create_virtual_channel(cc);
	CPPUNIT_ASSERT_EQUAL(INVALID_VIRTUAL_CHANNEL, x); //response means we could not add channel

	size_t count = get_virtual_channel_count();
	CPPUNIT_ASSERT_EQUAL((size_t)MAX_VIRTUAL_CHANNELS, count);
}

void VirtualChannelTest::testSetChannelValue(void){
	ChannelConfig cc = {"Blah","Units", 1.0f, 10.0f, SAMPLE_10Hz, 3};
	int id = create_virtual_channel(cc);

	set_virtual_channel_value(id, 1234.56);
	float value = get_virtual_channel_value(id);
	CPPUNIT_ASSERT_EQUAL((float)1234.56, (float)value);
}
