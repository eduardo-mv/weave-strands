/*
Title: "Weave Message System"
File: MessageSystem.h
Author(s): Eduardo Martínez Vidal

Abstract:
Implements a system to send messages between disconnected modules via a Listener pattern. 
Objects can register their listeners to receive broadcasted or personal messages including additional packet data allocated from a local memory pool.

Update Log:
14 Jan 2017:
File creation

*/
#pragma once
#ifdef REMOVEME

#include "system/hash/Hash.h"

#include "system/memory/RibbonAllocatorMT.h"
#include "system/memory/PoolAllocator.h"
#include "system/memory/RoundBufferAtomic.h"

#include "system/timing/ClockSystem.h"
#include <limits>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <mutex>
#include <atomic>

namespace weave {

class MessageSystem;

class Message {
	friend class MessageSystem;
public:
	static const constexpr uint64_t BroadcastSubId = ~0ull;

protected:
	uint64_t msgId = 0; //The ID of the message. A name that represents the "event" causing the message.
	uint64_t subId = 0; //The secondary ID used to create a specific filter for a message. 0 is used as broadcast.
	
	void *package = nullptr; //Internally allocated memory block with additional user-defined information 
	
	//Delay data is added as an optional block allocated from a memory pool. This reduces the footprint of non delayed Messages, which are the  majority.
	//The packet is created by the MessageSystem when sending a delayed Message and destroyed by Message when the "destroy" flag is set.
	struct DelayData {
		double deliveryTimeStamp; //The time the message is expected to be delivered.
		double interval; //Interval at which this message is automatically repeated. 0.0f or negative indicated no interval. In seconds.
		weave::ClockName clockName; //Clock name used for the delayed delivery and interval functionality
	
		DelayData(double deliveryTimeStamp, double interval, weave::ClockName clockName) :
			deliveryTimeStamp(deliveryTimeStamp), interval(interval), clockName(clockName) { }

	} *delay = nullptr;

public:
	Message() = default;
	Message(uint64_t msgId) : msgId(msgId) {}
	Message(uint64_t msgId, uint64_t subId) : msgId(msgId), subId(subId) {}
	Message(std::string const &msgId) { this->msgId = weave::HashString(msgId); }
	Message(std::string const &msgId, std::string const &subId) { 
		SetId(msgId);
		SetSubId(subId);
	}
	Message(std::string const &msgId, uint64_t subId) {
		SetId(msgId);
		this->subId = subId;
	}
	
	Message(Message const &msg) = default;
	Message(Message &&msg) noexcept : msgId(msg.msgId), subId(msg.subId), package(msg.package), delay(msg.delay) {
		msg.msgId = 0;
		msg.subId = 0;
		msg.package = nullptr;
		msg.delay = nullptr;
	}

	//With added buffer package
	Message(uint64_t msgId, void *package) : msgId(msgId), package(package) {}
	Message(uint64_t msgId, uint64_t subId, void *package) : msgId(msgId), subId(subId), package(package) {}
	explicit Message(std::string const &msgId, void *package) : package(package) { SetId(msgId); }
	Message(std::string const &msgId, std::string const &subId, void *package) : package(package) {
		SetId(msgId);
		SetSubId(subId);
	}
	Message(std::string const &msgId, uint64_t subId, void *package) : subId(subId), package(package) {
		SetId(msgId);
	}

	//With self allocating package (copy)
	template<typename Package>
	Message(uint64_t msgId, Package&& package) : msgId(msgId), package(AllocatePackage(std::forward(package))) {}
	template<typename Package>
	Message(uint64_t msgId, uint64_t subId, Package&& package) : msgId(msgId), subId(subId), package(AllocatePackage(std::forward(package))) {}
	template<typename Package>
	explicit Message(std::string const& msgId, Package&& package) : package(AllocatePackage(std::forward(package))) { SetId(msgId); }
	template<typename Package>
	Message(std::string const& msgId, std::string const& subId, Package&& package) : package(AllocatePackage(std::forward(package))) {
		SetId(msgId);
		SetSubId(subId);
	}
	template<typename Package>
	Message(std::string const& msgId, uint64_t subId, Package&& package) : subId(subId), package(AllocatePackage(std::forward(package))) {
		SetId(msgId);
	}

