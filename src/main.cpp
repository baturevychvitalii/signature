#include <iostream>
#include <boost/crc.hpp>
#include <cstring>

#include "include/hash_generator.h"
#include "include/input.h"

using hash_ret_type = decltype(std::declval<boost::crc_32_type>().checksum());

hash_generator::ubyte_arr crc_hash(hash_generator::byte_arr data, std::size_t size)
{
	boost::crc_32_type result;
	result.process_bytes(data.get(), size);
	hash_generator::ubyte_arr ret = std::make_unique<hash_generator::byte[]>(sizeof(hash_ret_type));
	hash_ret_type outcome = result.checksum();
	std::memcpy(ret.get(), reinterpret_cast<const hash_generator::byte*>(&outcome), sizeof(outcome));
	return ret;
}


int main (int argc, char * argv[])
{
	input input(argc, argv, std::cerr);

	if (input.is_bad())
		return 1;

	if (input.is_verbose())
		std::cout << input << std::endl;

	std::unique_ptr<hash_generator> generate_hash;

	try
	{
		generate_hash = std::make_unique<hash_generator>(input, &crc_hash, sizeof(hash_ret_type));
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	if(!(*generate_hash)(std::thread::hardware_concurrency()))
	//if(!(*generate_hash)(1))
	{
		std::cout << "errors occured during hash generation" << std::endl;
		return 2;
	}

	std::cout << "hash successfully generated and stored into " << input.get_output_file() << std::endl;
	return 0;
}
