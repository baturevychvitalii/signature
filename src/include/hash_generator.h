#ifndef __SYNCHRONIZER__
#define __SYNCHRONIZER__

#include <type_traits>
#include <fstream>

#include "thread_pool.h"

class input;

class hash_generator
{
	using kilobytes_to_bytes = std::ratio<1024,1>::type;
	using byte = char;

	/*
	 * size of a block to process for single task unit (in bytes)
	 */
	const std::size_t block_size;

	/*
	 * file size devided by block size
	 * (this much tasks will be launched)
	 */
	std::size_t amount_of_blocks;

	/*
	 * if set to true more info will be displayed
	 */
	const bool verbose;

	/*
	 * file where hash results will be saved
	 */
	std::ofstream signature_file;

	/*
	 * name of the file, from which input will be read
	 */
	const std::string input_file_name;

	/*
	 * size of input file (in bytes)
	 */
	std::size_t input_file_size;

	/*
	 * folowing declaration says that we are using thread pool
	 * and each thread will have it's own ifstream object and
	 * char array, so we don't create ifstream for each task
	 * and don't allocate and delete char buffer with each task,
	 * but keep them alive for all tasks, launched by thread
	 */
	thread_pool<std::ifstream, std::shared_ptr<byte[]>> pool;


	/*
	 * folowing lines will determine at compile time 
	 * hash functor type, hash output type and
	 * size of one output hash from one input block
	 */
	using hasher = std::hash<std::string>;
	using hash_return_t = std::invoke_result_t<hasher, std::string>;
	static constexpr std::size_t out_block_size = sizeof(hash_return_t);

	void do_one_block(
		std::ifstream & thread_file_instance,
		std::shared_ptr<byte[]> thread_buffer,
		std::size_t id
	);

	public:
		/*
		 * will ensure that input file exists and
		 * output file is created and empty, available
		 * for writing to it
		 */
		hash_generator(const input & user_input);

		/*
		 * starts n_threads threads for signature generation
		 * @return true if no exceptions were thrown, false otherwise
		 */
		bool operator()(std::size_t n_threads);
};

#endif
