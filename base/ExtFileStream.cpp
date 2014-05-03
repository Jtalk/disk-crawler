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

enum ExtOffsets : size_t {
	INODES_COUNT = 0x00,
	BLOCKS_COUNT = 0x04,
	BLOCK_SIZE = 0x18,
	
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

ExtFileStream::ExtFileStream(const std::string &device_name)
	: FSFileStream(device_name) {
	this->init();
}

void ExtFileStream::init() {
	this->device.correct = true;
	
	auto magic = this->get<uint16_t>(MAGIC);
	if (magic != EXT_SUPER_MAGIC) {
		this->device.correct = false;
		return;
	}
	
	auto state = this->get<uint16_t>(STATE);
	if (not VALID_STATES.count(state)) {
		this->device.correct = false;
		return;
	}
	
	auto error = this->get<uint16_t>(ERROR);
	if (not VALID_ERRORS.count(error)) {
		this->device.correct = false;
		return;
	}
	
	this->device.inodes_count = this->get<uint32_t>(INODES_COUNT);
	this->device.blocks_count = this->get<uint32_t>(BLOCKS_COUNT);
	
	if (this->device.inodes_count <= 1 or this->device.blocks_count <= 1) {
		this->device.correct = false;
		return;
	}
	
	auto blocks_offset = this->get<uint32_t>(BLOCK_SIZE);
	if (blocks_offset > MAX_BLOCK_SIZE_OFFSET) {
		this->device.correct = false;
		return;
	}
	
	this->device.block_size = 1024 << blocks_offset;
}

FSFileStream::streampos ExtFileStream::read(Buffer &buffer, streampos size) {

}

bool ExtFileStream::eof() const {

}

streampos ExtFileStream::tellg() const {

}

void ExtFileStream::seekg(streampos offset) {

}

const ExtFileStream::DeviceInfo& ExtFileStream::info() const {
	return this->device;
}

bool ExtFileStream::correct() const {

}
