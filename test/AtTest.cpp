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

#include "AtTest.hh"
#include "at.h"
#include "cpp_guard.h"
#include "macros.h"
#include "mock_serial.h"
#include "serial.h"
#include "serial_buffer.h"
#include <stdbool.h>
#include <string.h>

/* Inclue the code to test here */
extern "C" {
#include "at.c"
}

using std::string;

#define DELIMETER	"\n"
#define QUIET_PERIOD_MS	10

CPPUNIT_TEST_SUITE_REGISTRATION( AtTest );

CPP_GUARD_BEGIN

static const size_t g_sb_buff_size = 64;
static char g_sb_buff[g_sb_buff_size];
static struct serial_buffer g_sb = {
        .serial = NULL,
        .length = g_sb_buff_size,
        .buffer = g_sb_buff,
};

static struct at_info g_ati;
static struct at_urc g_urc;

static bool g_cb_called;
static void *g_up;
bool cb(struct at_rsp *rsp, void *up) {
        g_cb_called = true;
        g_up = up;
        return false;
}

static bool g_sparse_urc_cb_called;
bool sparse_urc_cb(char* msg) {
        g_sparse_urc_cb_called = true;
        return false;
}

CPP_GUARD_END

void AtTest::setUp()
{
        setupMockSerial();
        struct Serial *s = getMockSerial();
        g_sb.serial = s;

        /* Always init our structs */
        at_info_init(&g_ati, &g_sb);
	at_configure_device(&g_ati, QUIET_PERIOD_MS, DELIMETER,
			    AT_DEV_CFG_FLAG_NONE);
	at_set_sparse_urc_cb(&g_ati, sparse_urc_cb);

        g_cb_called = false;
        g_up = NULL;
        g_sparse_urc_cb_called = false;
}

void AtTest::test_at_info_init()
{
        /* Setup some random data to ensure it gets wiped */
        g_ati.cmd_state = AT_CMD_STATE_IN_PROGRESS;
        g_ati.rx_state = AT_RX_STATE_CMD;
        g_ati.dev_cfg.quiet_period_ms = 42;
        g_ati.urc_list.count = 57;

        CPPUNIT_ASSERT_EQUAL(true, at_info_init(&g_ati, &g_sb));
        CPPUNIT_ASSERT_EQUAL(AT_CMD_STATE_READY, g_ati.cmd_state);
        CPPUNIT_ASSERT_EQUAL(AT_RX_STATE_READY, g_ati.rx_state);
        CPPUNIT_ASSERT_EQUAL((size_t) 0, g_ati.urc_list.count);
        CPPUNIT_ASSERT_EQUAL(&g_sb, g_ati.sb);
        CPPUNIT_ASSERT_EQUAL((void*) NULL, (void*) g_ati.sparse_urc_cb);

	CPPUNIT_ASSERT_EQUAL(AT_DEFAULT_QP_MS, g_ati.dev_cfg.quiet_period_ms);
	CPPUNIT_ASSERT_EQUAL(string(AT_DEFAULT_DELIMETER),
			     string(g_ati.dev_cfg.delim));
	CPPUNIT_ASSERT_EQUAL(AT_DEV_CFG_FLAG_NONE, g_ati.dev_cfg.flags);
}

void AtTest::test_at_info_init_failures()
{
        /* No Serial Buffer */
        CPPUNIT_ASSERT_EQUAL(false, at_info_init(&g_ati, NULL));

        /* No at_info struct */
        CPPUNIT_ASSERT_EQUAL(false, at_info_init(NULL, &g_sb));
}

void AtTest::test_set_sparse_urc_cb()
{
	at_info_init(&g_ati, &g_sb);
	CPPUNIT_ASSERT_EQUAL((void*) NULL, (void*) g_ati.sparse_urc_cb);

	at_set_sparse_urc_cb(&g_ati, sparse_urc_cb);
	CPPUNIT_ASSERT_EQUAL((void*) sparse_urc_cb,
			     (void*) g_ati.sparse_urc_cb);
}

void AtTest::test_at_put_cmd_full()
{
        g_ati.cmd_queue.count = AT_CMD_MAX_CMDS;
        CPPUNIT_ASSERT(!at_put_cmd(&g_ati, "F", 0, cb, NULL));
}

void AtTest::test_at_put_cmd_too_long()
{
        char cmd[AT_CMD_MAX_LEN + 1] = {};
        memset(cmd, 'A', AT_CMD_MAX_LEN);
        CPPUNIT_ASSERT(!at_put_cmd(&g_ati, cmd, 0, cb, NULL));
}

