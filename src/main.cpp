#include <iostream>


#include "include/hash_generator.h"

#include "include/input.h"
int main (int argc, char * argv[])
{
	input input(argc, argv, std::cerr);

	#ifdef DEBUG
	std::cout << "showing input:\n" << input << std::endl;
	#endif

	if (input.is_bad())
	{
		return 1;
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

	if(!(*generate_hash)(std::thread::hardware_concurrency()))
	{
		std::cout << "errors occured during hash generation" << std::endl;
		return 2;
	}



	std::cout << "hash successfully generated and stored into " << input.get_output_file() << std::endl;
	return 0;
}
