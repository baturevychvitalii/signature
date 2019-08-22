#ifndef __CUSTOM_EXCEPTIONS__
#define __CUSTOM_EXCEPTIONS__

#include <exception>

class input_exception : public std::exception
{
	public:
		input_exception(const char * msg = "input is bad, check before using member getters")
			: message(msg)
		{
		}

		const char * what() const noexcept override
		{
			return message;
		}

	private:
		const char * message;
};

class paralel_exception : public std::exception
{
	public:
		paralel_exception(const char * msg = "invalid thread behavior")
			: message(msg)
		{
		}

		const char * what() const noexcept override
		{
			return message;
		}

	private:
		const char * message;
};

class file_exception : public std::exception
{
	public:
		file_exception(const char * msg = "file exception")
			: message(msg)
		{
		}

		const char * what() const noexcept override
		{
			return message;
		}

	private:
		const char * message;
};

#endif
