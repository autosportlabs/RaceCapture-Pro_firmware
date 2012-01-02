/*
 * newChannelProcessor.h
 *
 *  Created on: Oct 10, 2011
 *      Author: brent
 */

#ifndef NEWCHANNELPROCESSOR_H_
#define NEWCHANNELPROCESSOR_H_
#include "wx/wxprec.h"
#include "wx/wizard.h"

class AddChannelWizardParams;

class NewChannelProcessor {

public:
	virtual ~NewChannelProcessor(){}
	virtual void Process() {};
};

class NewChannelOptionsPageFactory{
public:
	virtual wxWizardPageSimple * createChannelProcessor(wxWizard * parent,  AddChannelWizardParams *params){ return NULL;}
	virtual ~NewChannelOptionsPageFactory() {}
};

#endif /* NEWCHANNELPROCESSOR_H_ */
