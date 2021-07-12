//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include <cstdint>
#include <chrono>

namespace Maple
{
	// Small class for measuring elapsed time between game loops.
	class Timer
	{
	public:
		Timer()
		{
			start();
		}

		~Timer()
		{
		}

		// Start the timer by setting the last measurement to now.
		auto start() -> void
		{
			point = clock::now();
			prev = current();
		}

		// Return time elapsed since the last measurement.
		auto stop() -> int64_t
		{
			clock::time_point last = point;
			point = clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(point - last);
			return duration.count();
		}

		auto currentTimestamp()
		{
			auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
			auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
			return tmp.count();
		}

		auto current() -> std::chrono::high_resolution_clock::time_point
		{
			return std::chrono::high_resolution_clock::now();
		}

		auto elapsed(std::chrono::high_resolution_clock::time_point begin,
					 std::chrono::high_resolution_clock::time_point end)
		{
			return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
		}

		auto step()-> float
		{
			auto currTime = current();

			auto ela =  elapsed(prev, currTime);

			prev = currTime;

			return ela / 1000000.f;
		}


	private:
		using clock = std::chrono::high_resolution_clock;
		clock::time_point point;

		std::chrono::high_resolution_clock::time_point prev;
	};

};
