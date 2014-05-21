/*
    Disk Crawler library.
    Copyright (C) 2013-2014 Roman Nazarenko <me@jtalk.me>

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

#pragma once

#include "FSFileStream.h"

class PlainFileStream : public FSFileStream {
public:
	PlainFileStream(const std::string& device_name);
	
	virtual ByteReader::streampos tellg() const override;
	virtual void seekg(ByteReader::streampos offset) override;
	virtual ByteReader::streampos read(Buffer& buffer, ByteReader::streampos size) override;
	
	virtual bool eof() const override;
	
protected:
	virtual bool correct() const override;
};