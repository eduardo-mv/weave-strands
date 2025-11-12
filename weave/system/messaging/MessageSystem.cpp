#ifdef REMOVEME
#include "MessageSystem.h"
#include "system/engine/Service.h"
#include "system/engine/SettingsManager.h"

using namespace weave;

void * weave::MessageSystem::AllocatePackage(size_t byteSize, size_t byteAlign) {
	return ribbon.Allocate(byteSize, byteAlign);
}

/*****************************/

//Allocates and returns a chunk of memory that will be sent as package
void * weave::Message::AllocatePackage(size_t byteSize, size_t byteAlign) {
	package = Service<MessageSystem>()->AllocatePackage(byteSize, byteAlign);
	return package;
}

//Sends a message immediately. The call will execute the listener's callback and return when it's finished. This requires a global MessageSystem to be set.
void Message::Send() {
	Service<MessageSystem>()->SendMessage(*this);
}

//Adds a message to the message queue. The message will be sent on the next message flush. This requires a global MessageSystem to be set.This requires a global MessageSystem to be set.
void Message::Queue() {
	Service<MessageSystem>()->QueueMessage(*this);
}

//Adds a message to be sent at the specified time. The message will be sent when the delivery time is overdue. This requires a global MessageSystem to be set.
void Message::DelayQueue(double msgdelay, double interval, weave::ClockName clockName) {
	Service<MessageSystem>()->QueueMessage(*this, msgdelay, interval, clockName);
}

/*****************************/

MessageSystem::Handle::Handle(uint64_t id, MessageSystem *handler) : id(id), handler(handler) {
	if(handler && !handler->SetHandle(id, this)) {
		id = 0;
		this->handler = nullptr;
	}
}

//The move ctr is used to move the handle to another instance, invalidating the previous handle
MessageSystem::Handle::Handle(Handle &&other)  noexcept {
	//The complete handle is moved to this instance and the message handler is notified of the change
	this->handler = other.handler;
	this->id = other.id;

	other.handler = nullptr;
	other.id = 0;

	if(handler && !handler->SetHandle(id, this)) {
		id = 0;
		handler = nullptr;
	}
}

MessageSystem::Handle& MessageSystem::Handle::operator=(Handle &&other) noexcept {
	//The complete handle is moved to this instance and the message handler is notified of the change
	this->handler = other.handler;
	this->id = other.id;

	other.handler = nullptr;
	other.id = 0;

	if(handler && !handler->SetHandle(id, this)) {
		id = 0;
		handler = nullptr;
	}

	return *this;
}

//The destructor automatically removes the listener from the original message handler
MessageSystem::Handle::~Handle() {
	if(handler)
		handler->UnregisterListener(id);
	handler = nullptr;
}

//Removes the listener from the MessageSystem
void MessageSystem::Handle::UnregisterListener() {
	if(handler)
		handler->UnregisterListener(id);
	handler = nullptr;
}

//Returns true if the listener is enabled
bool MessageSystem::Handle::IsEnabled() {
	if(handler) {
		return handler->IsListenerEnabled(this->id);
	}
	else
		return false;
}

//Disables the Listener without unregistering it. The Listener can be enabled again.
void MessageSystem::Handle::Disable() {
	if(handler) {
		handler->EnableListener(this->id, false);
	}
}

//Enables the listener, if valid
void MessageSystem::Handle::Enable() {
	if(handler) {
		handler->EnableListener(this->id, true);
	}
}

//Registers a listener by event name using this instance to hold the returned listener ID. This requires a global MessageSystem to be set.
//Returns a reference to itself, which can be used to store on a listener group
MessageSystem::Handle& MessageSystem::Handle::RegisterListener(std::string const &eventName, ListenerCallback callback, uint64_t priority) {
	UnregisterListener();

	if(Service<MessageSystem>()) {
		(*this) = Service<MessageSystem>()->RegisterListener(eventName, callback, priority);
	}

	return *this;
}

MessageSystem::Handle& MessageSystem::Handle::RegisterListener(std::string const &eventName, uint64_t subId, ListenerCallback callback, uint64_t priority) {
	UnregisterListener();

	if(Service<MessageSystem>()) {
		*this = Service<MessageSystem>()->RegisterListener(eventName, subId, callback, priority);
	}

	return *this;
}

