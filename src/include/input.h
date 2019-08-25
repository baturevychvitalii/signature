#ifndef __INPUT_HANDLER__
#define __INPUT_HANDLER__
#include <string>
#include <ratio>

struct input
{
	input(int argc, char * argv[], std::ostream & error_stream);
	input(const input & l) = default;
	input(input && r) = default;

	std::string get_input_file() const;
	std::string get_output_file() const;

	
	/*
	 * size of one block to process hash (defaults to kilobytes from user input)
	 * ratio can be used to adjust return value
	 */
	template<typename Ratio = std::ratio<1,1>::type>
	std::size_t get_block_size() const
	{
		check_bad();
		return b_size * Ratio::num / Ratio::den;
	}

	bool is_verbose() const;
	bool is_bad() const noexcept;

	private:
		bool bad;
		std::string inputfile, outputfile;
		std::size_t b_size;
		bool verbose;

		void check_bad() const;
};

std::ostream & operator << (std::ostream & os, const input & i);

#endif
