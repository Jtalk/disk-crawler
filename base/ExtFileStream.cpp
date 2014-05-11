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

#include "ExtFileStream.h"

#include "utility.h"

#include <functional>
#include <unordered_set>

#include <cstdio>

#define DEVICE_CHECK(expr) \
	if (not (expr)) { \
		this->device.correct = false; \
		return; \
	} (void)0

enum ExtOffsets : size_t {
        INODES_COUNT = 0x00,
        BLOCKS_COUNT = 0x04,
        FIRST_DATA_BLOCK = 0x14,
        BLOCK_SIZE = 0x18,
	BLOCKS_PER_GROUP = 0x20,
	INODES_PER_GROUP = 0x28,

        MAGIC = 56,
        STATE = 58,
        ERROR = 60,
	
	REVISION = 76,
	
	FIRST_INODE = 84,
};

enum GroupDescriptorOffsets : size_t {
	BLOCKS_BITMAP = 0,
	INODE_BITMAP = 4,
	INODE_TABLE = 8,
};

enum INodeOffsets : size_t {
	TYPE = 0,
	FILE_SIZE = 4,
	BLOCKS = 40,
	DIR_ACL = 108,
};

static const uint16_t REGULAR_FILE_TYPE = 0x8000;

enum ExtState : uint16_t {
        EXT2_VALID_FS = 1, // Unmounted cleanly
        EXT2_ERROR_FS = 2, // Errors detected
};

static const std::unordered_set<uint16_t> VALID_STATES = {
	EXT2_VALID_FS,
	EXT2_ERROR_FS,
};

enum ExtErrors : uint16_t {
        EXT2_ERRORS_CONTINUE = 1, // continue as if nothing happened
        EXT2_ERRORS_RO = 2, // remount read-only
        EXT2_ERRORS_PANIC = 3, // cause a kernel panic
};

static const std::unordered_set<uint16_t> VALID_ERRORS = {
	EXT2_ERRORS_CONTINUE,
	EXT2_ERRORS_RO,
	EXT2_ERRORS_PANIC,
};

static const std::unordered_set<uint32_t> VALID_REVISIONS = {
	ExtFileStream::EXT2_GOOD_OLD_REV,
	ExtFileStream::EXT2_DYNAMIC_REV,
};

ExtFileStream::ExtFileStream(const std::string &device_name, streampos absolute_offset)
	: FSFileStream(device_name), is_correct(true) {
	this->init();

	if (not this->device.correct) {
		return;
	}

	this->init_blocks(absolute_offset);

	if (not this->correct()) {
		return;
	}
}

void ExtFileStream::init() {
	this->device.correct = true;

	DEVICE_CHECK(EXT_SUPER_MAGIC == this->superblock_get<uint16_t>(MAGIC));
	DEVICE_CHECK(VALID_STATES.count(this->superblock_get<uint16_t>(STATE)));
	DEVICE_CHECK(VALID_ERRORS.count(this->superblock_get<uint16_t>(ERROR)));

	this->device.revision = (Revision)this->superblock_get<uint32_t>(REVISION);
	DEVICE_CHECK(VALID_REVISIONS.count(this->device.revision));
	
	this->device.inodes_count = this->superblock_get<uint32_t>(INODES_COUNT);
	this->device.blocks_count = this->superblock_get<uint32_t>(BLOCKS_COUNT);

	DEVICE_CHECK(this->device.inodes_count > 1 and this->device.blocks_count > 1);

	auto blocks_offset = this->superblock_get<uint32_t>(BLOCK_SIZE);
	DEVICE_CHECK(blocks_offset <= MAX_BLOCK_SIZE_OFFSET);
	this->device.block_size = 1024 << blocks_offset;

	this->device.first_data_block = this->superblock_get<uint32_t>(FIRST_DATA_BLOCK);
	this->device.blocks_per_group = this->superblock_get<uint32_t>(BLOCKS_PER_GROUP);
	this->device.inodes_per_group = this->superblock_get<uint32_t>(INODES_PER_GROUP);
	
	DEVICE_CHECK(this->device.first_data_block != SUPERBLOCK_OFFSET / this->device.block_size);
	
	if (this->device.revision == EXT2_GOOD_OLD_REV) {
		this->device.group_first_data_inode = REV_0_INODE_TABLE_RESERVED_ENTRIES_COUNT;
	} else {
		this->device.group_first_data_inode = this->superblock_get<uint32_t>(FIRST_INODE);
	}
}