void AtTest::test_at_put_cmd_ok()
{
        tiny_millis_t to = 42;
        void *up = (void*) 1;
        const char *str = "ABC123";
        struct at_cmd *cmd = at_put_cmd(&g_ati, str, to, cb, up);
        CPPUNIT_ASSERT(cmd);

        CPPUNIT_ASSERT_EQUAL((size_t) 1, g_ati.cmd_queue.count);
        CPPUNIT_ASSERT_EQUAL(cmd, g_ati.cmd_queue.head);
        CPPUNIT_ASSERT_EQUAL(to, cmd->timeout_ms);
        CPPUNIT_ASSERT_EQUAL(&cb, cmd->rsp_cb);
        CPPUNIT_ASSERT_EQUAL(up, cmd->rsp_up);
        CPPUNIT_ASSERT(0 == strcmp(str, cmd->cmd));
}

void AtTest::test_at_get_next_cmd_ok()
{
        tiny_millis_t to = 46;
        const char *str = "123ABC";
        struct at_cmd *put_cmd = at_put_cmd(&g_ati, str, to, cb, NULL);
        struct at_cmd *get_cmd = at_task_get_next_cmd(&g_ati);

        CPPUNIT_ASSERT_EQUAL((size_t) 0, g_ati.cmd_queue.count);
        CPPUNIT_ASSERT_EQUAL(put_cmd, get_cmd);
}

void AtTest::test_at_get_next_cmd_no_cmd()
{
        CPPUNIT_ASSERT(!at_task_get_next_cmd(&g_ati));
}

void AtTest::test_cmd_ring_buff_logic()
{
        /*
         * Loop over the put and get commands a couple of times to ensure
         * that the values we get match what we expect.
         */
        tiny_millis_t to = 23;
        const char *str = "AT+BEERME NOW,SIERRA_NEVADA,TORPEDO";

        for (size_t i = 0; i < AT_CMD_MAX_CMDS * 2; ++i) {
                struct at_cmd *put_cmd = at_put_cmd(&g_ati, str, to, cb, NULL);
                struct at_cmd *get_cmd = at_task_get_next_cmd(&g_ati);
                CPPUNIT_ASSERT_EQUAL((size_t) 0, g_ati.cmd_queue.count);
                CPPUNIT_ASSERT_EQUAL(put_cmd, get_cmd);
        }
}

void AtTest::test_at_task_cmd_handler_bad_state()
{
        g_ati.cmd_state = AT_CMD_STATE_QUIET;
        CPPUNIT_ASSERT(!at_task_cmd_handler(&g_ati));
}

void AtTest::test_at_task_cmd_handler_no_cmd()
{
        /* By default we are in proper state without commands */
        CPPUNIT_ASSERT(!at_task_cmd_handler(&g_ati));
}

void AtTest::test_at_task_cmd_handler_ok()
{
        /* Puts a command */
        test_at_put_cmd_ok();

        CPPUNIT_ASSERT(at_task_cmd_handler(&g_ati));
        CPPUNIT_ASSERT(g_ati.cmd_ip);
        CPPUNIT_ASSERT_EQUAL(AT_CMD_STATE_IN_PROGRESS, g_ati.cmd_state);
        CPPUNIT_ASSERT_EQUAL(getUptime(), g_ati.timing.cmd_start_ms);
}


void AtTest::test_at_register_urc_full()
{
        g_ati.urc_list.count = AT_URC_MAX_URCS;
        CPPUNIT_ASSERT(!at_register_urc(&g_ati, "F", AT_URC_FLAGS_NONE,
                                        cb, NULL));
}

void AtTest::test_at_register_urc_too_long()
{
        char pfx[AT_URC_MAX_LEN + 1] = {};
        memset(pfx, 'U', AT_URC_MAX_LEN);
        CPPUNIT_ASSERT(!at_register_urc(&g_ati, pfx, AT_URC_FLAGS_NONE,
                                        cb, NULL));
}

