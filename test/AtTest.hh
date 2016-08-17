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

#ifndef _ATTEST_H_
#define _ATTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class AtTest : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( AtTest );

        CPPUNIT_TEST( test_at_info_init );
        CPPUNIT_TEST( test_at_info_init_failures );
	CPPUNIT_TEST( test_set_sparse_urc_cb );
        CPPUNIT_TEST( test_at_put_cmd_full );
        CPPUNIT_TEST( test_at_put_cmd_too_long );
        CPPUNIT_TEST( test_at_put_cmd_ok );
        CPPUNIT_TEST( test_at_get_next_cmd_no_cmd );
        CPPUNIT_TEST( test_cmd_ring_buff_logic );
        CPPUNIT_TEST( test_at_task_cmd_handler_bad_state );
        CPPUNIT_TEST( test_at_task_cmd_handler_no_cmd );
        CPPUNIT_TEST( test_at_task_cmd_handler_ok );
        CPPUNIT_TEST( test_at_register_urc_full );
        CPPUNIT_TEST( test_at_register_urc_too_long );
        CPPUNIT_TEST( test_at_register_urc_ok );
        CPPUNIT_TEST( test_at_qp_handler_no_change );
        CPPUNIT_TEST( test_at_qp_handler_change );
        CPPUNIT_TEST( test_at_begin_urc_msg);
        CPPUNIT_TEST( test_at_process_msg_generic );
        CPPUNIT_TEST( test_at_process_msg_generic_too_many );
        CPPUNIT_TEST( test_at_process_urc_msg_with_status );
        CPPUNIT_TEST( test_at_process_urc_msg_no_status );
        CPPUNIT_TEST( test_at_process_cmd_msg );
        CPPUNIT_TEST( test_at_task_run_bytes_read );
        CPPUNIT_TEST( test_is_urc_msg_none );
        CPPUNIT_TEST( test_is_urc_msg_no_match );
        CPPUNIT_TEST( test_is_urc_msg_match );
        CPPUNIT_TEST( test_urc_unhandled_cb );
        CPPUNIT_TEST( test_urc_unhandled_cb_cb_undefined );
        CPPUNIT_TEST( test_is_rsp_status_nope );
        CPPUNIT_TEST( test_is_rsp_status_ok );
        CPPUNIT_TEST( test_complete_cmd );
        CPPUNIT_TEST( test_complete_urc );
        CPPUNIT_TEST( test_is_timed_out_fail );
        CPPUNIT_TEST( test_is_timed_out_ok );
        CPPUNIT_TEST( test_at_configure_device );
	CPPUNIT_TEST( test_at_configure_device_returns );
        CPPUNIT_TEST( test_at_ok );
        CPPUNIT_TEST( test_at_parse_rsp_line );
        CPPUNIT_TEST( test_at_parse_rsp_str );

        CPPUNIT_TEST_SUITE_END();

public:
        void setUp();
        void test_at_info_init();
        void test_at_info_init_failures();
	void test_set_sparse_urc_cb();
        void test_at_put_cmd_full();
        void test_at_put_cmd_too_long();
        void test_at_put_cmd_ok();
        void test_at_get_next_cmd_ok();
        void test_at_get_next_cmd_no_cmd();
        void test_cmd_ring_buff_logic();
        void test_at_task_cmd_handler_bad_state();
        void test_at_task_cmd_handler_no_cmd();
        void test_at_task_cmd_handler_ok();
        void test_at_register_urc_full();
        void test_at_register_urc_too_long();
        void test_at_register_urc_ok();
        void test_at_qp_handler_no_change();
        void test_at_qp_handler_change();
        void test_at_begin_urc_msg();
        void test_at_process_msg_generic();
        void test_at_process_msg_generic_too_many();
        void test_at_process_urc_msg_with_status();
        void test_at_process_urc_msg_no_status();
        void test_at_process_cmd_msg();
        void test_at_task_run_bytes_read();
        void test_is_urc_msg_none();
        void test_is_urc_msg_no_match();
        void test_is_urc_msg_match();
        void test_urc_unhandled_cb();
        void test_urc_unhandled_cb_cb_undefined();
        void test_is_rsp_status_nope();
        void test_is_rsp_status_ok();
        void test_complete_cmd();
        void test_complete_urc();
        void test_is_timed_out_fail();
        void test_is_timed_out_ok();
        void test_at_configure_device();
	void test_at_configure_device_returns();
        void test_at_ok();
        void test_at_parse_rsp_line();
        void test_at_parse_rsp_str();
};

#endif /* _ATTEST_H_ */
