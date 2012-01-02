/*
 * scaledValueNewChannelProcessor.h
 *
 *  Created on: Oct 10, 2011
 *      Author: brent
 */

#ifndef SCALEDVALUENEWCHANNELPROCESSOR_H_
#define SCALEDVALUENEWCHANNELPROCESSOR_H_
#include "wx/wxprec.h"
#include "newChannelProcessor.h"
#include "addChannelWizardDialog.h"

class ScaledValueNewChannelProcessor : public NewChannelProcessor{

public:
	void Process();
};

class ScaledValueOptionsPage : public wxWizardPageSimple{

public:
	ScaledValueOptionsPage(wxWizard *parent, AddChannelWizardParams *params);
	void OnWizardPageFinished(wxWizardEvent &event);
	void OnWizardPageChanging(wxWizardEvent &event);
	void OnWizardPageChanged(wxWizardEvent &event);
	DECLARE_EVENT_TABLE()

private:
	AddChannelWizardParams *m_params;
};

class ScaledValueNewChannelOptionsPageFactory : public NewChannelOptionsPageFactory{
public:
	virtual wxWizardPageSimple * createChannelProcessor(wxWizard *parent, AddChannelWizardParams *params){
		return new ScaledValueOptionsPage(parent,params);
	}
};

#endif /* SCALEDVALUENEWCHANNELPROCESSOR_H_ */