void AtTest::test_at_register_urc_ok()
{
        const enum at_urc_flags flags = AT_URC_FLAGS_NO_RSP_STATUS;
        const char *pfx = "+FOOBAR:";
        void *up = (void*) 2;
        struct at_urc *urc = at_register_urc(&g_ati, pfx, flags, cb, up);
        CPPUNIT_ASSERT(urc);

        CPPUNIT_ASSERT_EQUAL((size_t) 1, g_ati.urc_list.count);
        CPPUNIT_ASSERT_EQUAL(urc, &g_ati.urc_list.urcs[0]);
        CPPUNIT_ASSERT_EQUAL(flags, urc->flags);
        CPPUNIT_ASSERT_EQUAL(&cb, urc->rsp_cb);
        CPPUNIT_ASSERT_EQUAL(up, urc->rsp_up);
        CPPUNIT_ASSERT(!strcmp(pfx, urc->pfx));
}


void AtTest::test_at_qp_handler_no_change()
{
        g_ati.cmd_state = AT_CMD_STATE_IN_PROGRESS;
        at_task_quiet_period_handler(&g_ati);
        CPPUNIT_ASSERT_EQUAL(AT_CMD_STATE_IN_PROGRESS, g_ati.cmd_state);

        g_ati.cmd_state = AT_CMD_STATE_QUIET;
        g_ati.timing.quiet_start_ms = getUptime();
        g_ati.dev_cfg.quiet_period_ms = 1;
        at_task_quiet_period_handler(&g_ati);
        CPPUNIT_ASSERT_EQUAL(AT_CMD_STATE_QUIET, g_ati.cmd_state);
}

void AtTest::test_at_qp_handler_change()
{
        g_ati.cmd_state = AT_CMD_STATE_QUIET;
        g_ati.timing.quiet_start_ms = getUptime();
        g_ati.dev_cfg.quiet_period_ms = 0;
        at_task_quiet_period_handler(&g_ati);
        CPPUNIT_ASSERT_EQUAL(AT_CMD_STATE_READY, g_ati.cmd_state);
}

void AtTest::test_at_begin_urc_msg()
{
        tiny_millis_t uptime = getUptime();
        begin_urc_msg(&g_ati, &g_urc);

        CPPUNIT_ASSERT_EQUAL(&g_urc, g_ati.urc_ip);
        CPPUNIT_ASSERT_EQUAL(uptime, g_ati.timing.urc_start_ms);
}

void AtTest::test_at_process_msg_generic()
{
        char msg[] = "FOOOOOO";
        const enum at_rx_state state = AT_RX_STATE_URC;
        _process_msg_generic(&g_ati, state, msg);

        CPPUNIT_ASSERT_EQUAL(state, g_ati.rx_state);
        CPPUNIT_ASSERT_EQUAL((char*) msg, g_ati.rsp.msgs[0]);
        CPPUNIT_ASSERT_EQUAL((size_t) 1, g_ati.rsp.msg_count);
}

void AtTest::test_at_process_msg_generic_too_many()
{
        char msg[] = "FOOOOOO";
        const enum at_rx_state state = AT_RX_STATE_URC;
        for(size_t i = 0; i < AT_RSP_MAX_MSGS; ++i)
                CPPUNIT_ASSERT(_process_msg_generic(&g_ati, state, msg));

        /* Now it should fail because we have too many messages */
        CPPUNIT_ASSERT(!_process_msg_generic(&g_ati, state, msg));
}

void AtTest::test_at_process_urc_msg_with_status()
{
        const enum at_urc_flags flags = AT_URC_FLAGS_NONE;
        char pfx[] = "+ABC123";
        struct at_urc *urc = at_register_urc(&g_ati, pfx, flags, cb, NULL);
        CPPUNIT_ASSERT(urc);

        begin_urc_msg(&g_ati, urc);
        process_urc_msg(&g_ati, pfx);

         /* No callback yet b/c no status msg yet */
        CPPUNIT_ASSERT(!g_cb_called);
        CPPUNIT_ASSERT_EQUAL(AT_RX_STATE_URC, g_ati.rx_state);

        char pfx_status[] = "OK";
        process_urc_msg(&g_ati, pfx_status);

        /* Now we expecte a callback b/c of status msg */
        CPPUNIT_ASSERT(g_cb_called);
        CPPUNIT_ASSERT_EQUAL(AT_RX_STATE_READY, g_ati.rx_state);
}

