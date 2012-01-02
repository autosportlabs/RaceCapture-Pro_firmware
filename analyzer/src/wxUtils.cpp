/*
 * wxUtils.cpp
 *
 *  Created on: Oct 9, 2011
 *      Author: brent
 */
#include "wxUtils.h"




int wxGridUtils::getSelectedRowCount(wxGrid *grid){
	int count = 0;
	size_t rowCount = grid->GetRows();
	for (size_t i = 0; i < rowCount; i++){
		if (grid->IsInSelection(i,0)) count++;
		}
	return count;
}
