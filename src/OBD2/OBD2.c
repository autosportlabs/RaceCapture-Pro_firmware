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
    return kph * 0.621371;
}

/*
 * PSIG is the measurement of pressure relative to ambient atmospheric 
 * pressure and is quantified in pounds per square inch gauge. 
 * This is used in almost all standard gauges leveraging a realtive to zero pressure environment.
 */
static float kPa_to_psig(float kPa)
{
    return (kPa * 0.1450377377)-14.6959494;
}

/* 
 * look into data bytes beyond 4
 * need 4F,50, 55,56,57,58,64
 */
static int decode_pid(unsigned char pid, CAN_msg *msg, int *value)
{
    int result = 0;

    if (msg->addressValue == STANDARD_PID_RESPONSE &&
        msg->data[0] >= 3 &&
        msg->data[1] == CUSTOM_MODE_SHOW_CURRENT_DATA &&
        msg->data[2] == pid ) {

        result = 1;

        int A = msg->data[3];
        int B = msg->data[4];
        int C = msg->data[5];
        int D = msg->data[6];

        switch(pid) {
            /* Calculated engine load */
            case 0x04:
                *value = A * 100 / 255;
                break;
            /* Engine coolant temperature (C) */
            case 0x05:
                *value = celcius_to_farenheight(A - 40);
                break;
            /* Short term fuel % trim - Bank 1 */
            /* Short term fuel % trim - Bank 1 */
            /* Short term fuel % trim - Bank 2 */
            /* Short term fuel % trim - Bank 2 */
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
                *value = (A - 128) * 100 / 128;
                break;
            /* Fuel pressure (KPa (gauge)) */
            case 0x0A:
                *value  = A * 3;
                break;
            /* Intake manifold pressure (KPa absolute) */
            case 0x0B:
                *value = A;
                break;
            /* RPM */
            case 0x0C:
                *value = ((A * 256) + B) / 4;
                break;
            /* Vehicle speed (km/ h) */
            case 0x0D:
                *value = kph_to_mph(A);
                break;
            /* Timing advance (degrees) */
            case 0x0E:
                *value = (A - 128) / 2;
                break;
            /* Intake air temperature (C) */
            case 0x0F:
                *value = celcius_to_farenheight(A - 40);
                break;
            /* MAF airflow rate (grams / sec) */
            case 0x10:
                *value = ((A * 256) + B) / 100;
                break;
            /* Throttle position % */
            case 0x11:
                *value = A * 100 / 255;
                break;
            /* 
             * Oxygen sensor voltage - Bank 1, Sensor 1
             * Oxygen sensor voltage - Bank 1, Sensor 2
             * Oxygen sensor voltage - Bank 1, Sensor 3
             * Oxygen sensor voltage - Bank 1, Sensor 4
             * Oxygen sensor voltage - Bank 2, Sensor 1
             * Oxygen sensor voltage - Bank 2, Sensor 2
             * Oxygen sensor voltage - Bank 2, Sensor 3
             * Oxygen sensor voltage - Bank 2, Sensor 4
             */
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1A:
            case 0x1B:
                *value = A / 200;
                break;
            /* Run time since engine start */
            case 0x1F:
                *value = (A * 256) + B;
                break;
            /* Fuel rail Pressure (relative to manifold vacuum) psi */
            case 0x22:
                *value = kPa_to_psig(((A * 256) + B) * 0.079);
                break;
            /* Fuel rail Pressure (diesel, or gasoline direct inject) psi */
            case 0x23:
                *value = kPa_to_psig(((A * 256) + B) * 10);
                break;
            /* Commanded evaporative purge % */
            case 0x2E:
                *value = A * 100 / 255;
                break;
            /* Fuel level input % */
            case 0x2F:
                *value = A * 100 / 255;
                break;
            /* Number of warm-ups since codes cleared */
            case 0x30:
                *value = A;
                break;
            /* Distance traveled since codes cleared - Validate km to miles */
            case 0x31:
                *value = kph_to_mph((A * 256) + B);
                break;
            /* Evap. System Vapor Pressure */
            case 0x32:
                *value = ((A * 256) + B) / 4;
                break;
            /* Barometric pressure psi */
            case 0x33:
                *value = kPa_to_psig(A);
                break;
            /* 
             * O2S1_WR_lambda Current
             * O2S2_WR_lambda Current
             * O2S3_WR_lambda Current
             * O2S4_WR_lambda Current
             * O2S5_WR_lambda Current
             * O2S6_WR_lambda Current
             * O2S7_WR_lambda Current
             * O2S8_WR_lambda Current
             */
            case 0x34:
            case 0x35:
            case 0x36:
            case 0x37:
            case 0x38:
            case 0x39:
            case 0x3A:
            case 0x3B:
                *value = ((C * 256) + D) / 256 - 128;
                break;
            /* 
             * Catalyst Temperature Bank 1, Sensor 1
             * Catalyst Temperature Bank 1, Sensor 2
             * Catalyst Temperature Bank 2, Sensor 1
             * Catalyst Temperature Bank 2, Sensor 1
             */
            case 0x3C:
            case 0x3D:
            case 0x3E:
            case 0x3F:
                *value = celcius_to_farenheight((( A * 256) + B) / 10 - 40);
                break;
            /* Control module voltage V */
            case 0x42:
                *value = ((A * 256) + B) / 1000;
                break;
             /* Absolute load value percent */
            case 0x43:
                *value = ((A * 256) + B) * 100 / 255;
                break;
            /* Fuel/Air commanded equivalence ratio */
            case 0x44:
                *value = ((A * 256) + B) / 32768;
                break;
            /* Relative throttle position percent */
            case 0x45:
                *value = A * 100 / 255;
                break;
            /* Ambient air temperature */
            case 0x46:
                *value = celcius_to_farenheight(A - 40);
                break;
            /* 
             * Absolute throttle position B percent
             * Absolute throttle position C percent
             * Absolute throttle position D percent
             * Absolute throttle position E percent
             * Absolute throttle position F percent
             */
            case 0x47:
            case 0x48:
            case 0x49:
            case 0x4A:
            case 0x4B:
                *value = A * 100 / 255;
                break;
            /* 
             * Time run with MIL on minutes
             * Time since trouble codes cleared minutes 
             */
            case 0x4D:
            case 0x4E:
                *value = (A * 256) + B;
                break;
            /* 
             * Fuel Types
             * 00	Not available
             * 01	Gasoline
             * 02	Methanol
             * 03	Ethanol
             * 04	Diesel
             * 05	LPG
             * 06	CNG
             * 07	Propane
             * 08	Electric
             * 09	Bifuel running Gasoline
             * 0A	Bifuel running Methanol
             * 0B	Bifuel running Ethanol
             * 0C	Bifuel running LPG
             * 0D	Bifuel running CNG
             * 0E	Bifuel running Propane
             */
            case 0x51:
                *value = A;
                break;
            /* Ethanol fuel percent */
            case 0x52:
                *value = A * 100 / 255;
                break;
            /* Absolute Evap system Vapor Pressure psi */
            case 0x53:
                *value = kPa_to_psig(((A * 256) + B) / 200);
                break;
             /* Evap system vapor pressure Pa */
            case 0x54:
                *value = ((A * 256) + B) - 32767;
                break;
            /* Fuel rail pressure (absolute) psi */
            case 0x59:
                *value = kPa_to_psig(((A * 256) + B) * 10);
                break;
            /* Relative accelerator pedal position percent */
            case 0x5A:
                *value = A * 100 / 255;
                break;
            /* Hybrid battery pack remaining life percent */
            case 0x5B:
                *value = A * 100 / 255;
                break;
            /* Engine oil temp (F) */
            case 0x5C:
                *value = celcius_to_farenheight(A - 40);
                break;
            /* Fuel injection timing degree */
            case 0x5D:
                *value = (((A * 256) + B) - 26880) / 128;
                break;
            /* Engine fuel rate l/h */
            case 0x5E:
                *value = ((A * 256) + B) * 0.05;
                break;
            /* 
             * Driver's demand engine - percent torque percent
             * Actual engine - percent torque percent
             */
            case 0x61:
            case 0x62:
                *value = A - 125;
                break;
            /* Engine reference torque - foot lbs */
            case 0x63:
                *value = ((A * 256) + B)*0.737562149277;
                break;
            /* Engine coolant temperature */
            case 0x67:
                *value = celcius_to_farenheight(A - 40);
                break;
            /* Intake air temperature sensor */
            case 0x68:
                *value = celcius_to_farenheight(A - 40);
                break;
            /* Exhaust gas recirculation temperature */
            case 0x6B:
                *value = celcius_to_farenheight(A - 40);
                break;
            /* Turbocharger compressor inlet pressure psi */
            case 0x6F:
                *value = kPa_to_psig(A);
                break;
            /* Boost pressure control in psi */
            case 0x70:
                *value = kPa_to_psig(A);
                break;
            /* Boost pressure control in psi */
            case 0x74:
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