void ExtFileStream::init_blocks(streampos absolute_offset) {
	BlockOffsets offsets;
	
	offsets.block_n_abs = absolute_offset / this->device.block_size;
	offsets.start_offset_relative_block_n_abs = offsets.block_n_abs - this->device.first_data_block;
	offsets.block_group_n = offsets.start_offset_relative_block_n_abs / this->device.blocks_per_group;
	offsets.block_group_start = offsets.block_group_n * this->device.blocks_per_group;
	offsets.block_n_group_relative = offsets.block_n_abs - offsets.block_group_start;
	
	BlockDescriptor desc = this->read_descriptor(offsets.block_group_n);
	
	Bitmap blocks_bitmap = this->read_group_bitmap(desc.blocks_bitmap);
	
	if (blocks_bitmap[offsets.block_n_group_relative]) {
		this->rebuild_existent(desc, blocks_bitmap, offsets);
	} else {
		this->rebuild_deleted(desc, blocks_bitmap, offsets);
	}
}

ExtFileStream::BlockDescriptor ExtFileStream::read_descriptor(size_t block_group_n) {
	size_t groups_descriptor_table_start = this->device.block_size * (this->device.first_data_block + 1);
	size_t group_descriptor_start = groups_descriptor_table_start + block_group_n * GROUP_DESCRIPTOR_SIZE;
	
	BlockDescriptor desc;
	desc.blocks_bitmap = this->get<uint32_t>(group_descriptor_start + BLOCKS_BITMAP);
	desc.inodes_bitmap = this->get<uint32_t>(group_descriptor_start + INODE_BITMAP);
	desc.inodes_table = this->get<uint32_t>(group_descriptor_start + INODE_TABLE);
	
	return desc;
}

ExtFileStream::Bitmap ExtFileStream::read_group_bitmap(size_t bitmap_start_block_n) {
	Buffer buffer(this->device.blocks_per_group);
	fseek(this->stream, bitmap_start_block_n * this->device.block_size, SEEK_SET);
	auto read = fread(buffer.begin(), 1, this->device.blocks_per_group, this->stream);
	buffer.shrink(read);
	
	if (buffer.empty()) {
		this->is_correct = false;
		return Bitmap();
	}
	
	Bitmap result;
	result.reserve(read);
	for (uint8_t item : buffer) {
		result.push_back(item);
	}
	
	return std::move(result);
}

void ExtFileStream::rebuild_deleted(const ExtFileStream::BlockDescriptor &desc, const ExtFileStream::Bitmap &blocks_bitmap, const ExtFileStream::BlockOffsets &offset) {
	size_t start = offset.block_n_group_relative;
	while (this->add(blocks_bitmap, false, offset.block_group_start, start++))
	{}
}

void ExtFileStream::rebuild_existent(const ExtFileStream::BlockDescriptor &desc, const ExtFileStream::Bitmap &blocks_bitmap, const ExtFileStream::BlockOffsets &offset) {
	// TODO: Inodes cache powered by Bloom filters
	INode &&inode = this->find_inode(desc, offset);
	this->inode_foreach(inode, offset.block_group_start, [this, &blocks_bitmap, &offset] (size_t block_n_group_relative) {
		return this->add(blocks_bitmap, true, offset.block_group_start, block_n_group_relative);
	});
}

bool ExtFileStream::check_block(const ExtFileStream::Bitmap &blocks_bitmap, bool used, size_t block_n_group_relative) {
	bool fits_bitmap = blocks_bitmap.size() > block_n_group_relative;
	return fits_bitmap and blocks_bitmap[block_n_group_relative] == used;
}

bool ExtFileStream::add(const Bitmap &blocks_bitmap, bool used, size_t block_group_start, size_t block_n_group_relative) {
	bool checked = ExtFileStream::check_block(blocks_bitmap, used, block_group_start, block_n_group_relative);
	if (checked) {
		this->blocks.push_back(block_group_start + block_n_group_relative);
	}
	return checked;
}

INode ExtFileStream::find_inode(const ExtFileStream::BlockDescriptor &desc, const ExtFileStream::BlockOffsets &offset) {
	size_t inodes_table_start_abs = desc.inodes_table + offset.block_group_start;
	for (size_t i = this->device.group_first_data_inode; i < this->device.inodes_per_group; i++) {
		INode &&inode = this->read_inode(inodes_table_start_abs + (i * INODE_SIZE));
		bool found = false;
		this->inode_foreach(inode, offset.block_group_start, [&offset, &found] (size_t block_n_group_relative) {
			found = block_n_group_relative == offset.block_n_group_relative;
			return not found;
		});
		if (found) {
			return std::move(inode);
		}
	}
	return INode(INode::ALLOC_FULL);
}

