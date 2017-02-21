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

CPP_GUARD_BEGIN


float map_value(CAN_msg *can_msg, CANMapping *mapping);

float apply_formula(float value, CANMapping *mapping);

float extract_value(uint64_t raw_data, CANMapping *mapping);

CPP_GUARD_END
#endif /* CAN_MAPPING_H_ */