void AtTest::test_at_process_urc_msg_no_status()
{
        const enum at_urc_flags flags = AT_URC_FLAGS_NO_RSP_STATUS;
        char pfx[] = "+FOOBAR";
        struct at_urc *urc = at_register_urc(&g_ati, pfx, flags, cb, NULL);
        CPPUNIT_ASSERT(urc);

        begin_urc_msg(&g_ati, urc);
        process_urc_msg(&g_ati, pfx);

        /* We expecte a callback b/c of flags value */
        CPPUNIT_ASSERT(g_cb_called);
        CPPUNIT_ASSERT_EQUAL(AT_RX_STATE_READY, g_ati.rx_state);
}

void AtTest::test_at_process_cmd_msg()
{
        /* This sets up our command and start it */
        test_at_task_cmd_handler_ok();

        char msg[] = "+BAR: BAX,BIZ";
        process_cmd_msg(&g_ati, msg);
        CPPUNIT_ASSERT(g_ati.cmd_ip);
        CPPUNIT_ASSERT(!g_cb_called);
        CPPUNIT_ASSERT_EQUAL(AT_RX_STATE_CMD, g_ati.rx_state);

        char status[] = "FAIL";
        process_cmd_msg(&g_ati, status);
        /* We expecte a callback b/c of status message */
        CPPUNIT_ASSERT(g_cb_called);
        CPPUNIT_ASSERT(g_up); /* We set the user parameter in a prev test */
        CPPUNIT_ASSERT_EQUAL(AT_RX_STATE_READY, g_ati.rx_state);
}

void AtTest::test_at_task_run_bytes_read()
{
        /* This sets up our command and urc */
        test_at_task_cmd_handler_ok();
        const enum at_urc_flags flags = AT_URC_FLAGS_NONE;
        char pfx[] = "+FOOBAR";
        struct at_urc *urc = at_register_urc(&g_ati, pfx, flags, cb, NULL);
        CPPUNIT_ASSERT(urc);
        begin_urc_msg(&g_ati, urc);

        /* Just excersizes it... not much logic in there to test */
        g_ati.rx_state = AT_RX_STATE_CMD;
        char msg1[] = "Foo";
        at_task_run_bytes_read(&g_ati, msg1);

        /* Just excersizes it... not much logic in there to test */
        g_ati.rx_state = AT_RX_STATE_URC;
        at_task_run_bytes_read(&g_ati, msg1);

        /* Tests that we go into the URC state */
        g_ati.rx_state = AT_RX_STATE_READY;

        char msg2[] = "+FOOBAR: BLAH BLAH";
        at_task_run_bytes_read(&g_ati, msg2);
        CPPUNIT_ASSERT_EQUAL(AT_RX_STATE_URC, g_ati.rx_state);

        /* Tests that we go into the CMD state */
        g_ati.rx_state = AT_RX_STATE_READY;
        at_task_run_bytes_read(&g_ati, msg1);
        CPPUNIT_ASSERT_EQUAL(AT_RX_STATE_CMD, g_ati.rx_state);
}

void AtTest::test_is_urc_msg_none()
{
        char msg[] = "+FOO: BAZZ";
        CPPUNIT_ASSERT(!is_urc_msg(&g_ati, msg));
}

void AtTest::test_is_urc_msg_no_match()
{
        const enum at_urc_flags flags = AT_URC_FLAGS_NONE;
        const char *pfx = "+FOOBAR:";
        struct at_urc *urc = at_register_urc(&g_ati, pfx, flags, cb, NULL);
        CPPUNIT_ASSERT(urc);

        char msg[] = "+FOO: BAZZ";
        CPPUNIT_ASSERT(!is_urc_msg(&g_ati, msg));
}

void AtTest::test_urc_unhandled_cb()
{
        CPPUNIT_ASSERT(!g_ati.cmd_ip);

        char msg[] = "A,POOR,URC,MSG";
        process_cmd_or_urc_msg(&g_ati, msg);

        /*
         * Since no URC match, we should have gone into our unhandled URC
         * handler.  Ensure that we did.
         */
        CPPUNIT_ASSERT(g_sparse_urc_cb_called);
}

void AtTest::test_urc_unhandled_cb_cb_undefined()
{
        /* Like the test above, except cb is NULL */
        g_ati.sparse_urc_cb = NULL;

        CPPUNIT_ASSERT(!g_ati.sparse_urc_cb);
        CPPUNIT_ASSERT(!g_ati.cmd_ip);

        char msg[] = "A,POOR,URC,MSG";
        process_cmd_or_urc_msg(&g_ati, msg);

        /*
         * Since no URC match, we should have gone into our unhandled URC
         * handler.  However since one is not defined we should not have
         * made it in there.
         */
        CPPUNIT_ASSERT(!g_sparse_urc_cb_called);
}