INode ExtFileStream::read_inode(size_t inode_offset) {
	INode inode(INode::ALLOC_FULL);
	bool regular = (this->get<uint16_t>(inode_offset + TYPE) & REGULAR_FILE_TYPE);
	if (not regular) {
		return std::move(inode);
	}
	inode.file_size = this->get<uint32_t>(inode_offset + FILE_SIZE);
	if (this->device.revision != EXT2_GOOD_OLD_REV) {
		uint64_t high_size_bytes = this->get<uint32_t>(inode_offset + DIR_ACL);
		inode.file_size |= (high_size_bytes << 32);
	}
	for (size_t i = 0; i < INode::FILE_BLOCKS_MAX; i++) {
		inode.blocks[i] = this->get<uin32_t>(inode_offset + BLOCKS + i);
	}
	return std::move(inode);
}

void ExtFileStream::inode_foreach(const INode &inode, uint32_t group_start_abs, const inode_blocks_callback_t &callback) {
	for (uint8_t i = 0; i < INODE_FILE_BLOCKS_INDIRECT; i++) {
		uint32_t offset = inode.blocks[i];
		if (offset == END_OF_BLOCKCHAIN or not callback(offset)) {
			return;
		}
	}
	
	static const std::pair<uint8_t, std::function<uint32_t, uint32_t, const inode_blocks_callback_t&>> HANDLERS[] = {
		{INODE_FILE_BLOCKS_INDIRECT, &ExtFileStream::indirect},
		{INODE_FILE_BLOCKS_DOUBLY_INDIRECT, &ExtFileStream::doubly_indirect},
		{INODE_FILE_BLOCKS_TRIPLY_INDIRECT, &ExtFileStream::triply_indirect},		
	};
	
	for (auto &handler : HANDLERS) {
		uint32_t offset = inode.blocks[handler.first];
		if (offset == END_OF_BLOCKCHAIN or not handler.second(group_start_abs, offset, callback)) {
			return;
		}
	}
}

bool ExtFileStream::indirect(uint32_t blocks_group_start, uint32_t offset, const inode_blocks_callback_t &callback) {
	size_t blocks_end = offset + this->device.block_size;
	for (uint32_t block = offset; block < blocks_end; block++) {
		auto current_pointer = this->get<uint32_t>(blocks_group_start + block * sizeof(uint32_t));
		if (block == END_OF_BLOCKCHAIN or not callback(current_pointer)) {
			return false;
		}
	}
	return true;
}

bool ExtFileStream::doubly_indirect(uint32_t blocks_group_start, uint32_t offset, const inode_blocks_callback_t &callback) {
	size_t blocks_end = offset + this->device.block_size;
	for (uint32_t block = offset; block < blocks_end; block++) {
		auto current_pointer = this->get<uint32_t>(blocks_group_start + block * sizeof(uint32_t));
		if (block == END_OF_BLOCKCHAIN or not this->indirect(blocks_group_start, current_pointer, callback)) {
			return false;
		}
	}
	return true;
}

bool ExtFileStream::triply_indirect(uint32_t blocks_group_start, uint32_t offset, const inode_blocks_callback_t &callback) {
	size_t blocks_end = offset + this->device.block_size;
	for (uint32_t block = offset; block < blocks_end; block++) {
		auto current_pointer = this->get<uint32_t>(blocks_group_start + block * sizeof(uint32_t));
		if (block == END_OF_BLOCKCHAIN or not this->doubly_indirect(blocks_group_start, current_pointer, callback)) {
			return false;
		}
	}
	return true;
}

FSFileStream::streampos ExtFileStream::read(Buffer &buffer, streampos size) {

}

bool ExtFileStream::eof() const {
	return feof(this->stream) or ferror(this->stream) or not this->correct();
}

streampos ExtFileStream::tellg() const {

}

void ExtFileStream::seekg(streampos offset) {

}

const ExtFileStream::DeviceInfo &ExtFileStream::info() const {
	return this->device;
}

bool ExtFileStream::correct() const {
	return this->device.correct and this->is_correct;
}
