#include <iostream>
#include <iomanip>

#include "include/hash_generator.h"
#include "include/input.h"
#include "include/progress_bar.h"
#include "include/buffered_writer.h"


hash_generator::hash_generator(const input & user_input, hash_generator::hash_function_t func, std::size_t hash_block_len)
	: hash_function(func),
	hash_block_size(hash_block_len),
	block_size(user_input.get_block_size<kilobytes_to_bytes>()),
	verbose(user_input.is_verbose()),
	input_file_name(user_input.get_input_file()),
	output_file_name(user_input.get_output_file())
{
	auto input_file_size = check_input_file();

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

std::size_t hash_generator::check_input_file() const
{
	std::ifstream input_file(input_file_name, std::ios_base::binary | std::ios_base::ate);
	if (!input_file.is_open())
		throw file_exception("couldn't open input file for reading");

	// since file was opened wit ios_base::ate size can be determined immediately
	std::size_t size = input_file.tellg();

	if (size == 0)
		throw std::invalid_argument("file is empty!");

	if (verbose)
		std::cout << "input file size " << size << " bytes" << std::endl;

	return size;
}

bool hash_generator::operator()(std::size_t threads)
{
	if (threads == 0)
		throw std::invalid_argument("must be at least one thread");

	init_threads(threads);
	auto futures = init_tasks();

	return geather_results(futures);
}

void hash_generator::init_threads(std::size_t amount)
{
	if (verbose)
		std::cout << "creating " << amount << " thread environment" << std::endl;

	for (std::size_t i = 0 ; i < amount; i++)
		pool.add_thread(
			std::ifstream(input_file_name, std::ios_base::binary),
			byte_arr(new byte[block_size])
		);
}

std::queue<std::future<std::unique_ptr<hash_generator::byte[]>>> hash_generator::init_tasks()
{
	if (verbose)
		std::cout << "initializing tasks\n" << std::flush;

	std::queue<std::future<std::unique_ptr<byte[]>>> futures;

	// push all tasks
	for (std::size_t i = 0; i < blocks_n - 1; i++)
	{
		futures.emplace(
			pool.push_task(
				[this, i](std::ifstream & ifs, byte_arr & arr)
				{
					return this->do_one_block(ifs, arr, block_size, i);
				}
			)
		);
	}

	futures.emplace(
		pool.push_task(
			[this](std::ifstream & ifs, byte_arr & arr)
			{
				return this->do_one_block(ifs, arr, last_block_size, blocks_n - 1);
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

hash_generator::ubyte_arr  hash_generator::do_one_block(
	std::ifstream & thread_file_instance,
	byte_arr thread_buffer,
	std::size_t size,
	std::size_t id
) const
{
	if (!thread_file_instance.is_open())
		throw file_exception("input file must be open by thread");
	
	thread_file_instance.seekg(id * block_size);
	thread_file_instance.read(thread_buffer.get(), size);
	return hash_function(thread_buffer, size);
}

