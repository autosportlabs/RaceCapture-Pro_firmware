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


#include "CAN.h"
#include "FreeRTOS.h"
#include "OBD2.h"
#include "loggerConfig.h"
#include "printk.h"
#include "task.h"
#include "taskUtil.h"


#define STANDARD_PID_RESPONSE 			0x7e8
#define CUSTOM_MODE_SHOW_CURRENT_DATA 	0x41

static int OBD2_current_values[OBD2_CHANNELS];

void OBD2_set_current_PID_value(size_t index, int value)
{
    if (index < OBD2_CHANNELS) {
        OBD2_current_values[index] = value;
    }
}

int OBD2_get_current_PID_value(int index)
{
    return OBD2_current_values[index];
}

static float celcius_to_farenheight(float celcius)
{
    return celcius * 1.8 + 32.0;
}

static float kph_to_mph(float kph)
{
    return kph * 1.60934;
}

static float kPa_to_psig(float kPa)
{
    return (kPa * 0.1450377377)-14.6959494;
}

//look into data bytes beyond 4
//need 4F,50, 55,56,57,58,64
static int decode_pid(unsigned char pid, CAN_msg *msg, int *value)
{
    int result = 0;

    if (	msg->addressValue == STANDARD_PID_RESPONSE &&
            msg->data[0] >= 3 &&
            msg->data[1] == CUSTOM_MODE_SHOW_CURRENT_DATA &&
            msg->data[2] == pid ) {
    	
        result = 1;
        
        int A = msg->data[3];
        int B = msg->data[4];
        int C = msg->data[5];
        int D = msg->data[6];

        switch(pid) {
        case 0x04: //calculated engine load
            *value = A * 100 / 255;
            break;
        case 0x05: //engine coolant temperature (C)
            *value = celcius_to_farenheight(A - 40);
            break;
        case 0x06: //short term fuel % trim - Bank 1
        case 0x07: //short term fuel % trim - Bank 1
        case 0x08: //short term fuel % trim - Bank 2
        case 0x09: //short term fuel % trim - Bank 2
            *value = (A - 128) * 100 / 128;
            break;
        case 0x0A: //fuel pressure (KPa (gauge))
            *value  = A * 3;
            break;
        case 0x0B: //intake manifold pressure (KPa absolute)
            *value = A;
            break;
        case 0x0C: //RPM
            *value = ((A * 256) + B) / 4;
            break;
        case 0x0D: //vehicle speed (km/ h)
            *value = kph_to_mph(A);
            break;
        case 0x0E: //timing advance (degrees)
            *value = (A - 128) / 2;
            break;
        case 0x0F: //Intake air temperature (C)
            *value = celcius_to_farenheight(A - 40);
            break;
        case 0x10: //MAF airflow rate (grams / sec)
            *value = ((A * 256) + B) / 100;
            break;
        case 0x11: //throttle position %
            *value = A * 100 / 255;
            break;
        case 0x14: //Oxygen sensor voltage - Bank 1, Sensor 1
        case 0x15: //Oxygen sensor voltage - Bank 1, Sensor 2
        case 0x16: //Oxygen sensor voltage - Bank 1, Sensor 3
        case 0x17: //Oxygen sensor voltage - Bank 1, Sensor 4
        case 0x18: //Oxygen sensor voltage - Bank 2, Sensor 1
        case 0x19: //Oxygen sensor voltage - Bank 2, Sensor 2
        case 0x1A: //Oxygen sensor voltage - Bank 2, Sensor 3
        case 0x1B: //Oxygen sensor voltage - Bank 2, Sensor 4
            *value = A / 200;
            break;
        case 0x1F: //Run time since engine start
            *value = (A * 256) + B;
            break;
        case 0x22: //Fuel rail Pressure (relative to manifold vacuum) kPa
            *value = ((A * 256) + B) * 0.079;
            break;
        case 0x23: //Fuel rail Pressure (diesel, or gasoline direct inject) kPa 
            *value = ((A * 256) + B) * 10;
            break;
        case 0x2E: //Commanded evaporative purge %
            *value = A * 100 / 255;
            break;
        case 0x2F: //fuel level input %
            *value = A * 100 / 255;
            break;
        case 0x30: //# of warm-ups since codes cleared
            *value = A;
            break;
        case 0x31: //Distance traveled since codes cleared - Validate km to miles
            *value = kph_to_mph((A * 256) + B);
            break;
        case 0x32: //Evap. System Vapor Pressure
            *value = ((A * 256) + B) / 4;
            break;
        case 0x33: //Barometric pressure kPa 
            *value = A;
            break;
        case 0x34: //O2S1_WR_lambda Current
        case 0x35: //O2S2_WR_lambda Current
        case 0x36: //O2S3_WR_lambda Current
        case 0x37: //O2S4_WR_lambda Current
        case 0x38: //O2S5_WR_lambda Current
        case 0x39: //O2S6_WR_lambda Current
        case 0x3A: //O2S7_WR_lambda Current
        case 0x3B: //O2S8_WR_lambda Current
            *value = ((C * 256) + D) / 256 - 128;
            break;
        case 0x3C: //Catalyst Temperature Bank 1, Sensor 1
        case 0x3D: //Catalyst Temperature Bank 1, Sensor 2
        case 0x3E: //Catalyst Temperature Bank 2, Sensor 1
        case 0x3F: //Catalyst Temperature Bank 2, Sensor 2
            *value = celcius_to_farenheight((( A * 256) + B) / 10 - 40);
            break;
        case 0x42: //Control module voltage V
            *value = ((A * 256) + B) / 1000;
            break;
        case 0x43: //Absolute load value %
            *value = ((A * 256) + B) * 100 / 255; 
            break;
        case 0x44: //Fuel/Air commanded equivalence ratio
            *value = ((A * 256) + B) / 32768; 
            break;
        case 0x45: //Relative throttle position %
            *value = A * 100 / 255; 
            break;
        case 0x46: //Ambient air temperature
            *value = celcius_to_farenheight(A - 40); 
            break;
        case 0x47: //Absolute throttle position B %
        case 0x48: //Absolute throttle position C %
        case 0x49: //Absolute throttle position D %
        case 0x4A: //Absolute throttle position E %
        case 0x4B: //Absolute throttle position F %
            *value = A * 100 / 255; 
            break;
        case 0x4D: //Time run with MIL on minutes
        case 0x4E: //Time since trouble codes cleared minutes
            *value = (A * 256) + B; 
            break;
        case 0x51: //Fuel Type - see table
            *value = A; 
            break;
        case 0x52: //Ethanol fuel %
            *value = A * 100 / 255; 
            break;
        case 0x53: //Absolute Evap system Vapor Pressure kPa
            *value = ((A * 256) + B) / 200; 
            break;
        case 0x54: //Evap system vapor pressure Pa
            *value = ((A * 256) + B) - 32767; 
            break;
        case 0x59: //Fuel rail pressure (absolute) kPa
            *value = ((A * 256) + B) * 10; 
            break;
        case 0x5A: //Relative accelerator pedal position % 
            *value = A * 100 / 255; 
            break;
        case 0x5B: //Hybrid battery pack remaining life % 
            *value = A * 100 / 255; 
            break;
        case 0x5C: //Engine oil temp (C)
            *value = celcius_to_farenheight(A - 40);
            break;
        case 0x5D: //Fuel injection timing degree
            *value = (((A * 256) + B) - 26880) / 128;
            break;
        case 0x5E: //Engine fuel rate l/h
            *value = ((A * 256) + B) * 0.05;
            break;
        case 0x61: //Driver's demand engine - percent torque %
        case 0x62: //Actual engine - percent torque %
            *value = A - 125;
            break;
        case 0x63: //Engine reference torque - foot lbs
            *value = ((A * 256) + B)*0.737562149277;
            break;    
        case 0x67: //Engine coolant temperature
            *value = celcius_to_farenheight(A - 40); 
            break;
        case 0x68: //Intake air temperature sensor
            *value = celcius_to_farenheight(A - 40); 
            break;
        case 0x6B: //Exhaust gas recirculation temperature
            *value = celcius_to_farenheight(A - 40); 
            break;
        case 0x6F: //Turbocharger compressor inlet pressure psi
            *value = kPa_to_psig(A); 
            break;
        case 0x70: //Boost pressure control in psi
            *value = kPa_to_psig(A); 
            break;
        case 0x74: //Boost pressure control in psi
            *value = kPa_to_psig(A); 
            break;
        default:
            result = 0;
            break;
        }
    }
    return result;
}

