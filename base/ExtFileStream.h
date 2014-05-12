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

#include "Bitmap.h"
#include "FSFileStream.h"
#include "INode.h"

#include <deque>
#include <functional>
#include <vector>

class ExtFileStream : public FSFileStream {
	static const streampos SUPERBLOCK_OFFSET = 1024;
	static const uint32_t MAX_BLOCK_SIZE_OFFSET = 5;
	static const uint16_t EXT_SUPER_MAGIC = 0xEF53;
	static const uint8_t GROUP_DESCRIPTOR_SIZE = 32;
	static const uint8_t INODE_FILE_BLOCKS_INDIRECT = 12;
	static const uint8_t INODE_FILE_BLOCKS_DOUBLY_INDIRECT = 13;
	static const uint8_t INODE_FILE_BLOCKS_TRIPLY_INDIRECT = 14;
	static const uint32_t END_OF_BLOCKCHAIN = 0;
	static const uint8_t REV_0_INODE_TABLE_RESERVED_ENTRIES_COUNT = 11;
	static const uint8_t REV_0_INODE_SIZE = 128;

	enum Revision {
		EXT2_GOOD_OLD_REV = 0,
		EXT2_DYNAMIC_REV = 1,
	};
	
	struct BlockDescriptor {
		uint32_t blocks_bitmap;
		uint32_t inodes_bitmap;
		uint32_t inodes_table;
	};
	
	struct BlockOffsets {
		size_t block_n_abs;
		size_t start_offset_relative_block_n_abs;
		size_t block_group_n;
		size_t block_group_start;
		size_t block_n_group_relative;
	};
	
public:
	struct DeviceInfo {
		uint32_t inodes_count;
		uint32_t blocks_count;
		uint32_t block_size;
		
		uint32_t first_data_block;
		uint32_t blocks_per_group;
		uint32_t inodes_per_group;
		
		uint32_t group_first_data_inode;
		uint16_t inode_size;
		
		Revision revision;

		bool correct;
	};
	
private:
	static const streampos FS_TEST_OFFSET = 32 * 1024; // Second block with 32 kbytes block size
	
	typedef std::function<bool(size_t)> inode_blocks_callback_t;

	std::deque<size_t> blocks;
	
	DeviceInfo device;
	
	bool is_correct;
	size_t offset;

	static bool check_block(const Bitmap &blocks_bitmap, bool used, size_t block_n_group_relative);
	
	void init();
	void init_blocks(streampos absolute_offset);

	template<typename T>
	T superblock_get(streampos offset) const {
		return this->FSFileStream::get<T>(offset + SUPERBLOCK_OFFSET);
	}
	
	BlockDescriptor read_descriptor(size_t block_group_n);
	Bitmap read_group_bitmap(size_t bitmap_start_block_n);
	
	void rebuild_existent(const BlockDescriptor &desc, const Bitmap &blocks_bitmap, const BlockOffsets &offset);
	void rebuild_deleted(const Bitmap &blocks_bitmap, const BlockOffsets &offset);
	
	bool add(const Bitmap &blocks_bitmap, bool used, size_t block_group_start, size_t block_n_group_relative);
	INode find_inode(const ExtFileStream::BlockDescriptor &desc, const ExtFileStream::BlockOffsets &offset);
	INode read_inode(size_t inode_offset);
	
	void inode_foreach(const INode &inode, uint32_t group_start_n, const inode_blocks_callback_t &callback);
	
	bool indirect(uint32_t blocks_group_start, uint32_t offset, const inode_blocks_callback_t &callback);
	bool doubly_indirect(uint32_t blocks_group_start, uint32_t offset, const inode_blocks_callback_t &callback);
	bool triply_indirect(uint32_t blocks_group_start, uint32_t offset, const inode_blocks_callback_t &callback);

protected:

public:
	ExtFileStream(const std::string &device_name, streampos absolute_offset = FS_TEST_OFFSET);

	virtual streampos read(Buffer &buffer, streampos size);

	virtual void seekg(streampos offset);
	virtual streampos tellg() const override;
	virtual bool eof() const override;

	const DeviceInfo &info() const;

	virtual bool correct() const override;
};
