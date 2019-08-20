#include <future>
#include <chrono>
#include <iostream>
#include <fstream>


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

	

	return 0;
}
