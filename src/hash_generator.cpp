#include <iostream>
#include <iomanip>
#include <functional>
#include <type_traits>

#include <openssl/sha.h>

#include "include/hash_generator.h"
#include "include/input.h"


hash_generator::hash_generator(const input & user_input)
	: block_size(user_input.get_block_size<kilobytes_to_bytes>()),
	verbose(user_input.is_verbose()),
	input_file_name(user_input.get_input_file())
{
	// check if input file is possible to read
	{
		std::ifstream input_file(input_file_name, std::ios_base::binary | std::ios_base::ate);
		if (!input_file.is_open())
		{
			throw file_exception("couldn't open input file for reading");
		}

		// since file was opened wit ios_base::ate size can be determined immediately
		input_file_size = input_file.tellg();

		if (input_file_size == 0)
		{
			throw std::invalid_argument("file is empty!");
		}

	}

	// check if output file is possible to write to
	signature_file.open(user_input.get_output_file(), std::ios_base::trunc);
	if (!signature_file.is_open())
	{
		throw file_exception("couldn't open output file for writing");
	}

	// add zeros to the end of file if it's incompatiple with block size
	// so there are no checks during task execution
	// because constructor is one, and tasks are legion
	if (input_file_size % block_size != 0)
	{
		std::size_t missing = block_size - input_file_size % block_size;
		std::ofstream tmp(input_file_name, std::ios_base::binary | std::ios_base::ate);

		std::unique_ptr<char[]> tmp_ptr = std::make_unique<char[]>(missing);
		std::fill_n(tmp_ptr.get(), missing, 0);
		tmp.write(tmp_ptr.get(), missing);
		input_file_size += missing;
	}
}

bool hash_generator::operator()(std::size_t threads)
{
	threads = threads > 0 ? threads : 1;

	for (std::size_t i = 0 ; i < threads; i++)
	{
		pool.add_thread(
			std::ifstream(input_file_name, std::ios_base::binary),
			std::make_shared<byte[]>(block_size) 
		);
	}

	std::size_t blocks = input_file_size / block_size;
	std::size_t done = 0;

	std::queue<std::future<std::string>> futures;

	// push all tasks
	for (std::size_t i = 0; i < blocks; i++)
	{
		futures.emplace(
			pool.push_task(
				std::bind(
					&hash_generator::do_one_block,
					this,
					std::placeholders::_1,
					std::placeholders::_2,
					i
				)
			)
		);
	}

	// wait for them to finish in right sequence and
	// write results to a file
	for (std::size_t i = 0; i < blocks; i++)
	{
		signature_file << futures.front().get();
		futures.pop();
	}
	 
	pool.finish();
}


std::string hash_generator::do_one_block(
	std::ifstream & thread_file_instance,
	byte_arr thread_buffer,
	std::size_t id
)
{
	if (!thread_file_instance.is_open())
	{
		throw file_exception("file must be open by thread");
	}

	thread_file_instance.seekg(id * block_size);
	thread_file_instance.read(thread_buffer.get(), block_size);

	return hash(thread_buffer, block_size);
}

std::string hash_generator::hash(byte_arr bytes, size_t size)
{
	unsigned char result[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, bytes.get(), size);
	SHA256_Final(result, &sha256);
	std::stringstream ss;
	for (int i = 0; i < SHA224_DIGEST_LENGTH; i++)
	{
		ss << std::ios_base::hex << std::setw(2) << std::setfill('0') << (int)result[i];
	}

	return ss.str();
}
