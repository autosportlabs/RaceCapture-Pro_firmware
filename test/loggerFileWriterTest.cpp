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

#include "loggerFileWriterTest.hh"
#include "FreeRTOS.h"
#include "fileWriter.h"
#include "fileWriter_testing.h"
#include "mod_string.h"
#include "task.h"

#include <string>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LoggerFileWriterTest );

struct logging_status _ls;
struct logging_status *ls;

void LoggerFileWriterTest::setUp()
{
        _ls = (struct logging_status) { 0 };
        ls = &_ls;
}

void LoggerFileWriterTest::tearDown() {}

void LoggerFileWriterTest::testFlushLogfile()
{
        int rc;

        ls->writing_status = WRITING_INACTIVE;
        rc = flush_logfile(ls);
        CPPUNIT_ASSERT_EQUAL(1, rc);

        ls->writing_status = WRITING_ACTIVE;
        ls->flush_tick = FLUSH_INTERVAL_MS - 1;
        rc = flush_logfile(ls);
        CPPUNIT_ASSERT_EQUAL(2, rc);

        ls->flush_tick = FLUSH_INTERVAL_MS;
        rc = flush_logfile(ls);
        CPPUNIT_ASSERT_EQUAL(2, rc);
        CPPUNIT_ASSERT_EQUAL(xTaskGetTickCount(), ls->flush_tick);
}

void LoggerFileWriterTest::testLoggingStart()
{
        ls->logging = false;
        logging_start(ls);
        CPPUNIT_ASSERT_EQUAL(true, ls->logging);
}

void LoggerFileWriterTest::testLoggingStop()
{
        ls->logging = true;
        strcpy(ls->name, "Foo");
        logging_stop(ls);
        CPPUNIT_ASSERT_EQUAL(false, ls->logging);
        CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(ls->name));
}

void LoggerFileWriterTest::testLoggingSample()
{
        ls->logging = false;
        const int rc = logging_sample(ls, NULL);
        CPPUNIT_ASSERT_EQUAL(1, rc);
}

/*
 * TODO: Build in tests for file open and close methods.
 */
