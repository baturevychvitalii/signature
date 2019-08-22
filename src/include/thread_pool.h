#ifndef __THREAD_POOOOOL__
#define __THREAD_POOOOOL__

#include <queue>
#include <vector>
#include <future>
#include <mutex>

#include "custom_exceptions.h"

template <class ... ArgsOwnedByThread>
class thread_pool
{
	public:
		using task_type = std::packaged_task<void(ArgsOwnedByThread ...)>;

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
		std::future<void> push_task(task_type && task)
		{
			auto fut = task.get_future();

			{
				std::scoped_lock<std::mutex> l(m);
				tasks.push(std::move(task));
			}

			cond_var.notify_one(); // wake one thread to work on the task

			return fut;
		}

		void add_thread(ArgsOwnedByThread&& ... args_for_thread)
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
		void thread_routine(ArgsOwnedByThread & ... args_for_tasks)
		{
			while(!job_done)
			{
				task_type perform_task;

				{
					std::unique_lock<std::mutex> l(m);
					if (tasks.empty())
					{
						cond_var.wait(l,[this]{return !tasks.empty() || job_done;});

						if (job_done)
							return;
					}

					perform_task = std::move(tasks.front());
					tasks.pop();
				}

				// note: args are not forwarded, because this thread shall be
				// their owner. this is the point of this args, that they are
				// owned by thread
				perform_task(args_for_tasks ...);
			}
		}
};

#endif
