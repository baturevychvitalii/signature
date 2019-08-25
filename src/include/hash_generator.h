#ifndef __SYNCHRONIZER__
#define __SYNCHRONIZER__

#include <fstream>

#include "thread_pool.h"

class input;


class hash_generator
{
	using kilobytes_to_bytes = std::ratio<1024,1>::type;

	public:
		using byte = char;
		using byte_arr = std::shared_ptr<byte[]>;
		using ubyte_arr = std::unique_ptr<byte[]>;
		using hash_function_t = ubyte_arr (*)(byte_arr, std::size_t);

	private:
		hash_function_t hash_function;
		std::size_t hash_block_size;

		/*
		* size of a block to process for single task unit (in bytes)
		*/
		const std::size_t block_size;
		std::size_t last_block_size;

		/*
		* amount of blocks in a file (including the last one)
		*/
		std::size_t blocks_n;

		/*
		* if set to true more info will be displayed
		*/
		const bool verbose;

		
		/*
		* name of the file, from which input will be read
		*/
		const std::string input_file_name;
		const std::string output_file_name;


		/*
		* folowing declaration says that we are using thread pool
		* and each thread will have it's own ifstream object and
		* char array, so we don't create ifstream for each task
		* and don't allocate and delete char buffer with each task,
		* but keep them alive for all tasks, launched by thread
		*/
		thread_pool<std::ifstream, byte_arr> pool;

		void init_threads(std::size_t amount);
		
		std::queue<std::future<ubyte_arr>> init_tasks();

		bool geather_results(std::queue<std::future<ubyte_arr>> & futures);

		std::size_t check_input_file() const;


		ubyte_arr do_one_block(
			std::ifstream & thread_file_instance,
			byte_arr thread_buffer,
			std::size_t size,
			std::size_t id
		) const;

	public:
		/*
		 * will ensure that input file exists and
		 * output file is created and empty, available
		 * for writing to it
		 */
		hash_generator(const input & user_input, hash_function_t func, std::size_t hash_block_len);

		/*
		 * starts n_threads threads for signature generation
		 * @return true if no exceptions were thrown, false otherwise
		 */
		bool operator()(std::size_t n_threads);
};

#endif
