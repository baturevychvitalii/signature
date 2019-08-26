#include <iostream>
#include <iomanip>

#include "include/hash_generator.h"
#include "include/input.h"
#include "include/progress_bar.h"
#include "include/buffered_reader.h"
#include "include/buffered_writer.h"


hash_generator::hash_generator(const input & user_input, hash_generator::hash_function_t func, std::size_t hash_block_len)
	: hash_function(func),
	hash_block_size(hash_block_len),
	block_size(user_input.get_block_size<kilobytes>()),
	verbose(user_input.is_verbose()),
	output_file_name(user_input.get_output_file()),
	input_file_name(user_input.get_input_file())
{
}

void hash_generator::resolve_input_file_size(std::size_t input_file_size)
{
	// determine block size, amount of blocks and last block size
	if (input_file_size % block_size == 0)
	{
		last_block_size = block_size;
		blocks_n = input_file_size / block_size;
	}
	else
	{
		last_block_size = input_file_size % block_size;
		blocks_n = input_file_size / block_size + 1;
	}

	if (verbose)
		std::cout << "will process " << blocks_n << " blocks (" << block_size << " bytes each)\n"
			<< "output file size must be " << blocks_n * hash_block_size << " bytes" << std::endl;
}

/*
 * thread splitting logic
 */
struct thread_division
{
	thread_division(std::size_t available)
	{
		if (available < 4) available = 4; //at least 3 needed (current, one for reading and one for computing)
		available --; //remove current thread
		readers = available / 3 * 2;
		computers = available - readers;
	}

	std::size_t readers, computers;
};

bool hash_generator::operator()(std::size_t threads)
{
	thread_division division(threads);

	buffered_reader reader(input_file_name, 4 * 1024 * 1024 * 1024L, block_size, division.readers, verbose); 
	resolve_input_file_size(reader.get_file_size());
	init_threads(division.computers);
	auto futures = init_tasks(reader);

	return geather_results(futures);
}

void hash_generator::init_threads(std::size_t amount)
{
	if (amount < 1)
		throw std::invalid_argument("amount of threads can't be < 1");

	if (verbose)
		std::cout << "creating " << amount << " thread environment for hash computing" << std::endl;

	for (std::size_t i = 0 ; i < amount; i++)
		pool.add_thread(byte_arr(new byte[block_size]));
}

std::queue<std::future<std::unique_ptr<hash_generator::byte[]>>> hash_generator::init_tasks(buffered_reader & reader)
{
	if (verbose)
		std::cout << "initializing tasks\n" << std::flush;

	std::queue<std::future<std::unique_ptr<byte[]>>> futures;

	// push all tasks
	for (std::size_t i = 0; i < blocks_n - 1; i++)
	{
		futures.emplace(
			pool.push_task(
				[this, i, &reader](byte_arr & arr)
				{
					reader.copy(i, arr.get(), block_size);
					return hash_function(arr, block_size);
				}
			)
		);
	}

	futures.emplace(
		pool.push_task(
			[this, &reader](byte_arr & arr)
			{
				reader.copy(blocks_n - 1, arr.get(), last_block_size);
				return hash_function(arr, last_block_size);
			}
		)
	);

	if (verbose)
		std::cout << "done initializing tasks\n" << std::flush;

	return futures;
}

bool hash_generator::geather_results(std::queue<std::future<ubyte_arr>> & futures)
{
	// wait for them to finish in right sequence and
	// write results to a file
	progress_bar bar(blocks_n, '@', 20);
	bool all_right {true};

	{
		buffered_writer writer(output_file_name, hash_block_size, 1024*10);
		for (std::size_t i = 0; i < blocks_n; i++)
		{
			try
			{
				writer << futures.front().get().get();
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what();
				all_right = false;
				break;
			}
			
			futures.pop();

			if (verbose)
				bar.update(i + 1);
		}
	}
	 
	// signal threads
	pool.finish();
	return all_right;
}
