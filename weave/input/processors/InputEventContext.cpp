#include "InputEventContext.h"

using namespace weave::input;

//Checks the condition against key data
bool InputEventContextCondition::Check(InputStateMap const& inputState) const {
	if (states.any && prevStates.any)
		return true;

	auto data = inputState.QueryKeyData(device, key).state.value_or(KeyStatePayload{});

	if(!states.any){
		if (
			!(states.up && data.current == VirtualKeyState::Up) &&
			!(states.down && data.current == VirtualKeyState::Down) &&
			!(states.hold && data.current == VirtualKeyState::Hold) &&
			!(states.doubleDown && data.current == VirtualKeyState::Double)) {
			return false;
		}
	}

	if (!prevStates.any) {
		if (
			!(prevStates.up && data.previous == VirtualKeyState::Up) &&
			!(prevStates.down && data.previous == VirtualKeyState::Down) &&
			!(prevStates.hold && data.previous == VirtualKeyState::Hold) &&
			!(prevStates.doubleDown && data.previous == VirtualKeyState::Double)) {
			return false;
		}
	}

	//All pass or there was no requirement set for this
	return  true;
}

bool InputEventContextRule::Check(InputStateMap const& inputState) const {
	for(auto const &cond : conditions) {
		if(!cond.Check(inputState)) {
			return false;
		}
	}
	
	return true;
}

void InputEventContext::AddRule(InputEventContextRule rule) {
	auto ruleUniquePtr = std::make_unique<InputEventContextRule>(std::move(rule));
	auto rulePtr = ruleUniquePtr.get();
	rules.emplace_back(std::move(ruleUniquePtr));

	for (auto & condition : rulePtr->conditions) {
		if (weave::input::SupportsCursor(condition.key)) {
			condition.states.any = true;
			condition.prevStates.any = true;
		}

		if (condition.isTrigger) {
			auto &ruleVector = triggeredRules[{condition.device, condition.key}];
			ruleVector.emplace_back(rulePtr);

			std::sort(std::begin(ruleVector), std::end(ruleVector), [](auto const& a, auto const& b) {
				if (a->priority > b->priority)
					return true;
				return a->conditions.size() > b->conditions.size();
			});
		}
	}
}


void InputEventContext::InputEvent(InputEventData inputData, InputStateMap& inputState) {
	
	auto it = triggeredRules.find({ inputData.device, inputData.key });
	if (it == std::end(triggeredRules)) {
		return NextProcessor(inputData, inputState);
	}

	//The rules are pre ordered by their priority and condition complexity so to catch complex combinations first
	bool ruleTriggered = false;
	for(auto* rule : it->second) {
		if(rule->Check(inputState)) {
			ruleTriggered = true;
			
			SendEvent(*rule, inputState);
			if(rule->forwardMode == InputEventForwardMode::Sink) {
				break;
			}
		}
	}

	if (!(forwardMode == InputEventForwardMode::Sink && ruleTriggered)) {
		NextProcessor(inputData, inputState);
	}
}

void InputEventContext::SendEvent(InputEventContextRule const& rule, InputStateMap& inputState) const {
	auto message = inputState.CreateMessage(rule.eventId, rule.feedbackRequests.size());

	size_t reportIndex = 0;
	for(auto const &request : rule.feedbackRequests) {
		message.report[reportIndex++] = inputState.QueryKeyData(request.first, request.second);
	}

	inputState.WriteMessage(message);
}