int OBD2_request_PID(unsigned char pid, int *value, size_t timeout)
{
    CAN_msg msg;
    msg.addressValue = 0x7df;
    msg.data[0] = 2;
    msg.data[1] = 1;
    msg.data[2] = pid;
    msg.data[3] = 0x55;
    msg.data[4] = 0x55;
    msg.data[5] = 0x55;
    msg.data[6] = 0x55;
    msg.data[7] = 0x55;
    msg.dataLength = 8;
    msg.isExtendedAddress = 0;

    int pid_request_success = 0;

    if (CAN_tx_msg(0, &msg, timeout)) {
        size_t start_time = xTaskGetTickCount();
        while (!isTimeoutMs(start_time, OBD2_PID_DEFAULT_TIMEOUT_MS)) {
            int result = CAN_rx_msg(0, &msg, OBD2_PID_DEFAULT_TIMEOUT_MS);
            if (result) {
                result = decode_pid(pid, &msg, value);
                if (result) {
                    pid_request_success = 1;
                    if (DEBUG_LEVEL) {
                        pr_debug("read OBD2 PID ");
                        pr_debug_int(pid);
                        pr_debug("=")
                        pr_debug_int(*value);
                        pr_debug("\r\n");
                    }
                    break;
                }
            }
        }
    }
    return pid_request_success;
}
