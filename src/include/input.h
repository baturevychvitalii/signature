#ifndef __INPUT_HANDLER__
#define __INPUT_HANDLER__
#include <string>

struct input
{
	input(int argc, char * argv[], std::ostream & error_stream);
	input(const input & l) = default;
	input(input && r) = default;

	std::string get_input_file() const;
	std::string get_output_file() const;
	unsigned short get_block_size() const;
	bool is_verbose() const;
	bool is_bad() const noexcept;

	private:
		bool bad;
		std::string inputfile, outputfile;
		unsigned short b_size;
		bool verbose;

		void check_bad() const;
};

#ifdef DEBUG
std::ostream & operator << (std::ostream & os, const input & i);
#endif

#endif
