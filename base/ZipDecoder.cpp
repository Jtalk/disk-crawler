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
	BaseDecoder(stream), overlap(0),
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
	logger()->verbose("Starting ZIP archive reader initialization");
	
	this->stream->seekg(0);
	
	this->archive_state = archive_read_new();

	archive_read_support_filter_all(this->archive_state);
	archive_read_support_format_all(this->archive_state);

	auto result = archive_read_open2(this->archive_state, this, open_callback, read_callback, skip_callback, nullptr);

	logger()->verbose("ZIP Archive is opened with result %u", result);
	
	if (result != ARCHIVE_OK) {
		logger()->warning("Error %d while opening archive: %s", result, archive_error_string(this->archive_state));
		this->is_eof = true;
	}
}

void ZipDecoder::finalize()
{
	logger()->verbose("Finalizing ZIP archive reader");
	
	archive_read_free(this->archive_state);
	this->buffers.clear();
}

ssize_t ZipDecoder::read_callback(archive *archive_state, void *data_raw, const void **buffer)
{
	auto data = (ZipDecoder*)data_raw;

	logger()->verbose("Read callback is called for ZIP archive");
	
	if (!*data->stream) {
		archive_set_error(archive_state, ZipDecoder::READ_ERROR, "ZipDecoder is trying to read from an invalid object");
		return -1;
	}

	if (data->stream->eof()) {
		logger()->verbose("Stream EOF in ZIP read callback");
		return 0;
	}

	auto inserted = data->buffers.emplace(data->buffers.end(), BUFFER_SIZE);
	auto read = data->stream->read(*inserted, BUFFER_SIZE);
	inserted->reset(read);

	logger()->verbose("%u bytes is read from archive in ZIP read callback", read);
	
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
	
	logger()->verbose("Skip callback is called in ZIP archive handler for %d points", request);

	if (data->stream->eof()) {
		logger()->verbose("Stream EOF in ZIP skip callback");
		return 0;
	}

	auto current = data->stream->tellg();
	
	logger()->verbose("Skipping from pos %u", current);
	
	data->stream->seekg(current + request);
	auto new_off = data->stream->tellg();
	
	logger()->verbose("New offset is %u", new_off);

	return new_off - current;
}

ZipDecoder::streampos ZipDecoder::read(Buffer &buffer, streampos size)
{
	logger()->verbose("Entering ZipDecoder read, offset is %u, overlap buffer size is %u, size requested is %u", this->offset, this->overlap.size(), size);
	
	if (this->eof()) {
		return npos;
	}

	buffer.reset(size);
	size_t extracted = this->overlap.extract(buffer, size, this->offset);
	this->offset += extracted;
	
	DEBUG_ASSERT(extracted <= size, "Extracted too much data from overlap buffer at ZipDecoder: %u while %u requested", extracted, size);
	
	if (extracted == size or this->is_eof) {
		logger()->verbose("Leaving ZipDecoder read with overlap-only data, offset is %u, overlap buffer size is %u", this->offset, this->overlap.size());
		return buffer.size();
	}
	
	archive_entry *entry;
	
	do {
		auto result = archive_read_next_header(this->archive_state, &entry);
		
		logger()->verbose("Archive header extracted, pathname is %s", archive_entry_pathname(entry));
		
		if (result == ARCHIVE_EOF) {
			logger()->verbose("End of archive is reached in ZIP decode header reader");
			this->is_eof = true;
			break;
		} else if (result == ARCHIVE_RETRY) {
			logger()->warning("Archive retry requested in header read");
			continue;
		} else if (result != ARCHIVE_OK) {
			logger()->warning("Error %d while reading archive chunk in header: %s", result, archive_error_string(this->archive_state));
			this->is_eof = true;
			break;
		}
		
		Buffer tmp(BUFFER_SIZE);
		size_t chunk_size = std::min(BUFFER_SIZE, size - buffer.size());
		ssize_t read = archive_read_data(this->archive_state, tmp.begin(), chunk_size);
		
		if (read == 0) {
			logger()->verbose("End of archive is reached in ZIP decoder block reader");
			// Libarchive says it's an end of file, but that's not true. It has just got to the entry final
			continue;
		} else if (read == ARCHIVE_RETRY) {
			logger()->warning("Archive retry is returned");
			break;
		} else if (result == ARCHIVE_WARN or result == ARCHIVE_FATAL) {
			logger()->warning("Error %d while reading archive chunk in block: %s", result, archive_error_string(this->archive_state));
			this->is_eof = true;
			break;
		}
		
		buffer.capture(tmp.cbegin(), read);
		
	} while (buffer.size() < size);

	this->overlap.capture(buffer, extracted, this->offset);
	this->offset += (buffer.size() - extracted);
		
	return buffer.size();
}

void ZipDecoder::skip(streampos amount)
{
	Buffer buffer(0);
	
	logger()->verbose("Skipping %u bytes in ZIP decoder", amount);
	
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
	logger()->verbose("Resetting ZIP decoder");
	
	this->finalize();
	this->init();
	
	this->is_eof = false;
	
	this->overlap.clear();
	this->offset = 0;
}

void ZipDecoder::seekg(streampos requested_offset)
{
	if (requested_offset == this->offset) {
		return;
	}
	
	DEBUG_ASSERT(requested_offset >= this->offset or this->overlap.suitable(requested_offset), 
		     "Seeking invalid offset %u in ZipDecoder: current is %u, overlap size is %u", requested_offset, this->offset, this->overlap.size());
	
	if (requested_offset > this->offset) {
		this->skip(requested_offset - this->offset);
	} else {
		this->offset = requested_offset;
	}
}

ZipDecoder::streampos ZipDecoder::tellg() const
{
	return this->offset;
}

bool ZipDecoder::eof() const
{
	return this->is_eof and not this->overlap.suitable(this->offset) and not this->overlap.empty();
}
