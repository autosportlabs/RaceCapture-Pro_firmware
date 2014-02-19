/*
 * GpsLogReader.cpp
 *
 *  Created on: Feb 3, 2014
 *      Author: stieg
 */

#include "GpsLogReader.h"
#include <std.hh>

String str;

GpsLogReader::GpsLogReader(string fName) {

}

GpsLogReader::~GpsLogReader() {
	// TODO Auto-generated destructor stub
}

string GpsLogReader::readFile(string filename){
	ifstream t(filename.c_str());

	if (!t.is_open()){
		t.open(string(FILE_PREFIX + filename).c_str());
	}

	if (!t.is_open()){
		throw ("Can not find file " + filename);
	}

	string str;

	t.seekg(0, ios::end);
	int length = t.tellg();
	str.reserve(length);
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
				istreambuf_iterator<char>());
	return str;
}
