#include <boost/program_options.hpp>

#include "include/input.h"

Input::Input(int argc, char * argv[], std::ostream & error_stream)
	: bad(true)
{
	namespace po = boost::program_options;

	po::options_description desc("Usage");
	desc.add_options()
		("help,h","display this message")
		("input_file,i", po::value<std::string>()->required(), "file, signature of which will be generated")
		("output_file,o", po::value<std::string>()->required(), "file, where the signature will be stored")
		("block_size,b", po::value<unsigned short>()->default_value(1024), "block size KB")
		("verbose,v", po::value<bool>()->default_value(false)->implicit_value(true), "verbosity");

	po::variables_map var_map;

	try
	{
		po::store(po::parse_command_line(argc, argv, desc), var_map);
		po::notify(var_map);
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
		b_size = std::move(var_map["block_size"].as<unsigned short>());
		verbose = std::move(var_map["verbose"].as<bool>());
	}
}

bool Input::is_bad() const noexcept
{
	return bad;
}

std::string Input::in() const
{
	if (is_bad())
	{
		throw InputException();
	}

	return inputfile;
}

std::string Input::out() const
{
	if (is_bad())
	{
		throw InputException();
	}

	return outputfile;
}

unsigned short Input::block_size() const
{
	if (is_bad())
	{
		throw InputException();
	}

	return b_size;
}

bool Input::is_verbose() const
{
	if (is_bad())
	{
		throw InputException();
	}

	return verbose;
}

std::ostream & operator << (std::ostream & os, const Input & i)
{
	if (i.is_bad())
	{
		os << "bad input" << std::endl;
	}
	else
	{
		os << "input file = "<< i.in() 
			<< "\noutput file = " << i.out()
			<< "\nblock size = " << i.block_size()
			<< "\nvervose = " << std::ios_base::boolalpha << i.is_verbose() << std::endl;
	}

	return os;
}
