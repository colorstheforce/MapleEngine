//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <queue>
#include <functional>
#include <set>
#include <future>
#include "Event.h"
#include "EventHandler.h"
#include "Engine/Core.h"

namespace Maple 
{
	class MAPLE_EXPORT EventDispatcher final
	{
	public:
		EventDispatcher();
		~EventDispatcher();

		EventDispatcher(const EventDispatcher&) = delete;
		EventDispatcher& operator=(const EventDispatcher&) = delete;

		EventDispatcher(EventDispatcher&&) = delete;
		EventDispatcher& operator=(EventDispatcher&&) = delete;

		auto addEventHandler(EventHandler* handler) -> void;
		auto removeEventHandler(EventHandler* handler) -> void;
		auto dispatchEvent(std::unique_ptr<Event>&& event) -> bool;

		auto postEvent(std::unique_ptr<Event>&& event)->std::future<bool>;

		auto dispatchEvents() -> void;

	private:
		std::vector<EventHandler *> eventHandlers;
		std::set<EventHandler *> eventHandlerAddSet;
		std::set<EventHandler *> eventHandlerDeleteSet;

		std::mutex eventQueueMutex;
		std::queue<std::pair<std::promise<bool>, std::unique_ptr<Event>>> eventQueue;
	};

};

