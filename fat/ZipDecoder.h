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
#include "Buffer.h"

#include <archive.h>

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
	
	archive *archive;
	
	Buffer buffer;
	streampos position;
	
	static size_t read_callback(archive *archive, void *data_raw, const void **buffer);
	static int open_callback(archive *archive, void *data_raw);
	static off_t skip_callback(archive *archive, void *data_raw, off_t request);
	
public:	
	ZipDecoder(const stream_t &stream);
	virtual ~ZipDecoder();
	
	virtual streampos read(uint8_t* buffer, streampos size) override;
	
	virtual void seekg(streampos offset) override;
	virtual streampos tellg() const override;
	
	virtual bool eof() const override;
};