void AtTest::test_is_urc_msg_match()
{
        const enum at_urc_flags flags = AT_URC_FLAGS_NONE;
        const char *pfx = "+FOOBAR:";
        struct at_urc *urc = at_register_urc(&g_ati, pfx, flags, cb, NULL);
        CPPUNIT_ASSERT(urc);

        char msg[] = "+FOOBAR: STATUS, STATUS, STATUS";
        CPPUNIT_ASSERT(is_urc_msg(&g_ati, msg));
}

void AtTest::test_is_rsp_status_nope()
{
        enum at_rsp_status s;

        char msg1[] = "NOT_A_STATUS";
        CPPUNIT_ASSERT(!is_rsp_status(&s, msg1));

        char msg2[] = "NOT OK";
        CPPUNIT_ASSERT(!is_rsp_status(&s, msg2));
}

void AtTest::test_is_rsp_status_ok()
{
        for(size_t i = 0; i < ARRAY_LEN(at_status_msgs); ++i) {
                enum at_rsp_status s;
                CPPUNIT_ASSERT(is_rsp_status(&s, at_status_msgs[i].str));
                CPPUNIT_ASSERT_EQUAL(s, at_status_msgs[i].status);
        }
}

void AtTest::test_complete_cmd()
{
        g_ati.rx_state = AT_RX_STATE_CMD;

        /* Register and start a command */
        test_at_task_cmd_handler_ok();
        CPPUNIT_ASSERT(g_ati.cmd_ip);

        complete_cmd(&g_ati, AT_RSP_STATUS_OK);

        CPPUNIT_ASSERT(!g_ati.cmd_ip);
        CPPUNIT_ASSERT_EQUAL(AT_CMD_STATE_QUIET, g_ati.cmd_state);
        CPPUNIT_ASSERT_EQUAL(getUptime(), g_ati.timing.quiet_start_ms);
        CPPUNIT_ASSERT(g_cb_called);
}

void AtTest::test_complete_urc()
{
        g_ati.rx_state = AT_RX_STATE_URC;

        /* Fake Start a URC */
        const enum at_urc_flags flags = AT_URC_FLAGS_NO_RSP_STATUS;
        char pfx[] = "+FOOBAR";
        void *up = (void*) 3;
        struct at_urc *urc = at_register_urc(&g_ati, pfx, flags, cb, up);
        CPPUNIT_ASSERT(urc);
        begin_urc_msg(&g_ati, urc);

        complete_urc(&g_ati, AT_RSP_STATUS_OK);

        CPPUNIT_ASSERT(!g_ati.urc_ip);
        CPPUNIT_ASSERT_EQUAL(AT_CMD_STATE_READY, g_ati.cmd_state);
        CPPUNIT_ASSERT(g_cb_called);
        CPPUNIT_ASSERT_EQUAL(up, g_up);
}

void AtTest::test_is_timed_out_fail()
{
        CPPUNIT_ASSERT(!is_timed_out(getUptime(), 1));
}

void AtTest::test_is_timed_out_ok()
{
        CPPUNIT_ASSERT(is_timed_out(getUptime(), 0));
}

void AtTest::test_at_configure_device()
{
        const tiny_millis_t qp = 5;
        const char delim[] = "\b";

        at_configure_device(&g_ati, qp, delim, AT_DEV_CFG_FLAG_NONE);

        CPPUNIT_ASSERT_EQUAL(qp, g_ati.dev_cfg.quiet_period_ms);
        CPPUNIT_ASSERT(!strcmp(delim, g_ati.dev_cfg.delim));
}

void AtTest::test_at_configure_device_returns()
{
	/* A NULL delimeter should yield a false result */
	CPPUNIT_ASSERT_EQUAL(false, at_configure_device(
				     &g_ati, 1, NULL, AT_DEV_CFG_FLAG_NONE));

	/* A delimeter that is too large should cause a failure */
	char delim[AT_DEV_CVG_DELIM_MAX_LEN + 1];
	memset(delim, '\n', sizeof(delim));
	CPPUNIT_ASSERT_EQUAL(false, at_configure_device(
				     &g_ati, 2, delim, AT_DEV_CFG_FLAG_NONE));
}

