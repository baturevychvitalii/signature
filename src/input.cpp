#include <boost/program_options.hpp>

#include "include/custom_exceptions.h"
#include "include/input.h"

input::input(int argc, char * argv[], std::ostream & error_stream)
	: bad(true)
{
	using namespace boost::program_options;

	options_description desc("Usage");
	desc.add_options()
		("help,h","display this message")
		("input_file,i", value<std::string>()->required(), "file, signature of which will be generated")
		("output_file,o", value<std::string>()->required(), "file, where the signature will be stored")
		("block_size,b", value<std::size_t>()->default_value(1024), "block size KB")
		("verbose,v", value<bool>()->default_value(false)->implicit_value(true), "verbosity");

	variables_map var_map;

	try
	{
		store(parse_command_line(argc, argv, desc), var_map);
		notify(var_map);
		bad = false;
	}
	catch(const std::exception& e)
	{
		error_stream << e.what() << "\n\n" << desc << std::endl;
	}
	
	if (!bad)
	{
		inputfile = std::move(var_map["input_file"].as<std::string>());
		outputfile = std::move(var_map["output_file"].as<std::string>());
		b_size = std::move(var_map["block_size"].as<std::size_t>());
		verbose = std::move(var_map["verbose"].as<bool>());
	}
}

bool input::is_bad() const noexcept
{
	return bad;
}

void input::check_bad() const
{
	if (is_bad())
	{
		throw input_exception();
	}
}

std::string input::get_input_file() const
{
	check_bad();
	return inputfile;
}

std::string input::get_output_file() const
{
	check_bad();
	return outputfile;
}

bool input::is_verbose() const
{
	check_bad();
	return verbose;
}

std::ostream & operator << (std::ostream & os, const input & i)
{
	if (i.is_bad())
	{
		os << "bad input" << std::endl;
	}
	else
	{
		os << "input file = "<< i.get_input_file()
			<< "\noutput file = " << i.get_output_file()
			<< "\nblock size = " << i.get_block_size() << " (kilobytes)"
			<< "\nverbose = " << std::ios_base::boolalpha << i.is_verbose() << std::endl;
	}

	return os;
}
