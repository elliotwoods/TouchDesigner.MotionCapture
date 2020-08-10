#include "pch_SOP_Threadto.h"
#include "SOP_ThreadTo.h"
#include "SOP_CPlusPlusBase.h" 

extern "C"
{
	DLLEXPORT
		void
		FillSOPPluginInfo(SOP_PluginInfo* info)
	{
		// This must always be set to this constant
		info->apiVersion = SOPCPlusPlusAPIVersion;

		// The opType is the unique name for this TOP. It must start with a 
		// capital A-Z character, and all the following characters must lower case
		// or numbers (a-z, 0-9)
		info->customOPInfo.opType->setString("Threadto");

		// The opLabel is the text that will show up in the OP Create Dialog
		info->customOPInfo.opLabel->setString("Thread To");

		// Will be turned into a 3 letter icon on the nodes
		info->customOPInfo.opIcon->setString("TT");

		// Information about the author of this OP
		info->customOPInfo.authorName->setString("Elliot Woods");
		info->customOPInfo.authorEmail->setString("elliot@kimchiandchips.com");

		// This TOP works with no inputs connected
		info->customOPInfo.minInputs = 0;
		info->customOPInfo.maxInputs = 0;
	}

	DLLEXPORT
		SOP_CPlusPlusBase*
		CreateSOPInstance(const OP_NodeInfo* info)
	{
		// Return a new instance of your class every time this is called.
		// It will be called once per TOP that is using the .dll
		return new TD_MoCap::SOP_ThreadTo(info);
	}

	DLLEXPORT
		void
		DestroySOPInstance(SOP_CPlusPlusBase* instance)
	{
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the TOP using that instance is deleted, or
		// if the TOP loads a different DLL
		delete (TD_MoCap::SOP_ThreadTo*)instance;
	}

};