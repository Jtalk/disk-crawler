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

#include "base/utility.h"

ZipDecoder::ZipDecoder(const BaseDecoder::stream_t& stream):
	BaseDecoder(stream), buffer(BUFFER_SIZE), overlap_buffer(0)
{
	this->archive_state = archive_read_new();

	archive_read_support_filter_all(this->archive_state);
	archive_read_support_format_all(this->archive_state);

	archive_read_open2(this->archive_state, this, open_callback, read_callback, skip_callback, nullptr);
}

ZipDecoder::~ZipDecoder()
{
	archive_read_free(this->archive_state);
}

ssize_t ZipDecoder::read_callback(archive *archive_state, void *data_raw, const void **buffer)
{
	auto data = (ZipDecoder*)data_raw;

	if (!*data->stream) {
		archive_set_error(archive_state, ZipDecoder::READ_ERROR, "ZipDecoder is trying to read from an invalid object");
		return -1;
	}

	if (data->stream->eof())
		return 0;

	data->buffer.resize(BUFFER_SIZE);
	auto read = data->stream->read(data->buffer.begin(), data->buffer.size());
	data->buffer.resize(read);
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

	if (data->stream->eof())
		return 0;

	auto current = data->stream->tellg();
	data->stream->seekg(current + request);
	auto new_off = data->stream->tellg();

	return new_off - current;
}

ZipDecoder::streampos ZipDecoder::read(Buffer &buffer, streampos size)
{
	if (this->eof())
		return npos;
	
	buffer.clear();
	archive_entry *entry;
	streampos totally_read = 0;
	int result;
	
	do {
		result = archive_read_next_header(this->archive_state, &entry);
		
		this->is_eof = (result == ARCHIVE_EOF);
		
		if (this->eof() or result != ARCHIVE_OK)
			break;
		
		const uint8_t *read_buffer;
		size_t read = 0;
		int64_t offset;
		
		while (not archive_read_data_block(this->archive_state, (const void**)&read_buffer, &read, &offset)) {
			if (read == 0) {
				break;
			}
			
			buffer.capture(read_buffer, read);
			totally_read += read;
			
			if (totally_read >= size) {
				break;
			}
		}
		
	} while (totally_read < size);
	
	return totally_read;
}

void ZipDecoder::seekg(streampos offset)
{
	DEBUG_ASSERT(offset < this->overlap_buffer_offset, "Seeking by an invalid offset is asked in ZipDecoder. Offset requested is %u, but current buffer offset is %u", offset, this->buffer_offset);
	
	streampos buffer_end = this->overlap_buffer_offset + this->overlap_buffer.size();
	(void)buffer_end;
	DEBUG_ASSERT(buffer_end <= offset, "Seeking offset is increasing too fast in ZipDecoder. From %u to %u", buffer_end, offset);
		
	size_t in_buffer_offset = offset - this->overlap_buffer_offset;
	size_t in_buffer_rest = this->overlap_buffer.size() - in_buffer_offset;
	this->overlap_buffer.move_front(in_buffer_offset, in_buffer_rest);
}

ZipDecoder::streampos ZipDecoder::tellg() const
{
	return this->overlap_buffer_offset + this->overlap_buffer.size();
}

bool ZipDecoder::eof() const
{
	return this->is_eof or this->stream->eof();
}