MessageSystem::Handle& MessageSystem::Handle::RegisterListener(std::string const &eventName, std::string const &subName, ListenerCallback callback, uint64_t priority) {
	UnregisterListener();

	if(Service<MessageSystem>()) {
		*this = Service<MessageSystem>()->RegisterListener(eventName, subName, callback, priority);
	}

	return *this;
}

//Registers a listener by event ID and stores de listener ID on this handle. This requires a global MessageSystem to be set.
//Returns a reference to itself, which can be used to store on a listener group
MessageSystem::Handle& MessageSystem::Handle::RegisterListener(uint64_t eventID, uint64_t subId, ListenerCallback callback, uint64_t priority) {
	UnregisterListener();

	if(Service<MessageSystem>()) {
		*this = Service<MessageSystem>()->RegisterListener(eventID, subId, callback, priority);
	}

	return *this;
}

//*****************************//

MessageSystem::HandleGroup::HandleGroup(HandleGroup &&other) noexcept {
	this->handles = std::move(other.handles);
}

MessageSystem::HandleGroup& MessageSystem::HandleGroup::operator=(HandleGroup &&other) noexcept {
	this->handles = std::move(other.handles);
	return *this;
}

void MessageSystem::HandleGroup::operator=(Handle &&handle) noexcept {
	handles.emplace_back(std::move(handle));
}

//Returns true if all handles in the group are valid
bool MessageSystem::HandleGroup::IsValid() {
	if(handles.size() == 0)
		return false;
	//Because handles stores in a group are enforced to come from the same handler and only one handle of a listener can be valid, we can guarantee that either all or none of the handles are valid
	return handles[0].IsValid();
}

//Removes all listeners from the InputEventHandler
void MessageSystem::HandleGroup::UnregisterListeners() {
	handles.clear();
}

//Returns true if the listener is enabled
bool MessageSystem::HandleGroup::IsEnabled() {
	if(handles.size()) {
		//Because a handle can only pertain to a group or to an individual handle at any given time, we can guarantee that either all or none of the handles in the group share the same state.
		return handles[0].IsEnabled();
	}
	else
		return false;
}

//Disables the Listener without unregistering it. The Listener can be enabled again.
void MessageSystem::HandleGroup::Disable() {
	for(auto &h : handles) {
		h.Disable();
	}
}

//Enables the listener, if valid
void MessageSystem::HandleGroup::Enable() {
	for(auto &h : handles) {
		h.Enable();
	}
}

//Registers a listener by event name using this instance to hold the returned listener ID. This requires a global MessageSystem to be set.
//Returns a reference to itself, which can be used to store on a listener group
void MessageSystem::HandleGroup::RegisterListener(std::string const &eventName, ListenerCallback callback, uint64_t priority) {
	if(Service<MessageSystem>()) {
		(*this) = Service<MessageSystem>()->RegisterListener(eventName, callback, priority);
	}
}

void MessageSystem::HandleGroup::RegisterListener(std::string const &eventName, std::string const &subName, ListenerCallback callback, uint64_t priority) {
	if(Service<MessageSystem>()) {
		*this = Service<MessageSystem>()->RegisterListener(eventName, subName, callback, priority);
	}
}

void MessageSystem::HandleGroup::RegisterListener(std::string const &eventName, uint64_t subId, ListenerCallback callback, uint64_t priority) {
	if(Service<MessageSystem>()) {
		*this = Service<MessageSystem>()->RegisterListener(eventName, subId, callback, priority);
	}
}

void MessageSystem::HandleGroup::RegisterListener(uint64_t eventID, std::string const &subName, ListenerCallback callback, uint64_t priority) {
	if(Service<MessageSystem>()) {
		*this = Service<MessageSystem>()->RegisterListener(eventID, subName, callback, priority);
	}
}

//Registers a listener by event ID and stores de listener ID on this handle. This requires a global MessageSystem to be set.
//Returns a reference to itself, which can be used to store on a listener group
void MessageSystem::HandleGroup::RegisterListener(uint64_t eventID, uint64_t subId, ListenerCallback callback, uint64_t priority) {
	if(Service<MessageSystem>()) {
		*this = Service<MessageSystem>()->RegisterListener(eventID, subId, callback, priority);
	}
}

