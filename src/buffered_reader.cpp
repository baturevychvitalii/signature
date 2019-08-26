#include "include/buffered_reader.h"
#include "include/custom_exceptions.h"
#include <cstring>

buffered_reader::buffered_reader(
	const std::string & file_name,
	std::size_t buffer_size,
	std::size_t block_size,
	std::size_t threads
)
	: b1(buffer_size/2, block_size),
	b2(buffer_size/2, block_size),
	file_name(file_name)
{
	if (threads < 1)
		throw std::invalid_argument("must be at least 1 thread");

	std::ifstream file_test(file_name, std::ios_base::binary | std::ios_base::ate);
	if (!file_test.is_open())
		throw file_exception("couldn't open file for reading");

	file_size = file_test.tellg();

	if (file_size == 0)
		throw std::invalid_argument("empty file provided");
	
	for (std::size_t i = 0; i < threads; i++);
		pool_reader.add_thread(std::ifstream(file_name, std::ios_base::binary));

	b1.start_id = 0;
	b2.start_id = b2.blocks_in_buffer;

	request_read(b1);
	request_read(b2);
}


std::unique_ptr<char[]> buffered_reader::operator[](std::size_t idx)
{
	if (idx < b1.start_id || idx > b2.last_id())
		throw std::invalid_argument("you wish to fast");
		
	auto result = std::make_unique<char[]>(b1.block_size);
	if (idx < b2.start_id)
	{
		b1.future.wait();
		std::memcpy(result.get(), b1[idx], b1.block_size);
		b1.hit_count++;

		if (b1.hit_count == b1.blocks_in_buffer)
		{
			b1.swap(b2);
			request_read(b2);
		}
	}
	else
	{
		b2.future.wait();
		std::memcpy(result.get(), b2[idx], b2.block_size);
		b2.hit_count++;

		if (b2.hit_count == b2.blocks_in_buffer)
		{
			b2.start_id += b2.blocks_in_buffer;
			request_read(b2);
		}
	}

	return result;
}

void buffered_reader::request_read(buff & b)
{
	b.future = pool_reader.push_task(
		[&b](std::ifstream & ifs)
		{
			b.hit_count = 0;
			ifs.seekg(b.start_id * b.block_size);
			ifs.read(b.data, b.block_size * b.blocks_in_buffer);
		}
	);
}
