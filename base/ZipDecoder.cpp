/*
    Disk Crawler library.
    Copyright (C) 2013  Jtalk <me@jtalk.me>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public Licens
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "ZipDecoder.h"

#include "utility.h"

#include <archive_entry.h>

const ZipDecoder::streampos ZipDecoder::BUFFER_SIZE;
const ZipDecoder::streampos ZipDecoder::BUFFER_OVERLAP;

ZipDecoder::ZipDecoder(const BaseDecoder::stream_t& stream):
	BaseDecoder(stream), overlap_buffer(0), overlap_buffer_offset(0),
	offset(0), is_eof(false)
{
	this->init();
}
ZipDecoder::~ZipDecoder()
{
	this->finalize();
}

void ZipDecoder::init()
{
	logger->verbose("Starting ZIP archive reader initialization");
	
	this->stream->seekg(0);
	
	this->archive_state = archive_read_new();

	archive_read_support_filter_all(this->archive_state);
	archive_read_support_format_all(this->archive_state);

	auto result = archive_read_open2(this->archive_state, this, open_callback, read_callback, skip_callback, nullptr);

	logger->verbose("ZIP Archive is opened with result %u", result);
	
	if (result != ARCHIVE_OK) {
		logger->warning("Error %d while opening archive: %s", result, archive_error_string(this->archive_state));
		this->is_eof = true;
	}
}

void ZipDecoder::finalize()
{
	logger->verbose("Finalizing ZIP archive reader");
	
	archive_read_free(this->archive_state);
	this->buffers.clear();
}

ssize_t ZipDecoder::read_callback(archive *archive_state, void *data_raw, const void **buffer)
{
	auto data = (ZipDecoder*)data_raw;

	logger->verbose("Read callback is called for ZIP archive");
	
	if (!*data->stream) {
		archive_set_error(archive_state, ZipDecoder::READ_ERROR, "ZipDecoder is trying to read from an invalid object");
		return -1;
	}

	if (data->stream->eof()) {
		logger->verbose("Stream EOF in ZIP read callback");
		return 0;
	}

	auto inserted = data->buffers.emplace(data->buffers.end(), BUFFER_SIZE);
	auto read = data->stream->read(*inserted, BUFFER_SIZE);
	inserted->reset(read);

	logger->verbose("%u bytes is read from archive in ZIP read callback", read);
	
	*buffer = inserted->begin();
	return inserted->size();
}

int ZipDecoder::open_callback(archive *archive_state, void *data_raw)
{
	auto data = (ZipDecoder*)data_raw;

	if (!data->stream->operator!())
		return ARCHIVE_OK;

	archive_set_error(archive_state, ZipDecoder::STREAM_INCORRECT, "Incorrect stream passed to ZipDecoder");
	return ARCHIVE_FATAL;
}

off_t ZipDecoder::skip_callback(archive*, void* data_raw, off_t request)
{
	auto data = (ZipDecoder*)data_raw;
	
	logger->verbose("Skip callback is called in ZIP archive handler for %d points", request);

	if (data->stream->eof()) {
		logger->verbose("Stream EOF in ZIP skip callback");
		return 0;
	}

	auto current = data->stream->tellg();
	
	logger->verbose("Skipping from pos %u", current);
	
	data->stream->seekg(current + request);
	auto new_off = data->stream->tellg();
	
	logger->verbose("New offset is %u", new_off);

	return new_off - current;
}

void ZipDecoder::get_overlap(Buffer &buffer, streampos size)
{
	DEBUG_ASSERT(buffer.empty(), "Overlap buffer is not empty in ZipDecoder overlap getting, size is %u", buffer.size());
	DEBUG_ASSERT(this->offset >= this->overlap_buffer_offset, "Overlap buffer is not containing enough data: offset is %u, overlap offset is %u", this->offset, this->overlap_buffer_offset);
	DEBUG_ASSERT(this->offset <= this->overlap_buffer_offset + this->overlap_buffer.size(), "Offset is beyond overlap buffer: offset is %u, buffer end at %u", this->offset, this->overlap_buffer_offset + this->overlap_buffer.size());
	
	auto overlap_start = this->offset - size;
	if (this->offset < size) {
		overlap_start = 0;
	}
	
	if (overlap_start > this->overlap_buffer_offset) {
		auto moving_start = overlap_start - this->overlap_buffer_offset;
		this->overlap_buffer.move_front(moving_start, this->overlap_buffer.size() - moving_start);
		this->overlap_buffer_offset = overlap_start;
	}
	
	auto overlap_fetch_size = std::min(this->overlap_buffer.size(), size);
	buffer.capture(this->overlap_buffer.cbegin(), overlap_fetch_size);
	this->overlap_buffer.move_front(overlap_fetch_size, this->overlap_buffer.size() - overlap_fetch_size);
	this->offset += overlap_fetch_size;
	
	logger->verbose("Getting buffers overlap in ZIP decoder, size is %u, overlap buffer size is %u", buffer.size(), this->overlap_buffer.size());
}

void ZipDecoder::update_overlap(const Buffer &buffer, streampos old_offset)
{
	logger->verbose("Updating ZIP decoder overlap buffer");
	logger->verbose("New overlap size is %u", buffer.size());
	
	this->overlap_buffer.clear();
	this->overlap_buffer.capture(buffer);
	this->overlap_buffer_offset = old_offset;
	
	logger->verbose("Captured %u bytes from %u size buffer", this->overlap_buffer.size(), buffer.size());
}
#include <fstream>
ZipDecoder::streampos ZipDecoder::read(Buffer &buffer, streampos size)
{
	logger->verbose("Entering ZipDecoder read, offset is %u, overlap buffer size is %u, size requested is %u", this->offset, this->overlap_buffer.size(), size);
	
	if (this->eof()) {
		return npos;
	}

	buffer.clear();

	this->get_overlap(buffer, size);

	if (buffer.size() >= size or this->is_eof) {
		logger->verbose("Leaving ZipDecoder read with overlap-only data, offset is %u, overlap buffer size is %u", this->offset, this->overlap_buffer.size());
		return buffer.size();
	}
	
	archive_entry *entry;
	
	do {
		auto result = archive_read_next_header(this->archive_state, &entry);
		
		logger->debug("Archive header pathname is %s", archive_entry_pathname(entry));
		
		if (result == ARCHIVE_EOF) {
			logger->verbose("End of archive is reached in ZIP decode header reader");
			this->is_eof = true;
			break;
		} else if (result == ARCHIVE_RETRY) {
			logger->warning("Archive retry requested in header read");
			break;
		} else if (result != ARCHIVE_OK) {
			logger->warning("Error %d while reading archive chunk in header: %s", result, archive_error_string(this->archive_state));
			this->is_eof = true;
			break;
		}
		
		Buffer tmp(BUFFER_SIZE);
		size_t chunk_size = std::min(BUFFER_SIZE, size - buffer.size());
		ssize_t read = archive_read_data(this->archive_state, tmp.begin(), chunk_size);
		
		if (read == 0) {
			logger->verbose("End of archive is reached in ZIP decoder block reader");
			// Libarchive says it's an end of file, but that's not true. It has just got to the entry final
			continue;
		} else if (read == ARCHIVE_RETRY) {
			logger->warning("Archive retry is returned");
			break;
		} else if (result == ARCHIVE_WARN or result == ARCHIVE_FATAL) {
			logger->warning("Error %d while reading archive chunk in block: %s", result, archive_error_string(this->archive_state));
			this->is_eof = true;
			break;
		}
		
		buffer.capture(tmp.cbegin(), read);
		
	} while (buffer.size() < size);

	auto old_offset = this->offset;
	this->offset += std::min(buffer.size(), size);
	this->update_overlap(buffer, old_offset);
	
	return buffer.size();
}

void ZipDecoder::skip(streampos amount)
{
	Buffer buffer(0);
	
	logger->verbose("Skipping %u bytes in ZIP decoder", amount);
	
	while (amount > 0) {
		streampos new_amount = std::max(int64_t(amount) - int64_t(BUFFER_SIZE), int64_t());
		streampos current_amount = (amount - new_amount);
		amount = new_amount;
		buffer.reset(current_amount);
		this->read(buffer, current_amount);
	}
}

void ZipDecoder::reset()
{
	logger->verbose("Resetting ZIP decoder");
	
	this->finalize();
	this->init();
	
	this->is_eof = false;
	
	this->offset = this->overlap_buffer_offset = 0;
	this->overlap_buffer.clear();
}

void ZipDecoder::seekg(streampos requested_offset)
{
	if (requested_offset == this->offset) {
		return;
	}

	streampos buffer_end = this->overlap_buffer_offset + this->overlap_buffer.size();

	DEBUG_ASSERT(requested_offset >= this->overlap_buffer_offset, 
		     "Seeking offset in ZipDecoder fails: requested offset is %u, but overlap buffer starts at %u", requested_offset, this->overlap_buffer_offset);

	if (requested_offset <= buffer_end) {
		size_t in_buffer_offset = requested_offset - this->overlap_buffer_offset;
		size_t in_buffer_rest = this->overlap_buffer.size() - in_buffer_offset;
		bool moved = this->overlap_buffer.move_front(in_buffer_offset, in_buffer_rest);
		DEBUG_ASSERT(moved, "Unable to move buffer data at pos %u in ZipDecoder seekg with buffer size %u", in_buffer_offset, this->overlap_buffer.size());
	} else {
		auto to_skip = requested_offset - this->overlap_buffer_offset - this->overlap_buffer.size();
		this->overlap_buffer.clear();
		this->overlap_buffer_offset = 0;
		this->skip(to_skip);
	}
	
	this->offset = requested_offset;
}

ZipDecoder::streampos ZipDecoder::tellg() const
{
	return this->offset;
}

bool ZipDecoder::eof() const
{
	return this->is_eof and this->overlap_buffer.size() + this->overlap_buffer_offset <= this->offset;
}