//****************************************//


MessageSystem::MessageSystem() : 
	ribbon((Settings(*Service<SettingsManager>()))["MessageSystem"]["RibbonSize"].As<size_t>(WEAVE_MESSAGESYSTEM_RIBBONSIZE)),
	pool((Settings(*Service<SettingsManager>()))["MessageSystem"]["PoolSize"].As<size_t>(WEAVE_MESSAGESYSTEM_POOLSIZE) / sizeof(Message::DelayData), sizeof(Message::DelayData)),
	msgQueue((Settings(*Service<SettingsManager>()))["MessageSystem"]["MessageBufferSize"].As<size_t>(WEAVE_MESSAGESYSTEM_MESSAGEBUFFER) / sizeof(Message)),
	listenerIDCount(0)
{
	//Reserve a small pool for the delayed messages
	for(auto &queue : delayed_msgQueues)
		queue.msgQueue.reserve(20);
}

MessageSystem::~MessageSystem() {
	//Go through all the registered Listeners and invalidate the related Handles
	listenerInTray.clear();
	FlushListenerTrays();

	for(auto &entry : listenerMap) {
		if(entry.second && entry.second->handle) {
			entry.second->handle->handler = nullptr;
		}
	}
}

//Registers a listener by event name and returns the listener ID that can be used to unregister later
MessageSystem::Handle MessageSystem::RegisterListener(std::string const &eventName, ListenerCallback callback, uint64_t priority) {
	return RegisterListener(weave::HashString(eventName), 0, callback, priority);
}

MessageSystem::Handle MessageSystem::RegisterListener(std::string const &eventName, std::string const &subName, ListenerCallback callback, uint64_t priority) {
	return RegisterListener(weave::HashString(eventName), weave::HashString(subName), callback, priority);
}

MessageSystem::Handle MessageSystem::RegisterListener(std::string const &eventName, uint64_t subId, ListenerCallback callback, uint64_t priority) {
	return RegisterListener(weave::HashString(eventName), subId, callback, priority);
}

MessageSystem::Handle MessageSystem::RegisterListener(uint64_t eventID, std::string const &subName, ListenerCallback callback, uint64_t priority) {
	return RegisterListener(eventID, weave::HashString(subName), callback, priority);
}

//Registers a listener and returns the listener ID that can be used to unregister later
MessageSystem::Handle MessageSystem::RegisterListener(uint64_t eventID, uint64_t subId, ListenerCallback callback, uint64_t priority) {
	uint64_t lisID = 0;
	{
		std::lock_guard<std::mutex> lock(trayMutex);

		//Add the job to the tray
		lisID = listenerIDCount;
		++listenerIDCount;
		listenerInTray.emplace_back(lisID, eventID, subId, priority, callback);
	}
	//Return a new handle for this listener.
	return MessageSystem::Handle(lisID, this);
}

//Removes a listener from the registry
bool MessageSystem::UnregisterListener(uint64_t listenerID) {
	std::lock_guard<std::mutex> lock(trayMutex);

	//Add the removal job to the tray
	listenerOutTray.push_back(listenerID);

	return listenerMap.count(listenerID) > 0;
}

//Sends a message immediately. The call will execute the listener's callback and return when it's finished.
void MessageSystem::SendMessage(Message const &msg) {
	//Make sure all listeners are registered for this operation
	if(sending == 0)
		FlushListenerTrays();

	sending++;

	//Send the message directly
	ListenerQueue &bucket = listeners[msg.msgId];
	for(auto const &listener : bucket) {
		//Check for broadcast or private message conditions
		if(msg.subId == listener.subId || listener.subId == -1) {
			//Send the event to the callback assigned and break if the callback returns false, signaling that bubbling shoud be stoped
			if(listener.enabled) {
				if(!listener.callback(msg))
					break;
			}
		}
	}

	sending--;
}

//Adds a message to the message queue. The message will be sent on the next message flush.
void MessageSystem::QueueMessage(Message const &msg) {
	//static const uint64_t mask = (WEAVE_MESSAGESYSTEM_MESSAGEBUFFER / sizeof(Message)) - 1;
	//Increment the write pointer in the buffer. The actual pointer has to be wrapped around and is done using a mask instead of modulus.
	//This only works if the size of the buffer is 2^n
	//uint64_t wpoint = msgInPoint++;
	//msgQueue[wpoint & mask] = msg;
	msgQueue[msgQueue.Next()] = msg;
}


