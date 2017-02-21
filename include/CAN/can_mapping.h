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

bool canmapping_match_id(CAN_msg *can_msg, CANMapping *mapping);

bool canmapping_map_value(float *value, CAN_msg *can_msg, CANMapping *mapping);

float canmapping_apply_formula(float value, CANMapping *mapping);

float canmapping_extract_value(uint64_t raw_data, CANMapping *mapping);

CPP_GUARD_END
#endif /* CAN_MAPPING_H_ */
