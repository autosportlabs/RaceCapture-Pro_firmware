/**
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Stieg
 */

#include "geoCircle.h"
#include "geopoint.h"
#include "tracks.h"
#include "printk.h"
struct GeoCircle gc_createGeoCircle(const GeoPoint gp, const float r) {
   struct GeoCircle gc;

   gc.point = gp;
   gc.radius = r;

   return gc;
}

bool gc_isPointInGeoCircle(const GeoPoint point, const struct GeoCircle gc) {
	float dist = distPythag(&point, &(gc.point));
	pr_info("curr=");
	pr_info_float(point.latitude);
	pr_info(",");
	pr_info_float(point.longitude);
	pr_info("/target=");
	pr_info_float(gc.point.latitude);
	pr_info(",");
	pr_info_float(gc.point.longitude);
	pr_info("/rad=");
	pr_info_float(gc.radius);
	pr_info("; dist= ");
	pr_info_float(dist);
	pr_info("\r\n");

   return  dist <= gc.radius;
}

bool gc_isValidGeoCircle(const struct GeoCircle gc) {
   return isValidPoint(&(gc.point)) && gc.radius > 0.0;
}
