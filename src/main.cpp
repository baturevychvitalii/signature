#include <future>
#include <chrono>
#include <iostream>
#include <fstream>

void varvar(int seconds)
{
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
	std::cout << "almost finished" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(seconds/2));
}

#include "include/input.h"
int main (int argc, char * argv[])
{
	Input input(argc, argv, std::cerr);

	#ifdef DEBUG
	std::cout << "showing input:\n" << input << std::endl;
	#endif

	if (input.is_bad())
	{
		return 1;
	}

	auto fut = std::async(std::launch::async, varvar, 2);
	std::cout << "main thread is still going" << std::endl;
	fut.wait();
	std::cout << "varvar finished\n";

	return 0;
}
