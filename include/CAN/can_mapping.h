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

/**
 * match the can message based on the specified CAN mapping ID and ID mask
 * @param can_msg the CAN message to test
 * @param mapping the can mapping to check against
 * @return true if the CAN message matches the mapping's CAN ID and ID mask
 */
bool canmapping_match_id(CAN_msg *can_msg, CANMapping *mapping);

/**
 * performs the full mapping against for the CAN message using the CAN mapping
 * @param value the mapped value is set in this parameter if the CAN mapping matches the specified message
 * @param can_msg the CAN message containing the raw data
 * @param mapping the mapping to be applied to the message
 * @return true if the mapping was successfully applied
 */
bool canmapping_map_value(float *value, CAN_msg *can_msg, CANMapping *mapping);

/**
 * apply the mapping's formula against the specified value
 * @param value the raw value being applied to the formula
 * @param the mapping containing the formula
 * @return the value after transformed by the formula
 */
float canmapping_apply_formula(float value, CANMapping *mapping);

/**
 * extract the raw value using the specified mapping
 * @param the raw data containing
 * @param the mapping that specifies the offset and bit length within the raw data
 * @return the extracted value
 */
float canmapping_extract_value(uint64_t raw_data, CANMapping *mapping);

CPP_GUARD_END
#endif /* CAN_MAPPING_H_ */
