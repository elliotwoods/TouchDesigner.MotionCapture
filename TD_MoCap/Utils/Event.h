#pragma once

// Adapted from Event

#include <map>
#include <vector>
#include <functional>

namespace TD_MoCap {
	namespace Utils {
		template<class ArgType>
		class Event {
		public:
			//this used to be
			/*
			 typedef public std::function<void(ArgType&)> Functor;
			 typedef public std::function<void()> VoidFunctor;
			 not sure why... but anyway that doesn't work on Xcode 6
			 */
			typedef std::function<void(ArgType&)> Functor;
			typedef std::function<void()> VoidFunctor;
			typedef int32_t IndexType; // use negative index for bottom of stack
			struct Index {
				Index(IndexType order, void* owner) {
					this->order = order;
					this->owner = owner;
				}
				bool operator<(const Index& other) const {
					return this->order < other.order;
				}
				bool operator==(const IndexType& otherIndex) const {
					return this->order == otherIndex;
				}
				IndexType order;
				void* owner;
			};
			typedef std::map<Index, Functor> FunctorMap;
			typedef std::pair<Index, Functor> Pair;

			void operator+=(Functor functor) {
				this->addListener(functor, 0);
			}

			void addListener(Functor functor, void* owner) {
				IndexType order = 0;
				if (!this->listeners.empty()) {
					//loop until we find a free index
					while (listeners.find(Index(order, 0)) != listeners.end()) {
						order++;
					}
				}
				this->listeners.insert(Pair(Index(order, owner), functor));
			}

			void addListener(Functor functor, void* owner, IndexType order) {
				//loop until we find a free index
				while (listeners.find(Index(order, 0)) != listeners.end()) {
					order++;
				}
				this->listeners.insert(Pair(Index(order, owner), functor));
			}

			void removeListeners(void* owner) {
				std::vector<IndexType> toRemove;
				for (auto iterator : this->listeners) {
					if (iterator.first.owner == owner) {
						toRemove.push_back(iterator.first.order);
					}
				}
				for (auto order : toRemove) {
					this->listeners.erase(Index(order, owner));
				}
			}

			void notifyListeners(ArgType& arguments) {
				for (auto listener : this->listeners) {
					listener.second(arguments);
				}
			}

			/// Warning : You will not be able to call this if ArgType does not have a default public constructor
			void notifyListeners() {
				ArgType dummyArguments;
				for (auto listener : this->listeners) {
					listener.second(dummyArguments);
				}
			}

			/// Useful for mouse action stacks where last is top (first)
			void notifyListenersInReverse(ArgType& arguments) {
				auto it = this->listeners.rbegin();
				for (; it != this->listeners.rend(); it++) {
					it->second(arguments);
				}
			}

			void operator()(ArgType& arguments) {
				this->notifyListeners(arguments);
			}

			bool empty() const {
				return this->listeners.empty();
			}

			size_t size() const {
				return this->listeners.size();
			}

			const FunctorMap& getListeners() const {
				return this->listeners;
			}

			void clear() {
				this->listeners.clear();
			}

		protected:
			FunctorMap listeners;
		};


		//--
		//Specialization for Event<void>
		//--
		//
		//unhappily, we have to write the code out again 1:1 with the different type
		//since void is incompatible with some of the function templates above (e.g. use of ArgType&)
		//
		//
		/// NOTE : you can't call onMyEvent(), you must use onMyEvent.notifyListeners();
		template<>
		class Event<void> {
			typedef std::function<void()> Functor;
			typedef Event<int>::IndexType IndexType;
			typedef Event<int>::Index Index;
			typedef std::map<Index, Functor> FunctorMap;
			typedef std::pair<Index, Functor> Pair;
		public:
			void operator+=(Functor functor) {
				this->addListener(functor, 0);
			}

			void addListener(Functor functor, void* owner) {
				IndexType order = 0;
				if (!this->listeners.empty()) {
					//loop until we find a free index
					while (listeners.find(Index(order, 0)) != listeners.end()) {
						order++;
					}
				}
				this->listeners.insert(Pair(Index(order, owner), functor));
			}

			void addListener(Functor functor, IndexType order, void* owner) {
				//loop until we find a free index
				while (listeners.find(Index(order, 0)) != listeners.end()) {
					order++;
				}
				this->listeners.insert(Pair(Index(order, owner), functor));
			}

			void removeListeners(void* owner) {
				std::vector<IndexType> toRemove;
				for (auto iterator : this->listeners) {
					if (iterator.first.owner == owner) {
						toRemove.push_back(iterator.first.order);
					}
				}
				for (auto order : toRemove) {
					this->listeners.erase(Index(order, owner));
				}
			}

			void notifyListeners() {
				for (auto listener : this->listeners) {
					listener.second();
				}
			}

			/// Useful for mouse action stacks where last is top (first)
			void notifyListenersInReverse() {
				auto it = this->listeners.rbegin();
				for (; it != this->listeners.rend(); it++) {
					it->second();
				}
			}

			bool empty() const {
				return this->listeners.empty();
			}


			size_t size() const {
				return this->listeners.size();
			}

			const FunctorMap& getListeners() const {
				return this->listeners;
			}

			void clear() {
				this->listeners.clear();
			}
		protected:
			FunctorMap listeners;
		};
		//
		//--

	}
}