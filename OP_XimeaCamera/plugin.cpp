#include "pch_OP_XimeaCamera.h"
#include "OP_XimeaCamera.h"
#include "XimeaCameraFrame.h"

namespace TD_MoCap {
	auto library = Library::make<XimeaCameraFrame>();
	auto ptr = make_ptr<XimeaCameraFrame>();
	auto lockedPtr = ptr.lock();
}

THIS IS A DEAD END! 
because a class might be defined in OP_Synchroniser but instantiated/owned by a Recorder!
we need to move all frame types to the shared library instead!

extern "C"
{

	DLLEXPORT
		void
		FillDATPluginInfo(DAT_PluginInfo* info)
	{
		// Always return DAT_CPLUSPLUS_API_VERSION in this function.
		info->apiVersion = DATCPlusPlusAPIVersion;

		// The opType is the unique name for this TOP. It must start with a
		// capital A-Z character, and all the following characters must lower case
		// or numbers (a-z, 0-9)
		info->customOPInfo.opType->setString("Mocapcamximea");

		// The opLabel is the text that will show up in the OP Create Dialog
		info->customOPInfo.opLabel->setString("MoCap Ximea Camera");

		// Will be turned into a 3 letter icon on the nodes
		info->customOPInfo.opIcon->setString("MCX");

		// Information about the author of this OP
		info->customOPInfo.authorName->setString("Elliot Woods");
		info->customOPInfo.authorEmail->setString("elliot@kimchiandchips.com");

		info->customOPInfo.minInputs = 0;
		info->customOPInfo.maxInputs = 0;
	}

	DLLEXPORT
		DAT_CPlusPlusBase*
		CreateDATInstance(const OP_NodeInfo* info)
	{
		// Return a new instance of your class every time this is called.
		// It will be called once per DAT that is using the .dll
		return new TD_MoCap::OP_XimeaCamera(info);
	}

	DLLEXPORT
		void
		DestroyDATInstance(DAT_CPlusPlusBase* instance)
	{
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the DAT using that instance is deleted, or
		// if the DAT loads a different DLL
		delete (TD_MoCap::OP_XimeaCamera*)instance;
	}

};