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

ZipDecoder::ZipDecoder(const BaseDecoder::stream_t& stream):
	BaseDecoder(stream), buffer(BUFFER_SIZE)
{
	this->archive_state = archive_read_new();

	archive_read_support_filter_all(this->archive_state);
	archive_read_support_format_zip(this->archive_state);

	archive_read_open2(this->archive_state, this, open_callback, read_callback, skip_callback, nullptr);
}

ZipDecoder::~ZipDecoder()
{
	archive_read_free(this->archive_state);
}

ssize_t ZipDecoder::read_callback(archive *archive_state, void *data_raw, const void **buffer)
{
	auto data = (ZipDecoder*)data_raw;

	if (!data->stream->operator!()) {
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

	if (new_off == npos)
		return 0;

	return new_off;
}

ZipDecoder::streampos ZipDecoder::read(uint8_t* buffer, streampos size)
{
	if (this->eof())
		return npos;
	
	archive_entry *entry;
	streampos totally_read = 0;
	int result;
	
	do {
		result = archive_read_next_header(this->archive_state, &entry);
		
		this->is_eof = (result == ARCHIVE_EOF);
		if (this->eof())
			break;
		
		auto read = archive_read_data(this->archive_state, buffer + totally_read, size - totally_read);

		if (read == 0)
			break;
		
		totally_read += read;
		
	} while (result == ARCHIVE_OK);
	
	return totally_read;
}

void ZipDecoder::seekg(streampos offset)
{
}

ZipDecoder::streampos ZipDecoder::tellg() const
{
}

bool ZipDecoder::eof() const
{
	return this->is_eof;
}