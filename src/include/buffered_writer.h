#ifndef __BUFFERED_WRITER__
#define __BUFFERED_WRITER__
#include <fstream>
#include <memory>
#include <string>

class buffered_writer
{
	std::size_t buff_s, block_s, position;
	std::unique_ptr<char[]> buffer;
	std::ofstream file;

	public:
		buffered_writer(const std::string & file_name, std::size_t block_size, std::size_t buffer_size);
		buffered_writer(const buffered_writer & other) = delete;
		buffered_writer(buffered_writer && rvalue) = default;
		// destructor will write last block if tere is such
		~buffered_writer();
		buffered_writer & operator << (const char * block);
};

#endif
