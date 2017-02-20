/*
 * can_mapping.h
 *
 *  Created on: Feb 19, 2017
 *      Author: brent
 */

#ifndef CAN_MAPPING_H_
#define CAN_MAPPING_H_

#include "loggerConfig.h"
#include "CAN.h"
#include <byteswap.h>

float map_value(CAN_msg *can_msg, CANMapping *mapping);

#endif /* CAN_MAPPING_H_ */
