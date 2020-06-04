#pragma once

#include "Exception.h"

#include <shared_mutex>
#include <mutex>
#include <set>
#include <typeindex>
#include <memory>

namespace TD_MoCap {
	class AbstractPtr;

	/// <summary>
	/// The Library class is used to track the lifetime of a library so that objects defined in this library become safely invalidated before the library unloads.
	/// </summary>
	class TDMOCAP_API Library
	{
	private:
		Library(const std::type_index&);
	public:
		virtual ~Library();

		template<typename T>
		static std::unique_ptr<Library> make()
		{
			return std::unique_ptr<Library>(new Library(std::type_index(typeid(T))));
		}

		std::unique_ptr<std::unique_lock<std::mutex>> lockForRegisterWriting();
		std::unique_ptr<std::shared_lock<std::shared_mutex>> lockLibraryUse();

		void registerInstance(AbstractPtr*);
		void unregisterInstance(AbstractPtr*);
	protected:
		bool closing = false;

		std::mutex lockRegisteredInstances;

		// shared_locks for Ptr::Locked's
		mutable std::shared_mutex lockLibrary;

		std::set<AbstractPtr*> registeredInstances;
		std::type_index typeIndex;
	};

	class LibraryRegister
	{
	public:
		static LibraryRegister& X();


		Library*
			get(const std::type_index&);
		template<typename T>
		Library* 
			get()
		{
			return this->get(std::type_index(typeid(T)));
		}


		std::unique_ptr<std::unique_lock<std::mutex>>
			lockForRegisterWriting(const std::type_index&);
		template<typename T>
		std::unique_ptr<std::unique_lock<std::mutex>> 
			lockForRegisterWriting()
		{
			return this->lockForRegisterWriting(std::type_index(typeid(T)));
		}

		mutable std::shared_mutex collectionMutex;

	protected:

		friend Library;

		void add(const std::type_index &, Library*);
		void remove(const std::type_index&);

		std::map<std::type_index, Library*> libraries;
	};
}