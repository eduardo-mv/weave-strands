/*
* Input Processor:
Input Event Context

InputEventContext is used to generate Message events from complex key combination rules.

An XML parser is provided to generate the event conditions.
An example of an XML representation:

<inputcontext rid="InputEventContext" bubble="false">
	<action name="Debug_Switch">
		<input  key="esc" device="keyboard0" state="down" prevstate="up"/>
	</action>

	<action name="Debug_MouseMove">
		<input key="cursor" device="mouse0" trigger="true"/>
		<input key="x2click" device="mouse0" state="down, hold" prevstate="up, hold"/>
	</action>

	<action name="CtrlR">
		<input  key="r" device="keyboard0" state="down, hold" prevstate="up, hold"/>
		<input  key="ctrl" device="keyboard0" state="down, hold" prevstate="up,hold" modifier="true"/>
	</action>
</inputcontext>

*/
#pragma once

#include "weave/input/system/InputProcessor.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <any>

namespace weave::input {
	class InputEventContext;
	struct InputEventContextRule;
//Implements a full condition of states that must match the current state of a key
struct InputEventContextCondition {
	struct StateFlags {
		bool up : 1;
		bool down : 1;
		bool hold : 1;
		bool doubleDown : 1;
		bool any : 1;
	};
	
	VirtualDevice device = VirtualDevice::None;
	VirtualKey key = VirtualKey::None;

	StateFlags states{};
	StateFlags prevStates{};

	//Trigger flag indicating that the if this condition is met, all other conditions need to be checked
	//That is, meeting this condition can cause the triggering of an event if all other conditions are met too
	//Setting this to false is useful when a condition is meant to be a modifier 
	//(e.g. Ctrl+F, where Ctrl is not a trigger, won't cause an event on F+Ctrl)
	bool isTrigger = true;
private:
	friend class InputEventContext;
	friend struct InputEventContextRule;
	bool Check(InputStateMap const& inputState) const;
};

enum InputEventForwardMode {
	Forward,
	Sink
};

//Implements a list of Conditions that when met will generate the specified event output
struct InputEventContextRule {
	InputEventForwardMode forwardMode{ Forward } ;
	std::vector<InputEventContextCondition> conditions;
	std::vector<std::pair<VirtualDevice, VirtualKey>> feedbackRequests;
	int32_t priority{};
	uint64_t eventId{};

private:
	friend class InputEventContext;
	bool Check(InputStateMap const& inputState) const;
};


class InputEventContext : public InputProcessor {
private:
	using DeviceAndKey = std::pair<VirtualDevice, VirtualKey>;
	struct PairHash {
		std::size_t operator()(DeviceAndKey value) const {
			uint64_t i64value =
				static_cast<uint64_t>(value.first) << 32
				| static_cast<uint64_t>(value.second);
			return size_t(i64value);
		}

		constexpr bool operator()(const DeviceAndKey& lhs, const DeviceAndKey& rhs) const {
			return lhs.first == rhs.first
				&& lhs.second == rhs.second;
		}
	};

	std::vector<std::unique_ptr<InputEventContextRule>> rules;
	std::unordered_map<DeviceAndKey, std::vector<InputEventContextRule*>, PairHash, PairHash> triggeredRules;

	InputEventForwardMode forwardMode{ Forward };

public:
	void SetForwardMode(InputEventForwardMode mode) { forwardMode = mode; }
	void AddRule(InputEventContextRule rule);

private:
	void InputEvent(InputEventData inputData, InputStateMap& inputState) override;
	void SendEvent(InputEventContextRule const &rule, InputStateMap& inputState) const;
};

/*
TODO: XML parser herlper
//Creates a new input rule based on the supplied XML node
void CreateXMLInputRule(pugi::xml_node root);
//Loads a text based file
virtual Resource::LoadStates LoadXML(pugi::xml_node root) override;
//Parses an XML based input condition and returns a new object with it
InputEventContextCondition ParseXMLInputCondition(pugi::xml_node root);
//Parses an XML node with feedback rules and fills the supplied rule object with the result
void ParseXMLFeedbackNode(pugi::xml_node root, InputEventContextRule &rule);

*/

}
