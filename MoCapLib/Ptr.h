#pragma once

#include "Library.h"

#include <memory>

// scenario : library is unloading but hasn't unloaded yet
// * should not be able to lock a ptr at this stage
// * library unloading should wait until all ptrs become obsolete
//

namespace TD_MoCap {
	class AbstractPtr
	{
	public:
		virtual void reset() = 0;
	};

	/// <summary>
	/// A special type of shared pointer which automatically empties when the DLL 
	///  which defines the class contained within the pointer becomes invalid
	/// </summary>
	template<typename T>
	class Ptr : public AbstractPtr
	{
	public:
		class Locked
		{
		public:
			T& operator->() const {
				return *this->ptr;
			}

			operator bool() const {
				return (bool)this->ptr;
			}


		protected:
			friend Ptr;

			Locked(Ptr* ptr, const std::shared_lock<std::shared_mutex>& ptrLock, const std::shared_lock<std::shared_mutex>& libraryLock)
				: ptr(ptr)
				, ptrLock(ptrLock)
			{ 
				this->libraryLock = LibraryRegister::X().get<T>()->lockLibraryUse();
			}

			Locked()
				: ptr(nullptr)
			{
				// empty ptr
			}

			Ptr * ptr;
			std::shared_lock<std::shared_mutex> ptrLock;
			std::shared_lock<std::shared_mutex> libraryLock;
		};

		Ptr(T * newInstance)
		{
			if (!newInstance) {
				// Presume that empty pointers don't need managing
				// In fact, we shouldn't really ever be here
				return;
			}

			auto lock = LibraryRegister::X().lockForRegisterWriting<T>();
			if (lock) {
				auto library = LibraryRegister::X().get<T>();
				library->registerInstance(this);
				this->instance = std::shared_ptr<T>(newInstance);
				this->registered = true;
			}
			else {
				// Library is closing - we remain invalid
				return;
			}
		}

		~Ptr()
		{
			if (this->registered) {
				auto lock = LibraryRegister::X().lockForRegisterWriting<T>();
				if (lock) {
					auto library = LibraryRegister::X().get<T>();
					library->unregisterInstance(this);
				}
				else {
					// Library is closing - we do nothing
					return;
				}
			}
		}

		Locked lock()
		{
			if (!this->instance) {
				// already invalid (e.g. already unloaded Library)
				return Locked();
			}

			auto libraryRegisterLock = std::shared_lock<std::shared_mutex>(LibraryRegister::X().collectionMutex);
			auto libraryUseLock = LibraryRegister::X().get<T>()->lockLibraryUse();
			if (libraryUseLock) {
				auto locked = Locked(this
					, std::shared_lock<std::shared_mutex>(this->mutex)
					, libraryUseLock);

				return std::move(locked);
			}
			else {
				// library is unloading. Cannot get a a lock on this pointer
				return Locked();
			}
		}

		void reset() override
		{
			// wait for any shared locks to expire
			std::unique_lock<std::shared_mutex> uniqueLock(this->mutex);
			this->instance.reset();
		}
		
	protected:
		std::shared_ptr<T> instance;
		std::shared_mutex mutex;

		// Remains false if we failed to register.
		// Note : This should always be equal to whether the Ptr is not empty
		bool registered = false;
	};

	template<class T, class ... Args>
	Ptr<T> make_ptr(Args&&... args) {
		return Ptr(new T(args...));
	}
}