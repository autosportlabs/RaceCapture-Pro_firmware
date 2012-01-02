/*
 * lapDetectorNewChannelProcessor.h
 *
 *  Created on: Oct 10, 2011
 *      Author: brent
 */

#ifndef LAPDETECTORNEWCHANNELPROCESSOR_H_
#define LAPDETECTORNEWCHANNELPROCESSOR_H_
#include "wx/wxprec.h"
#include "wx/wizard.h"
#include "newChannelProcessor.h"
#include "addChannelWizardDialog.h"

class LapDetectorNewChannelProcessor : public NewChannelProcessor{

public:
	void Process();
};

class LapDetectorOptionsPage : public wxWizardPageSimple{

public:
	LapDetectorOptionsPage(wxWizard *parent, AddChannelWizardParams *params);
	void OnWizardPageFinished(wxWizardEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageChanged(wxWizardEvent &event);
	DECLARE_EVENT_TABLE()

private:
	AddChannelWizardParams *m_params;
};

class LapDetectorNewChannelOptionsPageFactory : public NewChannelOptionsPageFactory{
public:
	virtual wxWizardPageSimple * createChannelProcessor(wxWizard *parent, AddChannelWizardParams *params){
		return new LapDetectorOptionsPage(parent,params);
	}
};


#endif /* LAPDETECTORNEWCHANNELPROCESSOR_H_ */
