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

ssize_t ZipDecoder::read_callback(archive *archive, void *data_raw, const void **buffer)
{
	auto data = (ZipDecoder*)data_raw;
	
	if (!data->stream->operator!()) {
		archive_set_error(archive, ZipDecoder::READ_ERROR);
		return -1;
	}
	
	if (data->stream->eof())
		return 0;
	
	data->buffer.resize(BUFFER_SIZE);
	auto read = data->stream->read(data->buffer->begin(), data->buffer->size());
	data->buffer.resize(read);
	buffer = data->buffer->begin();
	return read;
}

static int ZipDecoder::open_callback(archive *archive, void *data_raw)
{
	auto data = (ZipDecoder*)data_raw;
	
	if (!data->stream->operator!())
		return ARCHIVE_OK;
	
	archive_set_error(archive, ZipDecoder::STREAM_INCORRECT);
	return ARCHIVE_FATAL;
}

off_t ZipDecoder::skip_callback(archive* archive, void* data_raw, off_t request)
{
	auto data = (ZipDecoder*)data_raw;
	
	auto current = data->stream->tellg();
	data->stream->seekg(current + request);
	auto new_off = data->stream->tellg();
	
	if (new_off == npos)
		return - current;
	
}

ZipDecoder::ZipDecoder(const BaseDecoder::stream_t& stream): 
	BaseDecoder(stream), buffer(BUFFER_SIZE)
{
	this->archive = archive_read_new();
	
	archive_read_support_filter_all(this->archive);
	archive_read_support_format_zip(this->archive);
	
	archive_read_open2(this->archive, this->stream.get(), );
}
