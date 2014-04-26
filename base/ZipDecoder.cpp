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

ZipDecoder::ZipDecoder(const BaseDecoder::stream_t& stream):
	BaseDecoder(stream), buffer(BUFFER_SIZE),
	overlap_buffer(0), overlap_buffer_offset(0),
	offset(0), is_eof(false), header_read(false)
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

	data->buffer.reset(BUFFER_SIZE);
	auto read = data->stream->read(data->buffer, data->buffer.size());
	
	logger->verbose("%u bytes is read from archive in ZIP read callback", read);
	
	data->buffer.reset(read);
	*buffer = data->buffer.begin();
	return read;
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

void ZipDecoder::get_overlap(Buffer &buffer)
{
	DEBUG_ASSERT(buffer.empty(), "Overlap buffer is not empty in ZipDecoder overlap getting, size is %u", buffer.size());
	DEBUG_ASSERT(this->offset >= this->overlap_buffer_offset, "Overlap buffer is not containing enough data: offset is %u, overlap offset is %u", this->offset, this->overlap_buffer_offset);
	DEBUG_ASSERT(this->offset <= this->overlap_buffer_offset + this->overlap_buffer.size(), "Offset is beyond overlap buffer: offset is %u, buffer end at %u", this->offset, this->overlap_buffer_offset + this->overlap_buffer.size());
	
	auto overlap_offset_diff = this->offset - this->overlap_buffer_offset;	
	buffer.capture(this->overlap_buffer.cbegin() + overlap_offset_diff, this->overlap_buffer.size() - overlap_offset_diff);
	
	logger->verbose("Getting buffers overlap in ZIP decoder, size is %u, overlap buffer size is %u", buffer.size(), this->overlap_buffer.size());
}

void ZipDecoder::update_overlap(const Buffer &buffer)
{
	streampos overlap_size = std::min(buffer.size(), size_t(BUFFER_OVERLAP));
	streampos in_buffer_overlap_offset = buffer.size() - overlap_size;
	
	logger->verbose("Updating ZIP decoder overlap buffer");
	logger->verbose("New overlap size is %u", overlap_size);
	logger->verbose("In-buffer overlap offset is %u", in_buffer_overlap_offset);
	
	this->overlap_buffer.clear();
	this->overlap_buffer.capture(buffer.cbegin() + in_buffer_overlap_offset, overlap_size);
	this->overlap_buffer_offset = this->offset - overlap_size;
	
	logger->verbose("Captured %u bytes from %u size buffer", this->overlap_buffer.size(), buffer.size());
}

ZipDecoder::streampos ZipDecoder::read(Buffer &buffer, streampos size)
{
	if (this->eof()) {
		return npos;
	}

	buffer.clear();

	this->get_overlap(buffer);

	archive_entry *entry;
	
	do {
		const uint8_t *read_buffer;
		size_t read = 0;
		int64_t offset;

		auto result = archive_read_next_header(this->archive_state, &entry);
		
		if (result == ARCHIVE_EOF) {
			logger->verbose("End of archive is reached in ZIP decode header reader");
			this->is_eof = true;
			break;
		} else if (result != ARCHIVE_OK) {
			logger->debug("Error %d while reading archive chunk in header: %s", result, archive_error_string(this->archive_state));
			this->is_eof = true;
			break;
		}
		
		result = archive_read_data_block(this->archive_state, (const void**)&read_buffer, &read, &offset);
		
		if (result == ARCHIVE_EOF) {
			logger->verbose("End of archive is reached in ZIP decoder block reader");
			this->is_eof = true;
			break;
		} else if (result != ARCHIVE_OK) {
			logger->debug("Error %d while reading archive chunk in block: %s", result, archive_error_string(this->archive_state));
			this->is_eof = true;
			break;
		}
		
		buffer.capture(read_buffer, read);
		
	} while (buffer.size() < size);

	this->offset += buffer.size();
	this->update_overlap(buffer);
	
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
	this->header_read = false;
	
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
		     "Seeking offset %u in ZipDecoder fails: requested offset is %u, but overlap buffer starts at %u", requested_offset, this->overlap_buffer_offset);

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
	return this->is_eof;
}
