#ifndef __BUFFERED_READDDDER__
#define __BUFFERED_READDDDER__
#include <memory>
#include <string>
#include <future>
#include <fstream>
#include "thread_pool.h"

struct buff
{
	buff(std::size_t size, std::size_t b_size)
		: data(new char[size]), block_size(b_size),
		size(size), blocks_in_buffer(size / block_size),
		hit_count(0)
	{
		if (blocks_in_buffer < 2)
			throw std::invalid_argument("pointless to create buffered reader");
	}

	~buff(){delete [] data;}
	void swap(buff & other)
	{
		std::swap(this->data, other.data);
		this->start_id = other.start_id;
		other.start_id += blocks_in_buffer;
		this->hit_count = other.hit_count;
		this->future = std::move(other.future);
	}

	std::size_t last_id() const
	{
		return start_id + blocks_in_buffer - 1;
	}

	const char * operator[](std::size_t idx)
	{
		if (idx < start_id || idx > last_id())
			throw std::invalid_argument("out of range");
		
		return data + (idx - start_id) * block_size;
	}

	char * data;
	const std::size_t block_size, size, blocks_in_buffer;
	std::size_t start_id, hit_count;
	std::future<void> future;
};

class buffered_reader
{
	buff b1, b2;
	const std::string file_name;
	std::size_t file_size;
	thread_pool<std::ifstream> pool_reader;

	void request_read(buff & b);
	public:
		buffered_reader(
			const std::string & file_name,
			std::size_t buffer_size,
			std::size_t block_size,
			std::size_t threads
		);
		std::unique_ptr<char[]> operator[](std::size_t idx);
};

#endif