//Adds a message to be sent at the specified time. The message will be sent when the delivery time is overdue.
void MessageSystem::QueueMessage(Message const &msg, double delay, double interval, weave::ClockName clockName) {
	double deliveryTime = Service<ClockSystem>()->GetClock(clockName).time + delay; //The target delivery time
	auto &queue = delayed_msgQueues[uint32_t(clockName)]; //The individual queue for this clock

	std::lock_guard<std::mutex> lock(queue.delayMutex);
	bool found = false;

	for(auto it = queue.msgQueue.begin(); it != queue.msgQueue.end(); ++it) {
		if(it->delay && it->delay->deliveryTimeStamp < deliveryTime) {
			//Keep a record that the node was found and where
			uint64_t location = std::distance(queue.msgQueue.begin(), it);
			//Insert the new message before the node we just found
			queue.msgQueue.emplace(it, msg);
			//Modify the desired values
			queue.msgQueue[location].delay = pool.AllocateNew<Message::DelayData>(deliveryTime, interval, clockName); 
			//Finish
			found = true;
			break;
		}
	}

	if(!found) {
		//Create the memory block needed for the delay data and set the values
		queue.msgQueue.emplace_back(msg);
		queue.msgQueue.back().delay = pool.AllocateNew<Message::DelayData>(deliveryTime, interval, clockName);
	}

	delayedMessagesFlag = true;
}

//Flushes the whole message queue by locking the queue and sending the events to the associated listeners. Messages with a time stamp will be sent if overdue, otherwise
//they will remain in the queue. The globalTime parameter is required to correctly handle the time requirements of delayed messages.
void MessageSystem::FlushMessageQueue() {
	//Go through the listener in and out trays to register any pending listeners
	FlushListenerTrays();
	
	//Is there any reason to flush messages?
	uint64_t pendingTotal = 0;
	if(delayedMessagesFlag) {
		for(auto &delayQ : delayed_msgQueues)
			pendingTotal += delayQ.msgQueue.size();
	}
	
	if(flushing || (msgQueue.Pending() + pendingTotal <= 0))
		return;

	//Lock the flush
	flushing = true;

	//Go through all the delayed messages and send the overdue ones
	if(delayedMessagesFlag) {
		delayedMessagesFlag = false;
		size_t totalLeft = 0;
		int32_t clockID = 0;
		for(auto &delayQ : delayed_msgQueues) {
			double globalTime = Service<ClockSystem>()->GetClock(static_cast<weave::ClockName>(clockID)).time;
			//Lock the delayed messages before processing them
			delayQ.delayMutex.lock();
			//Copy and pop all delayed messages that are overdue. Because this queue is ordered from the higher to lower, we start from behind
			while(delayQ.msgQueue.size() && delayQ.msgQueue.back().delay->deliveryTimeStamp <= globalTime) {
				//Copy the message to be sent with the rest. The message is not sent here to reduce the amount of time the delayed queue is locked
				msgQueue[msgQueue.Next()] = delayQ.msgQueue.back();
				//Remove the message from the queue
				delayQ.msgQueue.pop_back();
			}

			//Keep count of the amount of messages left
			totalLeft += delayQ.msgQueue.size();

			//Unlock the delayed queue
			delayQ.delayMutex.unlock();

			++clockID;
		}

		//If we didn't remove all delayed messages, flag it up again for next iteration
		if(totalLeft > 0)
			delayedMessagesFlag = true;
	}

	//Go through all the messages in the output queue and send them. The queue is marked by the last point processed and the new ending point.
	for(uint64_t i = msgQueue.Begin(), end = msgQueue.End(); i != end; ++i) {
		//Get the message reference and the listeners for this msg id
		Message &msg = msgQueue[i];
		ListenerQueue &bucket = listeners[msg.msgId];

		for(auto const &listener : bucket) {
			//Check for broadcast or private message conditions
			if(msg.subId == 0 || msg.subId == listener.subId) {
				//Send the event to the callback assigned and break if the callback returns false, signaling that bubbling shoud be stoped
				if(listener.enabled) {
					if(!listener.callback(msg)) {
						break;
					}
				}
			}
		}

		//Automatically add the message to the queue again if there's a defined interval
		if(msg.delay) {
			if(msg.delay->interval > 0.0f) {
				this->QueueMessage(msg, msg.delay->interval, msg.delay->interval, msg.delay->clockName);
			}
			//Free the memory block
			pool.Delete<Message::DelayData>(msg.delay);
		}
	}

	//Unlock the flush
	flushing = false;
}