void AtTest::test_at_ok()
{
        CPPUNIT_ASSERT(!at_ok(NULL));

        struct at_rsp rsp;
        rsp.status = AT_RSP_STATUS_ERROR;
        CPPUNIT_ASSERT(!at_ok(&rsp));

        rsp.status = AT_RSP_STATUS_NONE;
        CPPUNIT_ASSERT(!at_ok(&rsp));

        rsp.status = AT_RSP_STATUS_OK;
        CPPUNIT_ASSERT(at_ok(&rsp));
}

void AtTest::test_at_parse_rsp_line()
{
        char *bkts[6] = {};

        char rsp1[] = "+CWJAP:\"madworks\",\"2a:a4:3c:6d:46:37\",6,-53";
        const size_t cnt1 = at_parse_rsp_line(rsp1, bkts, 6);

        // printf("[ %s | %s | %s | %s | %s | %s ]\n", bkts[0], bkts[1], bkts[2],
        //        bkts[3], bkts[4], bkts[5]);

        CPPUNIT_ASSERT_EQUAL((size_t) 5, cnt1);
        CPPUNIT_ASSERT(!strcmp("+CWJAP", bkts[0]));
        CPPUNIT_ASSERT(!strcmp("\"madworks\"", bkts[1]));
        CPPUNIT_ASSERT(!strcmp("\"2a:a4:3c:6d:46:37\"", bkts[2]));
        CPPUNIT_ASSERT(!strcmp("6", bkts[3]));
        CPPUNIT_ASSERT(!strcmp("-53", bkts[4]));

        memset(bkts, 0, ARRAY_LEN(bkts));
        char rsp2[] = "+CWJAP:\"mad\\\"works\",,\"2a:a4:3c:6d:46:37\",6,-53";
        const size_t cnt2 = at_parse_rsp_line(rsp2, bkts, 6);

        // printf("[ %s | %s | %s | %s | %s | %s ]\n", bkts[0], bkts[1], bkts[2],
        //        bkts[3], bkts[4], bkts[5]);

        CPPUNIT_ASSERT_EQUAL((size_t) 6, cnt2);
        CPPUNIT_ASSERT(!strcmp("+CWJAP", bkts[0]));
        CPPUNIT_ASSERT(!strcmp("\"mad\\\"works\"", bkts[1]));
        CPPUNIT_ASSERT(!strcmp("", bkts[2]));
        CPPUNIT_ASSERT(!strcmp("\"2a:a4:3c:6d:46:37\"", bkts[3]));
        CPPUNIT_ASSERT(!strcmp("6", bkts[4]));
        CPPUNIT_ASSERT(!strcmp("-53", bkts[5]));

        char rsp3[] = "+CWMODE:2";
        const size_t cnt3 = at_parse_rsp_line(rsp3, bkts, 6);
        CPPUNIT_ASSERT_EQUAL((size_t) 2, cnt3);
        CPPUNIT_ASSERT(!strcmp("+CWMODE", bkts[0]));
        CPPUNIT_ASSERT(!strcmp("2", bkts[1]));

        CPPUNIT_ASSERT_EQUAL((size_t) 0, at_parse_rsp_line(NULL, bkts, 6));
        CPPUNIT_ASSERT_EQUAL((size_t) 0, at_parse_rsp_line(rsp3, NULL, 6));
        CPPUNIT_ASSERT_EQUAL((size_t) 0, at_parse_rsp_line(rsp3, bkts, 0));
}

void AtTest::test_at_parse_rsp_str()
{
        char str1[] = "\"madworks\"";
        char *rsp1 = at_parse_rsp_str(str1);
        CPPUNIT_ASSERT_EQUAL(str1 + 1, rsp1);
        CPPUNIT_ASSERT(!strcmp("madworks", rsp1));

        char str2[] = "\"mad\\\"works\"";
        char *rsp2 = at_parse_rsp_str(str2);
        CPPUNIT_ASSERT_EQUAL(str2 + 1, rsp2);
        CPPUNIT_ASSERT(!strcmp("mad\\\"works", rsp2));

        char str3[] = "\"mad\\\"works";
        CPPUNIT_ASSERT(NULL == at_parse_rsp_str(str3));

        char str4[] = "madworks";
        CPPUNIT_ASSERT(NULL == at_parse_rsp_str(str4));

        char str5[] = "(\"foo\"bar\"baz\")";
        char *rsp5 = at_parse_rsp_str(str5);
        CPPUNIT_ASSERT_EQUAL(str5 + 2, rsp5);
        CPPUNIT_ASSERT(!strcmp("foo", rsp5));
}
