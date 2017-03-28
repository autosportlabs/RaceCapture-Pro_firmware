/*
 * units_conversion.h
 *
 *  Created on: Mar 27, 2017
 *      Author: brent
 */

#ifndef UNITS_CONVERSION_H_
#define UNITS_CONVERSION_H_

/**
 * Perform a units conversion for the specified units conversion id
 * @param id the units conversion id
 * @param value the value to convert
 * @return the converted value if the id is a valid conversion id; if not, the same value is returned (no conversion)
 **/
float convert_units(int id, const float value);

#endif /* UNITS_CONVERSION_H_ */
