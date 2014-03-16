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

#pragma once

#include "BaseDecoder.h"
#include "ByteReader.h"

#include <archive.h>

#undef read

class ZipDecoder : public BaseDecoder
{
public:
	enum Error
	{
		STREAM_INCORRECT = 0,
		READ_ERROR,
	};

private:
	static const streampos BUFFER_SIZE = 100000000;
	static const streampos BUFFER_OVERLAP = 1000;
	
	archive *archive_state;
	
	Buffer buffer;
	
	Buffer overlap_buffer;
	streampos overlap_buffer_offset;
	
	streampos offset;
	
	bool is_eof;
	bool header_read;
	
	static ssize_t read_callback(archive *archive_state, void *data_raw, const void **buffer);
	static int open_callback(archive *archive_state, void *data_raw);
	static off_t skip_callback(archive *archive_state, void *data_raw, off_t request);
	
	void get_overlap(Buffer &buffer);
	void update_overlap(const Buffer &buffer);
	
	void init();
	void finalize();
	
	void skip(streampos requested_offset);
		
public:	
	ZipDecoder(const stream_t &stream);
	virtual ~ZipDecoder();
	
	ZipDecoder::streampos read(Buffer &buffer, streampos size) override;
	
	virtual void reset() override;
	
	virtual void seekg(streampos offset) override;
	virtual streampos tellg() const override;
	
	virtual bool eof() const override;
};