	//Information about the message
	//Returns the delivery time that was set for this message
	double DeliveryTimeStamp() const { return delay ? delay->deliveryTimeStamp : 0.0f; }
	//Returns the delivery interval set for this message
	double DeliveryInterval() const { return delay ? delay->interval : 0.0f; }

	//Sets the message ID
	void SetId(uint64_t id) { msgId = id; }
	void SetId(uint64_t id, uint64_t sid) { msgId = id; subId = sid; }
	void SetId(std::string const &id) { msgId = ( id.size() ? weave::HashString(id) : 0); }
	void SetId(std::string const &id, std::string const &sid) { 
		SetId(id);
		SetSubId(sid);
	}

	//Sets the message sub ID
	void SetSubId(std::string const &sid) { subId = (sid.size() ? weave::HashString(sid) : 0); }
	void SetSubId(uint64_t sid) { subId = sid; }

	//Returns the message ID
	uint64_t GetId() const { return msgId; }
	//Returns the message sub ID
	uint64_t GetSubId() const { return subId; }

	//Allocates and returns a chunk of memory that will be sent as package
	void* AllocatePackage(size_t byteSize, size_t byteAlign = 1);
	template<typename T>
	T* AllocatePackage(size_t byteSize, size_t byteAlign = 1) {
		return reinterpret_cast<T*>(AllocatePackage(byteSize, byteAlign));
	}

	template<typename T, size_t byteAlign = 1, typename ...Args>
	T* AllocatePackage(Args const &...args) {
		void *mem = AllocatePackage(sizeof(T), byteAlign);
		return new (mem) T(args...);
	}
	
	//Returns the package pointer
	template<typename T = void>
	T* GetPackage() const { return reinterpret_cast<T*>(package); }

	//Manually sets the package pointer
	void SetPackage(void *buffer) {
		package = buffer;
	}

	//Sends a message immediately. The call will execute the listener's callback and return when it's finished. This requires a global MessageHandler to be set.
	void Send();
	//Adds a message to the message queue. The message will be sent on the next message flush. This requires a global MessageHandler to be set.This requires a global MessageHandler to be set.
	void Queue();
	//Adds a message to be sent at the specified time. The message will be sent when the delivery time is overdue. This requires a global MessageHandler to be set.
	void DelayQueue(double delay, double interval = 0.0f, weave::ClockName clockName = weave::ClockName::Messaging);

	//Returns the hashed name of a property name
	static uint64_t HashMessageName(std::string const &name) { return weave::HashString(name); }

	Message& operator=(Message const &) = default;
	Message& operator=(Message &&other) noexcept {
		msgId = other.msgId;
		subId = other.subId;
		package = other.package;
		delay = other.delay;

		other.msgId = 0;
		other.subId = 0;
		other.package = nullptr;
		other.delay = nullptr;

		return *this;
	}
};

class MessageSystem {
public:
	//The listener callback function type
	typedef std::function<bool(Message const &)> ListenerCallback;

	//Sub class returned every time a listener for a message is registered. When the returned object falls out of scope the listener will automatically be unregistered from the MessageHandler
	//Handles can be moved but not copied. When the MessageHandler goes out of scope, all related Handles are invalidated automatically.
	class Handle {
		friend class MessageSystem;
	private:
		uint64_t id; //The listener identifier
		MessageSystem *handler; //The associated handler

		Handle(uint64_t id, MessageSystem *handler);
	public:
		Handle() : id(0), handler(nullptr) { }
		Handle(Handle const &) = delete;
		//The move ctr is used to move the handle to another instance, invalidating the previous handle
		Handle(Handle &&) noexcept;
		//The destructor automatically removes the listener from the original message handler
		~Handle();
		//Returns true if the handle is valid
		bool IsValid() { return handler != nullptr; }
		//Returns true if the listener is enabled
		bool IsEnabled();
		//Removes the listener from the MessageHandler
		void UnregisterListener();
		//Disables the Listener without unregistering it. The Listener can be enabled again.
		void Disable();
		//Enables the listener, if valid
		void Enable();