//Empties the listener trays registering and removing any pending listeners
void MessageSystem::FlushListenerTrays() {
	if(listenerInTray.size() == 0 && listenerOutTray.size() == 0)
		return;

	//Lock out the tray
	std::lock_guard<std::mutex> lock(trayMutex);

	//Outgoing
	for(uint64_t job : listenerOutTray) {
		//Remove the listener from the map and/or the in tray (it can happen that a listener registers and unregisters before it can be processed
		auto const &it = listenerMap.find(job);
		if(it != listenerMap.end() && it->second) {
			Listener *listener = it->second;
			auto &bucket = listeners[listener->msgId];
			//Remove the listener from the bucket keeping the correct priority order
			bucket.erase(std::remove_if(bucket.begin(), bucket.end(), [&listener](Listener const &item) -> bool {	return &item == listener; }), bucket.end());
			if(bucket.size() == 0) {
				listeners.erase(listener->msgId);
			}
			else {
				//Because modifying the contents of the bucket can cause memory reallocation, the listenerMap has to be fixed for all listeners in this bucket
				for(auto &lis : bucket) {
					listenerMap[lis.id] = &lis;
				}
			}
			//Remove the reference for this listener ID
			//listenerMap[job] = nullptr;
			listenerMap.erase(job);
		}

		for(uint64_t i = 0; i < listenerInTray.size(); ++i) {
			if(listenerInTray[i].id == job) {
				//Listener found waiting for registration, we can safely remove it from the queue
				listenerInTray[i] = listenerInTray.back();
				listenerInTray.pop_back();
				--i;
			}
		}
	}

	//Incoming
	for(auto &job : listenerInTray) {
		//Add the listener
		auto &bucket = listeners[job.msgId];
		bucket.emplace_back(job);
		//Reorder the bucket so that it's set on priority order
		std::stable_sort(bucket.begin(), bucket.end(), [](Listener const &a, Listener const &b) -> bool { return a.priority > b.priority; });
		//Because modifying the contents of the bucket can cause memory reallocation, the listenerMap has to be fixed for all listeners in this bucket
		for(auto &lis : bucket) {
			listenerMap[lis.id] = &lis;
		}
	}

	listenerInTray.clear();

	listenerOutTray.clear();
}

//Chages the external handle for a given Listener
bool MessageSystem::SetHandle(uint64_t id, Handle *handle) {
	//Lock out the tray
	{
		std::lock_guard<std::mutex> lock(trayMutex);

		//Look for the listener in the input tray (the most likely case)
		for(auto &job : listenerInTray) {
			if(job.id == id) {
				job.handle = handle;
				return true;
			}
		}
	}

	//Look for the listener in the registry
	Listener *listener = GetListener(id);
	if(listener) {
		listener->handle = handle;
		return true;
	}

	return false;

}

//Returns a Listener given its id. 
MessageSystem::Listener* MessageSystem::GetListener(uint64_t id) {
	//Find the listener on the map
	auto const &it = listenerMap.find(id);
	if(it != listenerMap.end()) {
		return it->second;
	}
	//Look for it on the listener tray queue
	else {
		std::lock_guard<std::mutex> lock(trayMutex);

		for(auto &job : listenerInTray) {
			if(job.id == id) {
				return &job;
			}
		}
	}

	return nullptr;
}

//Enables a listener
void MessageSystem::EnableListener(uint64_t id, bool flag) {
	auto *lis = GetListener(id);
	if(lis)
		lis->enabled = flag;
}

//Returns true if a listener is enabled
bool MessageSystem::IsListenerEnabled(uint64_t id) {
	auto *lis = GetListener(id);
	if(lis)
		return lis->enabled;
	else
		return false;
}

#endif