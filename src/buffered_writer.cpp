#include <cstring>
#include "include/buffered_writer.h"
#include "include/custom_exceptions.h"

buffered_writer::buffered_writer(
	const std::string & file_name,
	std::size_t block_size,
	std::size_t buffer_size
)
	: buff_s(buffer_size), block_s(block_size), position(0),
	buffer(std::make_unique<char[]>(buff_s)),
	file(file_name, std::ios_base::binary | std::ios_base::trunc)
{
	if (block_s > buff_s)
		throw std::invalid_argument("block size must be smaller than buffer size");
	if (!file.is_open())
		throw file_exception("couldn't open file for writing");
}

buffered_writer::~buffered_writer()
{
	if (position > 0)
		file.write(buffer.get(), position);
}

buffered_writer & buffered_writer::operator<<(const char * block)
{
	if (position + block_s > buff_s)
	{
		file.write(buffer.get(), position);
		position = 0;
	}

	std::memcpy(buffer.get() + position, block, block_s);
	position += block_s;
	return *this;
}
