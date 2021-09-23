///////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "ThreadPool.h"
#include "Application.h"
#include "Engine/Profiler.h"

namespace Maple
{
	auto Thread::sleep(int64_t ms) -> void
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	Thread::Thread(const std::string& name)
	{
		thread = std::make_shared<std::thread>(&Thread::run, this);
		PROFILE_SETTHREADNAME(name.c_str());
	}

	Thread::~Thread()
	{
		if (thread->joinable())
		{
			wait();
			mutex.lock();
			close = true;
			condition.notify_one();
			mutex.unlock();
			thread->join();
		}
	}

	auto Thread::wait() -> void
	{
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this]() {
			return jobs.empty();
			});
	}
	auto Thread::getTaskSize() -> int32_t
	{
		std::lock_guard<std::mutex> lock(mutex);
		return jobs.size();
	}

	auto Thread::addTask(const Task& task) -> void
	{
		std::lock_guard<std::mutex> lock(mutex);
		jobs.emplace_back(task);
		condition.notify_one();
	}

	auto Thread::addTask(const std::function<void*()> & job, const std::function<void(void*)> & complete) -> void
	{
		std::lock_guard<std::mutex> lock(mutex);
		jobs.emplace_back(job, complete);
		condition.notify_one();
	}

	auto Thread::run() -> void
	{
		while (true)
		{
			Task task;
			{
				std::unique_lock<std::mutex> lock(mutex);
				condition.wait(lock, [this] {
					return !jobs.empty() || close;
					});

				if (close)
				{
					break;
				}

				task = jobs.front();
			}

			if (task.job)
			{
				void* result = task.job();
				if (task.complete)
				{
					Application::get()->postOnMainThread([=]() {
						task.complete(result);
						return true;
					});
					/*if (task.wait)
					{
						future.wait();
					}*/
				}
			}
			{
				std::lock_guard<std::mutex> lock(mutex);
				jobs.pop_front();
				condition.notify_one();
			}
		}
	}


	ThreadPool::ThreadPool(int32_t count)
	{
		for (int32_t i = 0; i < count; i++)
		{
			threads.emplace_back(std::make_shared<Thread>("Thread:"+std::to_string(i)));
		}
	}

	auto ThreadPool::waitAll() -> void
	{
		for (auto& i : threads)
		{
			i->wait();
		}
	}

	auto ThreadPool::addTask(const std::function<void*()> & job, const std::function<void(void*)> & complete, int32_t threadIndex) -> void
	{
		if (threadIndex >= 0 && threadIndex < threads.size())
		{
			threads[threadIndex]->addTask(job, complete);
		}
		else
		{
			int32_t minLen = INT32_MAX;
			int32_t minIndex = -1;

			for (int32_t i = 0; i < threads.size(); ++i)
			{
				auto len = threads[i]->getTaskSize();
				if (minLen > len)
				{
					minLen = len;
					minIndex = i;
					if (minLen == 0)
					{
						break;
					}
				}
			}
			threads[minIndex]->addTask(job, complete);
		}
	}

	auto ThreadPool::addTask(const Thread::Task& task, int32_t threadIndex) -> void
	{
		if (threadIndex >= 0 && threadIndex < threads.size())
		{
			threads[threadIndex]->addTask(task);
		}
		else
		{
			int32_t minLen = INT32_MAX;
			int32_t minIndex = -1;

			for (int32_t i = 0; i < threads.size(); ++i)
			{
				auto len = threads[i]->getTaskSize();
				if (minLen > len)
				{
					minLen = len;
					minIndex = i;
					if (minLen == 0)
					{
						break;
					}
				}
			}
			threads[minIndex]->addTask(task);
		}
	}
};