/*
* TODO: XML support
//Creates a new input rule based on the supplied XML node
void InputEventContext::CreateXMLInputRule(pugi::xml_node root) {
	//Rules are required to have the name attribute set
	if(!root.attribute("name"))
		return;

	//Create a Rule to fill in with the conditions extracted from the body
	auto rule = std::make_unique<InputEventContextRule>();
	//Hash the name into an ID
	std::string name = root.attribute("name").as_string();
	rule->id = Message::HashMessageName(name);

	//Check the bubble flag. By default bubble is set to true and it can be deactivated by any rule. Bubbling will allow rules with the exact same combinations to fire at the same time.
	rule->bubble = root.attribute("bubble").as_bool(rule->bubble);

	//Parse each input node from the action
	for(auto input = root.first_element_by_path("input"); input != nullptr; input = input.next_sibling("input")) {
		//Add the condition to the list in the rule. This will set the master flag if set
		rule->conditions.push_back(ParseXMLInputCondition(input));
	}

	//Parse the feedback section where the rule states the kind of data it wants to be sent with the event.
	//This is done before sorting the conditions because we want to keep the data in the same order specified by the input.
	ParseXMLFeedbackNode(root.first_element_by_path("feedback"), *rule);

	//Sort the conditions so that the master condition is the first
	std::sort(rule->conditions.begin(), rule->conditions.end(), [](InputEventContextCondition &c1, InputEventContextCondition &) {
		return c1.master;
	});

	//Once all the conditions have been created for the rule, scan the rule to see what keycodes it applies to
	for(auto i = rule->conditions.begin(), end = rule->conditions.end(); i<end; ++i) {
		uint64_t id = input::VirtualDeviceKeyPair(i->dev, i->key);
		//Insert the rule to the related device/key combo. This duplicates rules for all the keys involved so that it can be caught whenever an event comes in from whichever of the involved keys
		rulesDevKey[id].push_back(rule.get());
		//Sort the rules so that they are always ordered by the ones with more combinations
		std::sort(rulesDevKey[id].begin(), rulesDevKey[id].end(), [](InputEventContextRule *r1, InputEventContextRule *r2) {
			return r1->conditions.size() > r2->conditions.size();
		});

		//Stop assigning the rule to the keycodes if there's a master condition within the rule
		if(i->master)
			break;
	}

	//Store the generated rule for later removal
	rules.push_back(std::move(rule));
}

//Every attribute is converted to a string, stripped of any commas and converted to lower case with this helper method
inline std::string fixAttribute(char const *att){
	if(!att) {
		return "none";
	}

	std::string buff = att;
	std::replace(buff.begin(), buff.end(), ',', ' ');
	std::transform(buff.begin(), buff.end(), buff.begin(), [](char c) { return (char)::tolower((int)c); });
	return buff;
};

//Parses an XML based input condition and returns a new object with it
InputEventContextCondition InputEventContext::ParseXMLInputCondition(pugi::xml_node root) {
	//Create the instance that will be returned
	InputEventContextCondition cond;

	//Check for every possible attribute of the node
	cond.key = input::VirtualKeyValue("key_" + std::string(root.attribute("key").as_string()));
	cond.dev = input::VirtualDeviceValue("device_" + std::string(root.attribute("device").as_string()));
	cond.master = root.attribute("master").as_bool();
	
	//States can have multiple values listed
	if(auto att = root.attribute("state")) {
		//Split the string into members
		std::stringstream ss(fixAttribute(att.as_string()));
		std::string name;
		while(ss >> name) {
			cond.states |= input::VirtualKeyStateBit(input::VirtualKeyStateValue("state_" + name));
		}
	}

	if(auto att = root.attribute("prevstate")) {
		//Split the string into members
		std::stringstream ss(fixAttribute(att.as_string()));
		std::string name;
		while(ss >> name) {
			cond.prevStates |= input::VirtualKeyStateBit(input::VirtualKeyStateValue("state_" + name));
		}
	}

	return cond;
}

//Parses an XML node with feedback rules and fills the supplied rule object with the result
void InputEventContext::ParseXMLFeedbackNode(pugi::xml_node root, InputEventContextRule &rule) {
	using namespace weave::input;
	//No feedback node will indicate that the default feedback is expected, which is nothing for key entries, cursor positions for every cursor key and pressure data for every gamepad key
	//The feedback node can define the attribute default="true", where it will include the default behaviour on top of the items on the feedback list while
	//Not including this flag will tell the feedback builder that only the list items are intended as feedback.
	
	//Set the default feedback first, if required. This is when there's no node or the attribute default is set.
	if(!root || root.attribute("default").as_bool()) {
		//Default feedback goes through all the rule's conditions and flags them for feedback if appropiate
		for(auto const &cond : rule.conditions) {
			if(SupportsCursor(cond.key) || SupportsPressure(cond.key)) {
				rule.feedbackRequests.emplace_back(cond.dev, cond.key);
			}
		}
	}

	//Finally add in any other information requested in the list
	if(root) {
		for(auto feed = root.first_element_by_path("request"); feed != nullptr; feed = feed.next_sibling("request")) {
			//Request nodes simply have a key and device attribute set, like input nodes.
			if(feed.attribute("key") && feed.attribute("device")) {
				auto key = VirtualKeyValue("key_" + std::string(feed.attribute("key").as_string()));
				auto dev = VirtualDeviceValue("device_" + std::string(feed.attribute("device").as_string()));
				rule.feedbackRequests.emplace_back(dev, key);
			}
		}
	}
}



//Resource interface implementation
Resource::LoadStates InputEventContext::LoadXML(pugi::xml_node root) {

	InputContext::LoadXML(root);

	//Go through all the action nodes found and parse them individually
	for(auto action = root.first_element_by_path("action"); action != nullptr; action = action.next_sibling("action")) {
		CreateXMLInputRule(action);
	}

	return Resource::LoadStates::OK;
}

*/
