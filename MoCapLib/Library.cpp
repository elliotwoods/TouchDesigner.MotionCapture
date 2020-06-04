#include "pch_MoCapLib.h"
#include "Library.h"
#include "Ptr.h"

using namespace std;

namespace TD_MoCap
{
#pragma mark Library
	//----------
	Library::Library(const type_index & typeIndex)
	: typeIndex(typeIndex) {
		LibraryRegister::X().add(typeIndex, this);
	}

	//----------
	Library::~Library()
	{
		// The library is unloading

		// 1. Disallow any new lock requests
		// This stops new Ptrs and new Ptr::Locked's from being instantiated
		this->closing = true;

		// Wait until no Ptr::Locked exist ... and never release
		this->lockLibrary.lock();

		// Wait until we have exclusive access on our collection ... and never release
		this->lockRegisteredInstances.lock();

		// 2. Invalidate all existing instances
		for (auto instance : this->registeredInstances)
		{
			// NB : This function waits for existing items to go out of scope
			instance->reset();
		}

		LibraryRegister::X().remove(this->typeIndex);
	}

	//----------
	unique_ptr<unique_lock<mutex>>
		Library::lockForRegisterWriting()
	{
		auto lock = make_unique<unique_lock<mutex>>(this->lockRegisteredInstances, defer_lock);
		if (!lock->try_lock()) {
			if (this->closing) {
				return nullptr;
			}
			else {
				lock->lock();
			}
		}
		return lock;
	}

	//----------
	unique_ptr<shared_lock<std::shared_mutex>>
		Library::lockLibraryUse()
	{
		auto lock = make_unique<shared_lock<std::shared_mutex>>(this->lockLibrary, defer_lock);
		if (!lock->try_lock()) {
			if (this->closing) {
				return nullptr;
			}
			else {
				lock->lock();
			}
		}
		return lock;
	}

	//----------
	void
		Library::registerInstance(AbstractPtr* instance)
	{
		// This function is called after the lock has already been acquired externally
		this->registeredInstances.insert(instance);
	}

	//----------
	void
		Library::unregisterInstance(AbstractPtr* instance)
	{
		// Be sure to lock externally before calling this function
		this->registeredInstances.erase(instance);
	}

#pragma mark LibraryRegister
	//----------
	LibraryRegister &
		LibraryRegister::X()
	{
		static auto instance = make_unique<LibraryRegister>();
		return *instance;
	}

	//----------
	Library *
		LibraryRegister::get(const type_index& type)
	{
		shared_lock<shared_mutex> collectionLock(this->collectionMutex);

		auto findLibrary = this->libraries.find(type);
		if (findLibrary == this->libraries.end()) {
			return nullptr;
		}
		else {
			return findLibrary->second;
		}
	}

	//----------
	unique_ptr<unique_lock<mutex>>
		LibraryRegister::lockForRegisterWriting(const type_index& type)
	{
		shared_lock<shared_mutex> collectionLock(this->collectionMutex);
		
		auto findLibrary = this->libraries.find(type);
		if (findLibrary == this->libraries.end()) {
			return nullptr;
		}
		else {
			return findLibrary->second->lockForRegisterWriting();
		}
	}

	//----------
	void
		LibraryRegister::add(const type_index& type, Library* library)
	{
		unique_lock<shared_mutex> collectionLock(this->collectionMutex);
		this->libraries.emplace(type, library);
	}

	//----------
	void
		LibraryRegister::remove(const type_index& type)
	{
		unique_lock<shared_mutex> collectionLock(this->collectionMutex);
		this->libraries.erase(type);
	}
}