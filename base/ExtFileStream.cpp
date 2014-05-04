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

        MAGIC = 56,
        STATE = 58,
        ERROR = 60,
};

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

	DEVICE_CHECK(EXT_SUPER_MAGIC == this->get<uint16_t>(MAGIC));
	DEVICE_CHECK(VALID_STATES.count(this->get<uint16_t>(STATE)));
	DEVICE_CHECK(VALID_ERRORS.count(this->get<uint16_t>(ERROR)));

	this->device.inodes_count = this->get<uint32_t>(INODES_COUNT);
	this->device.blocks_count = this->get<uint32_t>(BLOCKS_COUNT);

	DEVICE_CHECK(this->device.inodes_count > 1 and this->device.blocks_count > 1);

	auto blocks_offset = this->get<uint32_t>(BLOCK_SIZE);
	DEVICE_CHECK(blocks_offset <= MAX_BLOCK_SIZE_OFFSET);
	this->device.block_size = 1024 << blocks_offset;

	this->device.first_data_block = this->get<uint32_t>(FIRST_DATA_BLOCK);
	this->device.blocks_per_group = this->get<uint32_t>(BLOCKS_PER_GROUP);
	
	DEVICE_CHECK(this->device.first_data_block != SUPERBLOCK_OFFSET / this->device.block_size);
}

void ExtFileStream::init_blocks(streampos absolute_offset) {
	size_t block_n = absolute_offset / this->device.block_size;
	size_t relative_block_n = block_n - this->device.first_data_block;
	size_t block_group_n = relative_block_n / this->device.blocks_per_group;
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