		//Registers a listener by event name using this instance to hold the returned listener ID. This requires a global MessageHandler to be set.
		//Returns a reference to itself, which can be used to store on a listener group
		Handle& RegisterListener(std::string const &eventName, ListenerCallback callback, uint64_t priority = 0);
		Handle& RegisterListener(std::string const &eventName, std::string const &subName, ListenerCallback callback, uint64_t priority = 0);
		Handle& RegisterListener(std::string const &eventName, uint64_t subId, ListenerCallback callback, uint64_t priority = 0);
		//Registers a listener by event ID and stores de listener ID on this handle. This requires a global MessageHandler to be set.
		//Returns a reference to itself, which can be used to store on a listener group
		Handle& RegisterListener(uint64_t eventID, uint64_t subId, ListenerCallback callback, uint64_t priority = 0);

		Handle& operator=(Handle const &other) = delete;
		Handle& operator=(Handle &&other) noexcept;
	};

	//Helper subclass used to contain multiple message handlers. This can be used to minimize the amount of loose variables needed to use multiple message listeners, when the life span of the
	//listeners is expected to be the same. A group cannot be copied but it can be moved.
	class HandleGroup {
	private:
		std::vector<Handle> handles;
	public:
		HandleGroup() = default;
		HandleGroup(HandleGroup const &) = delete;
		HandleGroup(HandleGroup &&) noexcept;
		~HandleGroup() = default;

		HandleGroup& operator=(HandleGroup const &);
		HandleGroup& operator=(HandleGroup &&) noexcept;
		void operator=(Handle &&) noexcept;

		size_t ListenerCount() const { return handles.size(); }

		//Returns true if all handles in the group are valid
		bool IsValid();
		//Returns true if the listeners are enabled
		bool IsEnabled();
		//Disables the Listeners without unregistering them. The Listeners can be enabled again.
		void Disable();
		//Enables the listeners, if valid
		void Enable();

		//Removes all listeners from the MessageHandler
		void UnregisterListeners();

		//Registers a listener by event name using this instance to hold the returned listener ID. This requires a global MessageHandler to be set.
		void RegisterListener(std::string const &eventName, ListenerCallback callback, uint64_t priority = 0);
		void RegisterListener(std::string const &eventName, std::string const &subName, ListenerCallback callback, uint64_t priority = 0);
		void RegisterListener(std::string const &eventName, uint64_t subId, ListenerCallback callback, uint64_t priority = 0);
		void RegisterListener(uint64_t eventID, std::string const &subName, ListenerCallback callback, uint64_t priority = 0);
		//Registers a listener by event ID and stores the listener ID on this handle. This requires a global MessageHandler to be set.
		void RegisterListener(uint64_t eventID, uint64_t subId, ListenerCallback callback, uint64_t priority = 0);
	};

protected:
	//The hash function used to generate the IDs from the message names is Message::hasher

	//Ribbon allocator used to store Message data. The ribbon is never wiped and the lifetime of the objects depends on how much the ribbon is used.
	//Message objects are expected to drop their packets once the callback returns.
	//This is a thread safe allocator.
	RibbonAllocatorMT ribbon;
	//The pool allocator is used to generate packets with the delayed timing information for Delayed Messages, so that the non delayed Messages have a smaller package size
	PoolAllocator pool;

	//A single buffer scaled on initialization is used as a ribbon to store messages. The messages are copied into the next free slot and wrapped around.
	//This imposes a limit on how many messages can be sent in between iterations of the flush as the messages will start overwritting.
	//An atomic integer is used to keep track of the next free slot on the buffer and a marker on the last processed slot is kept.
	RoundBufferAtomic<Message> msgQueue;

	//For the delayed messages multiple queues are kept, one per supported clock, so that each can be ordered accordingly and locked individually
	struct DelayedQueue {
		std::mutex delayMutex;
		std::vector<Message> msgQueue;
	};
	std::array<DelayedQueue, uint32_t(weave::ClockName::Last)+1> delayed_msgQueues;
	std::atomic_bool delayedMessagesFlag {false}; //Used to control access to the delayed_msgQueues so that they aren't checked when there's no delayed Messages

