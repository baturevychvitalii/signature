#include <iostream>

#include "include/hash_generator.h"
#include "include/input.h"
int main (int argc, char * argv[])
{
	input input(argc, argv, std::cerr);

	if (input.is_bad())
	{
		return 1;
	}

	if (input.is_verbose())
	{
		std::cout << input << std::endl;
	}

	std::unique_ptr<hash_generator> generate_hash;

	try
	{
		generate_hash = std::make_unique<hash_generator>(input);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	//if(!(*generate_hash)(std::thread::hardware_concurrency()))
	if(!(*generate_hash)(1))
	{
		std::cout << "errors occured during hash generation" << std::endl;
		return 2;
	}



	std::cout << "hash successfully generated and stored into " << input.get_output_file() << std::endl;
	return 0;
}
