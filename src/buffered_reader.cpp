#include "include/buffered_reader.h"
#include "include/custom_exceptions.h"
#include <iostream>
#include <cstring>

buffered_reader::buff::buff(std::size_t buff_size, std::size_t b_size)
	: block_size(b_size), blocks_in_buffer(buff_size / block_size),
	size(block_size * blocks_in_buffer),
	data(new char[size]), 
	hit_count(0)
{
	if (blocks_in_buffer < 2)
		throw std::invalid_argument("pointless to create buffered reader");
}

buffered_reader::buff::~buff(){delete [] data;}

void buffered_reader::buff::swap(buff & other)
{
	std::swap(this->data, other.data);
	this->start_id = other.start_id;
	other.start_id += blocks_in_buffer;
	this->hit_count = other.hit_count;
	this->future = std::move(other.future);
}

std::size_t buffered_reader::buff::last_id() const
{
	return start_id + blocks_in_buffer - 1;
}

const char * buffered_reader::buff::operator[](std::size_t idx) const
{
	if (idx < start_id || idx > last_id())
		throw std::invalid_argument("out of range");
	
	return data + (idx - start_id) * block_size;
}

buffered_reader::buffered_reader(
	const std::string & file_name,
	std::size_t buffer_size,
	std::size_t block_size,
	std::size_t threads,
	bool verbose
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

	if (verbose)
		std::cout << "initializing " << threads << " threads for buffered reader\n"
		<< "input file size " << file_size << " bytes\nbuffered reader can satisfy "
		<< b1.blocks_in_buffer * 2 << " requests at a time" << std::endl;

	if (file_size == 0)
		throw std::invalid_argument("empty file provided");
	
	for (std::size_t i = 0; i < threads; i++)
		pool_reader.add_thread(std::ifstream(file_name, std::ios_base::binary));

	b1.start_id = 0;
	b2.start_id = b2.blocks_in_buffer;

	#ifdef DEBUG
	std::cout << "reading into b1: "
	<< b1.block_size * b1.blocks_in_buffer << " B from " << b1.start_id * b1.block_size << std::endl;
	#endif
	request_read(b1); 

	#ifdef DEBUG
	std::cout << "reading into b2: "
	<< b2.block_size * b2.blocks_in_buffer << " B from " << b2.start_id * b2.block_size << std::endl;
	#endif
	request_read(b2);
}


void buffered_reader::copy(std::size_t idx, char *const dest, std::size_t size)
{
	// operation must be atomic
	std::scoped_lock l(request_guard);

	if (idx < b1.start_id)
		throw std::invalid_argument("this idx must have been already queried");
		
	if (idx < b2.start_id)
	{
		b1.future.wait();
		std::memcpy(dest, b1[idx], size);
		b1.hit_count++;

		if (b1.hit_count == b1.blocks_in_buffer)
		{
			#ifdef DEBUG
			std::cout << "swapping b2 into b1: now b1 start_id is " << b1.start_id << std::endl; 
			#endif

			b1.swap(b2);

			#ifdef DEBUG
			std::cout << "reading into b2: "
			<< b2.block_size * b2.blocks_in_buffer << " B from " << b2.start_id * b2.block_size << std::endl;
			#endif
			request_read(b2);
		}
	}
	else
	{
		if (b2.hit_count == b2.blocks_in_buffer)
		{
			b2.start_id += b2.blocks_in_buffer;
			#ifdef DEBUG
			std::cout << "reading into b2: "
			<< b2.block_size * b2.blocks_in_buffer << " B from " << b2.start_id * b2.block_size << std::endl;
			#endif
			request_read(b2);
		}

		b2.future.wait();
		std::memcpy(dest, b2[idx], size);
		b2.hit_count++;
	}
}

void buffered_reader::request_read(buff & b)
{
	b.future = pool_reader.push_task(
		[&b, this](std::ifstream & ifs)
		{
			b.hit_count = 0;
			if (ifs.eof()) return;
			ifs.seekg(b.start_id * b.block_size);
			ifs.read(b.data, b.block_size * b.blocks_in_buffer);
		}
	);
}
