#ifndef __PROGRESS_BAR__
#define __PROGRESS_BAR__ 

#include <sstream>

struct progress_bar
{
	progress_bar(std::size_t max, char symbol, std::size_t length);

	void update(std::size_t current);

	private:
		const std::size_t max_value, max_length;
		char sym;
		std::stringstream ss;
		std::string line;
};

#endif
