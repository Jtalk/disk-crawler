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


#include "FATFileStream.h"

#include <algorithm>

#include <cmath>
#include <cstdlib>

enum FATOffsets : size_t {
	SECTOR_SIZE = 0x0B,
	CLUSTER_SIZE = 0x0D,
	RESERVED_SECTORS = 0x0E,
	NUMBER_OF_FATS = 0x10,
	NUMBER_OF_ROOT_ENTRIES = 0x11,
	LOGICAL_SECTORS_COUNT = 0x13,
	SECTORS_PER_FAT = 0x16,
	LOGICAL_SECTORS_32 = 0x20,
	SECTORS_PER_FAT32 = 0x24
};

enum FATDirectoryOffsets : size_t {
	SIZE_OFFSET = 0x1C
};

FATFileStream::FATFileStream(FILE *stream, streampos absolute_offset)
	: FSFileStream(stream), is_correct(true), is_eof(false)
{
	this->init();

	if (!this->correct())
		return;

	this->is_correct = (absolute_offset >= this->device.data_offset) && !ferror(this->stream);

	if (!this->correct())
		return;

	this->init_clusters(absolute_offset);
	this->seekg(0);
}

FATFileStream::~FATFileStream()
{}

size_t FATFileStream::fat_type() const
{
	auto count = this->device.data_clusters_count;

	if (count > 4085 and count <= 65524)
		return 16;

	if (count > 65524)
		return 32;

	abort();
}

size_t FATFileStream::eoc() const
{
	switch (this->device.fat_entry_size) {
	case 16:
		return 0xFFF8;

	case 32:
		return 0xFFFFFF8;

	default:
		abort();
	}
}

void FATFileStream::init()
{
	auto sector_size = this->get<uint16_t>(SECTOR_SIZE);
	this->device.cluster_size = sector_size * this->get<uint8_t>(CLUSTER_SIZE);

	this->device.fat_offset = sector_size * this->get<uint16_t>(RESERVED_SECTORS);

	auto number_of_fats = this->get<uint8_t>(NUMBER_OF_FATS);
	auto number_of_root_entries = this->get<uint16_t>(NUMBER_OF_ROOT_ENTRIES);

	this->device.fat_size = this->get<uint16_t>(SECTORS_PER_FAT) * sector_size;

	if (this->device.fat_size == 0)
		this->device.fat_size = this->get<uint32_t>(SECTORS_PER_FAT32) * sector_size;

	streampos fats_end = this->device.fat_offset + streampos(number_of_fats * this->device.fat_size);

	size_t root_directory_entries_size = number_of_root_entries * 32 / sector_size;

	this->device.data_offset = fats_end + streampos(root_directory_entries_size);

	this->device.total_sectors = this->get<uint16_t>(LOGICAL_SECTORS_COUNT);

	if (this->device.total_sectors == 0)
		this->device.total_sectors = this->get<uint32_t>(LOGICAL_SECTORS_32);

	this->device.size = this->device.total_sectors * sector_size;
	this->device.data_clusters_count = (this->device.total_sectors - this->device.data_offset / sector_size) * sector_size / this->device.cluster_size;

	this->device.fat_entry_size = this->fat_type();
	this->device.eoc = this->eoc();

	this->is_correct = !ferror(this->stream);
}

void FATFileStream::init_clusters(streampos file_offset)
{
	int64_t from_data_offset = file_offset - this->device.data_offset;
	
	streampos file_cluster_num = from_data_offset / this->device.cluster_size;

	streampos first_cluster_offset = this->device.cluster_size * file_cluster_num + this->device.data_offset;
	
	this->current_pos = first_cluster_offset;
	this->clusters.push_back(first_cluster_offset);
	streampos next;
	
	do {
		auto last_found = this->clusters.back();
		next = this->find_next_cluster(last_found);
	} while (next != npos && (this->clusters.push_back(next), true));
	
	this->file_size = this->clusters.size() * this->device.cluster_size;
}

size_t FATFileStream::read_fat(streampos fat_entry_offset)
{
	switch (this->device.fat_entry_size) {
	case 16:
		return this->get<uint16_t>(fat_entry_offset);

	case 32:
		return this->get<uint32_t>(fat_entry_offset);

	default:
		abort();
	}
}

FATFileStream::streampos FATFileStream::find_next_cluster(streampos source_cluster)
{
	size_t number = (source_cluster - this->device.data_offset) / this->device.cluster_size;

	streampos fat_entry_offset = this->device.fat_offset + streampos(this->device.fat_entry_size * number);

	auto entry = this->read_fat(fat_entry_offset);

	if (entry == 0)
		return source_cluster + this->device.cluster_size;
	else if (entry >= this->device.eoc)
		return npos;
	else
		return this->device.data_offset + streampos(this->device.cluster_size * entry);
}

void FATFileStream::update_cluster(streampos file_pos)
{
	this->is_eof = file_pos >= this->file_size;
	if (this->eof())
		return;
	
	streampos cluster_num = file_pos / this->device.cluster_size;
	streampos cluster_offset = file_pos % this->device.cluster_size;

	this->is_eof = cluster_num >= this->clusters.size();
	if (this->eof())
		return;
	
	streampos offset = this->clusters[cluster_num] + cluster_offset;

	fseek(this->stream, offset, SEEK_SET);
}

FATFileStream::streampos FATFileStream::read(uint8_t *buffer, streampos size)
{
	streampos total_read = 0;

	while (total_read < size) {
		streampos cluster_offset = (this->device.data_offset - this->current_pos) % this->device.cluster_size;
		auto rest = std::min(size, streampos(this->device.cluster_size - cluster_offset));
		
		streampos read = fread(buffer, 1, rest, this->stream);

		total_read += read;

		buffer += read;
		this->current_pos += read;

		this->update_cluster(this->tellg());
		
		if (this->eof())
			break;
	}

	return total_read;
}

FATFileStream::streampos FATFileStream::tellg() const
{
	return this->current_pos;
}

bool FATFileStream::eof() const
{
	return this->is_eof || feof(this->stream);
}

void FATFileStream::seekg(streampos offset)
{
	this->current_pos = std::min(offset, this->file_size);
	this->update_cluster(this->current_pos);
}

bool FATFileStream::correct() const
{
	return this->is_correct;
}
