/*
    Disk Crawler Library.
    Copyright (C) 2013  Jtalk <me@jtalk.me>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "FSFileStream.h"

class ExtFileStream : public FSFileStream {
	static const streampos SUPERBLOCK_OFFSET = 1024;
	static const uint32_t MAX_BLOCK_SIZE_OFFSET = 5;
	static const uint16_t EXT_SUPER_MAGIC = 0xEF53;

	struct DeviceInfo {
		uint32_t inodes_count;
		uint32_t blocks_count;
		uint32_t block_size;
		
		uint32_t first_data_block;
		uint32_t blocks_per_group;

		bool correct;
	};

	DeviceInfo device;
	
	bool is_correct;

	void init();
	void init_blocks(streampos absolute_offset);

	template<typename T>
	T get(streampos offset) const {
		return this->FSFileStream::get<T>(offset + SUPERBLOCK_OFFSET);
	}

protected:

public:
	ExtFileStream(const std::string &device_name, streampos absolute_offset);

	virtual streampos read(Buffer &buffer, streampos size);

	virtual void seekg(streampos offset);
	virtual streampos tellg() const override;
	virtual bool eof() const override;

	const DeviceInfo &info() const;

	virtual bool correct() const override;
};
