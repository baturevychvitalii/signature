#include <iostream>
#include <iomanip>

#include <openssl/sha.h>

#include "include/hash_generator.h"
#include "include/input.h"
#include "include/progress_bar.h"


hash_generator::hash_generator(const input & user_input)
	: block_size(user_input.get_block_size<kilobytes_to_bytes>()),
	verbose(user_input.is_verbose()),
	input_file_name(user_input.get_input_file()),
	output_file_name(user_input.get_output_file())
{
	auto input_file_size = check_input_file();


	// check if output file is possible to write to
	std::ofstream signature_file(output_file_name, std::ios_base::trunc);
	if (!signature_file.is_open())
		throw file_exception("couldn't open output file for writing");

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

	// wait for them to finish in right sequence and
	// write results to a file
	progress_bar bar(blocks_n, '#', 15);
	bool all_right {true};
	for (std::size_t i = 0; i < blocks_n; i++)
	{
		try
		{
			futures.front().get();
			if (verbose)
			{
				bar.update(i+1);
				std::cout << bar;
			}
		}
		catch(const std::exception& e)
		{
			all_right = false;
			break;
		}
		
		futures.pop();
	}
	 
	pool.finish();
	return all_right;
}

void hash_generator::init_threads(std::size_t amount)
{
	if (verbose)
		std::cout << "creating " << amount << " thread environment" << std::endl;

	for (std::size_t i = 0 ; i < amount; i++)
	{
		pool.add_thread(
			std::ifstream(input_file_name, std::ios_base::binary),
			std::ofstream(output_file_name, std::ios_base::binary | std::ios_base::ate),
			byte_arr(new byte[block_size]),
			byte_arr(new byte[SHA256_DIGEST_LENGTH])
		);
	}
}

std::queue<std::future<void>> hash_generator::init_tasks()
{
	if (verbose)
		std::cout << "initializing tasks\n" << std::flush;

	std::queue<std::future<void>> futures;

	// push all tasks
	for (std::size_t i = 0; i < blocks_n - 1; i++)
	{
		futures.emplace(
			pool.push_task(
				[this, i](std::ifstream & ifs, std::ofstream & ofs, byte_arr & arr1, byte_arr & arr2)
				{
					return this->do_one_block(ifs, ofs, arr1, arr2, block_size, i);
				}
			)
		);
	}

	futures.emplace(
		pool.push_task(
			[this](std::ifstream & ifs, std::ofstream & ofs, byte_arr & arr1, byte_arr & arr2)
			{
				return this->do_one_block(ifs, ofs, arr1, arr2, last_block_size, blocks_n - 1);
			}
		)
	);

	if (verbose)
		std::cout << "done initialializing tasks\n" << std::flush;

	return futures;
}


void hash(hash_generator::byte_arr bytes, hash_generator::byte_arr out, size_t size)
{
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, bytes.get(), size);
	SHA256_Final(reinterpret_cast<unsigned char *>(out.get()), &sha256);
}

void hash_generator::do_one_block(
	std::ifstream & thread_file_instance,
	std::ofstream & thread_out_instance,
	byte_arr thread_buffer,
	byte_arr thread_buffer_hash,
	std::size_t size,
	std::size_t id
) const
{
	if (!thread_file_instance.is_open())
		throw file_exception("input file must be open by thread");
	
	if (!thread_out_instance.is_open())
		throw file_exception("output ifle must be open by thread");


	thread_file_instance.seekg(id * block_size);
	thread_file_instance.read(thread_buffer.get(), size);

	hash(thread_buffer, thread_buffer_hash, size);
	thread_out_instance.seekp(id * SHA256_DIGEST_LENGTH);
	thread_out_instance.write(thread_buffer_hash.get(), SHA256_DIGEST_LENGTH);
}

