#ifndef __INPUT_HANDLER__
#define __INPUT_HANDLER__
#include <string>

struct Input
{
	Input(int argc, char * argv[], std::ostream & error_stream);
	Input(const Input & l) = default;
	Input(Input && r) = default;

	std::string in() const;
	std::string out() const;
	unsigned short block_size() const;
	bool is_verbose() const;

	bool is_bad() const noexcept;

	private:
		bool bad;
		std::string inputfile, outputfile;
		unsigned short b_size;
		bool verbose;
};

class InputException : public std::exception
{
	public:
		InputException(const char * msg = "input is bad, check before using member getters")
			: message(msg)
		{
		}

		const char * what() const noexcept override
		{
			return message.c_str();
		}

	private:
		std::string message;
};

#ifdef DEBUG
std::ostream & operator << (std::ostream & os, const Input & i);
#endif

#endif
