#include <iostream>

#include "include/thread_pool.h"

std::future<void> thread_pool::push_task(std::packaged_task<void(thread_pool *)> && task)
{
	auto fut = task.get_future();

	{
		std::scoped_lock<std::mutex> l(m);
		tasks.push(std::move(task));
	}

	cond_var.notify_one(); // wake one thread to work on the task

	return fut;
}

void thread_pool::start(std::size_t n_threads)
{
	for (std::size_t i = 0; i < n_threads; i++)
	{
		// each thread is a std::async running this->thread_task():
		workers.push_back(
			std::async(std::launch::async, &worker_routine, this)
		);
	}
}

void thread_pool::finish()
{
	if (job_done)
		throw paralel_exception();
	
	job_done = true;
	cond_var.notify_all();

}


void thread_pool::worker_routine()
{
	while(!job_done)
	{
		std::packaged_task<void(thread_pool *)> task_for_me;

		{
			std::unique_lock<std::mutex> l(m);
			if (tasks.empty())
			{
				cond_var.wait(l,[this]{return !tasks.empty() || job_done;});

				if (job_done)
					return;
			}

			task_for_me = std::move(tasks.front());
			tasks.pop();
		}

		task_for_me(this);
	}
}
