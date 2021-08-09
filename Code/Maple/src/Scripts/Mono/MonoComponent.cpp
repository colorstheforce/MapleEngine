
#include "MonoComponent.h"
#include "MonoScript.h"



namespace Maple 
{
	auto MonoComponent::addScript(const std::string& name, MonoSystem* system) -> void
	{
		if (scripts.find(name) == scripts.end()) {
			scripts.emplace(name, std::make_shared<MonoScript>(name,this, system));
		}
	}

	auto MonoComponent::remove(const std::string& script) -> void
	{
		scripts.erase(script);
	}

};