	//A mutex used to control access to the tray
	std::mutex trayMutex;

	//Flag used to ensure only one flush operation is done at the same time
	bool flushing = false;

	//Flag used to ensure a send operation doesn't try to flush the listener tray twice in nested SendMessage operations
	std::atomic<int> sending {0};

	//A type representing a listener with associated priority and ID
	struct Listener {
		uint64_t id; //The unique ID given to this listener entry
		uint64_t msgId; //The hashed message name
		uint64_t subId; //The secondary message name
		uint64_t priority; //The priority value of this listener
		
		Handle *handle; //The associated handle
		ListenerCallback callback; //The associated function object
		bool enabled;
		
		Listener(uint64_t id, uint64_t msgId, uint64_t subId, uint64_t priority, ListenerCallback cbk) : id(id), msgId(msgId), subId(subId), priority(priority), handle(nullptr), callback(cbk), enabled(true) {}
	};

	//A map associating event IDs to functions to be called
	typedef std::vector<Listener> ListenerQueue;
	std::unordered_map<uint64_t, ListenerQueue, NoHash> listeners;
	
	//Input and output listener tray where all new listeners go to before actually being registered. This is used because during a callback message the message may wish to remove or ad new listeners
	ListenerQueue listenerInTray;
	std::vector<uint64_t> listenerOutTray;

	//The following keeps a registry of all Listeners in a specific table so that they can be located by their individual ID
	uint64_t listenerIDCount;
	std::unordered_map<uint64_t, Listener*, NoHash> listenerMap;

public:

	MessageSystem();
	~MessageSystem();

	//Allocates and returns a chunk of memory from the local stack. This memory will persist until the next message flush
	void* AllocatePackage(size_t byteSize, size_t byteAlign = 1);
	template<typename T, size_t byteAlign = 1>
	T* AllocatePackage() {
		return reinterpret_cast<T*>(AllocatePackage(sizeof(T), byteAlign));
	}

	//Registers a listener by event name and returns the listener ID that can be used to unregister later
	Handle RegisterListener(std::string const &eventName, ListenerCallback callback, uint64_t priority = 0);
	Handle RegisterListener(std::string const &eventName, std::string const &subName, ListenerCallback callback, uint64_t priority = 0);
	Handle RegisterListener(std::string const &eventName, uint64_t subId, ListenerCallback callback, uint64_t priority = 0);
	Handle RegisterListener(uint64_t eventID, std::string const &subName, ListenerCallback callback, uint64_t priority = 0);

	//Registers a listener by event ID and returns the listener ID that can be used to unregister later
	Handle RegisterListener(uint64_t eventID, uint64_t subId, ListenerCallback callback, uint64_t priority = 0);
	
	//Sends a message immediately. The call will execute the listener's callback and return when it's finished.
	void SendMessage(Message const &msg);
	//Adds a message to the message queue. The message will be sent on the next message flush.
	void QueueMessage(Message const &msg);
	//Adds a message to be sent at the specified time. The message will be sent when the delivery time is overdue.
	void QueueMessage(Message const &msg, double delay, double interval, weave::ClockName clockName);

	//Flushes the whole message queue by locking the queue and sending the events to the associated listeners. Messages with a time stamp will be sent if overdue, otherwise
	//they will remain in the queue. The globalTime parameter is required to correctly handle the time requirements of delayed messages.
	void FlushMessageQueue();

private:
	//Empties the listener trays registering and removing any pending listeners
	void FlushListenerTrays();

	//Chages the external handle for a given Listener. This is used internally by the Handles to notify a handle move
	bool SetHandle(uint64_t id, Handle *handle);
	//Removes a listener from the registry. To unregister a listener from outside the handle the Handle must be used. This is an internal usage method.
	bool UnregisterListener(uint64_t listenerID);
	//Returns a Listener given its id. 
	Listener* GetListener(uint64_t id);
	//Enables a listener
	void EnableListener(uint64_t id, bool flag);
	//Returns true if a listener is enabled
	bool IsListenerEnabled(uint64_t id);
};

}

#endif