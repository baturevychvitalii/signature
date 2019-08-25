#ifndef __PROGRESS_BAR__
#define __PROGRESS_BAR__ 
#include <iostream>
#include <memory>

struct progress_bar
{
	progress_bar(std::size_t max, char symbol, std::size_t length)
		: max_value(max), max_length(length), sym(symbol),
		bar(std::make_unique<char[]>(length + 3))
	{
		bar[0] = '[';
		for (std::size_t i = 1; i < length + 1; i++)
			bar[i] = ' ';
		bar[length + 3 - 1] = '\0';
		bar[length + 3 - 2] = ']';

		// make sth to erase
		for (std::size_t i = 0; i < length + 3; i++)
			std::cout << ' ';
	}

	void update(std::size_t current) const
	{
		for (std::size_t i = 1; i < current * max_length / max_value; i++)
			bar[i] = sym;
	}

	std::ostream & print (std::ostream & os) const
	{
		return os << bar.get();
	}

	std::ostream & clear(std::ostream & os) const
	{
		for (std::size_t i = 0; i < max_length + 3; i++)
			os << "\b";

		return os;
	}

	friend std::ostream & operator << (std::ostream &, const progress_bar &);

	private:
		const std::size_t max_value, max_length;
		char sym;
		std::unique_ptr<char[]> bar;
};

std::ostream & operator << (std::ostream & os, const progress_bar & bar)
{
	bar.clear(os);
	bar.print(os);
	return os << std::flush;
}

#endif
