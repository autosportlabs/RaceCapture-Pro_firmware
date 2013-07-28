/*
 * linearInterp.h
 *
 *  Created on: Jul 27, 2013
 *      Author: brent
 */

#ifndef LINEARINTERP_H_
#define LINEARINTERP_H_

//linear interpolation routine
//            (y2 - y1)
//  y = y1 +  --------- * (x - x1)
//            (x2 - x1)
float LinearInterpolate(float x, float x1, float y1, float x2, float y2);


#endif /* LINEARINTERP_H_ */
