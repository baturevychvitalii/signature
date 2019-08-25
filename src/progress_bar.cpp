#include <iostream>

#include "include/progress_bar.h"

progress_bar::progress_bar(std::size_t max, char symbol, std::size_t length)
	: max_value(max), max_length(length), sym(symbol)
{
}

void progress_bar::update(std::size_t current)
{
	// clear old one
	for (std::size_t i = 0; i < line.length(); i++)
		std::cout << '\b';

	std::size_t symbs_n = current * max_length / max_value;
	ss.str(""); // clear string stream

	ss << "[";
	for (std::size_t i = 0; i < symbs_n; i++)
		ss << sym;

	for (std::size_t i = 0; i < max_length - symbs_n; i++)
		ss << ' ';

	ss << "] ... (" << current << "/" << max_value << ")";
	line = ss.str();
	std::cout << line << std::flush;
}
