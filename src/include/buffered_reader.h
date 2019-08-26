#ifndef __BUFFERED_READDDDER__
#define __BUFFERED_READDDDER__
#include <memory>
#include <string>
#include <future>
#include <fstream>
#include <mutex>
#include "thread_pool.h"


class buffered_reader
{
	struct buff
	{
		buff(std::size_t buff_size, std::size_t bl_size);
		buff(const buff & l) = delete;
		buff(buff && r) = delete;
		~buff();
		void swap(buff & other);
		std::size_t last_id() const;
		const char * operator[](std::size_t idx) const;

		const std::size_t block_size, blocks_in_buffer, size;
		char * data;
		std::size_t start_id, hit_count;
		std::future<void> future;
	};

	buff b1, b2;
	const std::string file_name;
	std::size_t file_size;
	thread_pool<std::ifstream> pool_reader;
	std::mutex request_guard;

	void request_read(buff & b);
	public:
		buffered_reader(
			const std::string & file_name,
			std::size_t buffer_size,
			std::size_t block_size,
			std::size_t threads,
			bool verbose
		);

		~buffered_reader(){pool_reader.finish();}

		inline std::size_t get_file_size() const {return file_size;}
		void copy(std::size_t idx, char *const dest, std::size_t size);
};

#endif
