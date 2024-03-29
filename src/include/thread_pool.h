#ifndef __THREAD_POOOOOL__
#define __THREAD_POOOOOL__

#include <queue>
#include <vector>
#include <future>
#include <thread>
#include <mutex>

#include "custom_exceptions.h"

template <class ... ArgsOwnedByThread>
class thread_pool
{
	using task_type = std::packaged_task<void(ArgsOwnedByThread & ...)>;
	public:
		thread_pool()
			: job_done(false)
		{
		}

		/*
		* waits for other threads to join
		*/
		~thread_pool()
		{
			// wait for other threads to join
			for (auto && t : workers)
			{
				try
				{
					t.get();
				}
				catch (const std::exception & e)
				{
					std::cerr << e.what() << " when joining threads";
				}
			}
		}

		/*
		* add task for workers
		* @return future, linked to newly pushed task
		*/
		template<typename Func, typename Ret = std::invoke_result_t<Func, ArgsOwnedByThread & ...>>
		std::future<Ret> push_task(Func && task)
		{
			std::packaged_task<Ret(ArgsOwnedByThread & ...)> pt(std::forward<Func>(task));
			auto fut = pt.get_future();

			{
				std::scoped_lock<std::mutex> l(m);
				tasks.emplace(std::move(pt));
			}

			cond_var.notify_one(); // wake one thread to work on the task

			return fut;
		}

		void add_thread(ArgsOwnedByThread && ... args_for_thread)
		{
			workers.push_back(
				std::async(
					std::launch::async,
					&thread_pool::thread_routine,
					this,
					std::forward<ArgsOwnedByThread>(args_for_thread)...
				)
			);
		}
		
		/*
		* sets job_done flag to true
		* waits for threads to join
		*/
		void finish()
		{
			if (job_done)
				throw paralel_exception();
			
			job_done = true;
			cond_var.notify_all();
		}

	private:
		/*
		* standard multithread utilities
		*/
		std::mutex m;
		std::condition_variable cond_var;

		/*
		* packaged tasks are being given pointer to this thread pool
		* this way when some special task finishes job - it can set job finished flag
		*/
		std::queue<task_type> tasks;
		std::vector<std::future<void>> workers;
		std::atomic<bool> job_done;

		// the work that a worker thread does:
		void thread_routine(ArgsOwnedByThread ... args_for_tasks)
		{
			task_type next_task;

			while(!job_done)
			{
				{
					std::unique_lock<std::mutex> l(m);
					if (tasks.empty())
					{
						cond_var.wait(l,[this]{return !tasks.empty() || job_done;});

						if (job_done)
							return;
					}

					next_task = std::move(tasks.front());
					tasks.pop();
				}

				// note: args are not forwarded, because this thread shall be
				// their owner. this is the point of this args, that they are
				// thread specific
				next_task(args_for_tasks ...);
			}
		}
};

#endif
