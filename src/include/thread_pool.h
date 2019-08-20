#ifndef __THREAD_POOOOOL__
#define __THREAD_POOOOOL__

#include <queue>
#include <vector>
#include <future>
#include <mutex>

#include "custom_exceptions.h"

class thread_pool
{


	thread_pool()
		: job_done(false)
	{
	}

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
	std::future<void> push_task(std::packaged_task<void(thread_pool *)> && task);

	/*
	 * start by launching n threads asynchronously.
	 */
	void start(std::size_t n_threads);

	/*
	 * sets job_done flag to true
	 * waits for threads to join
	 */
	void finish();

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
		std::queue<std::packaged_task<void(thread_pool *)>> tasks;
		std::vector<std::future<void>> workers;
		std::atomic<bool> job_done;

		// the work that a worker thread does:
		void worker_routine();
};

#endif
