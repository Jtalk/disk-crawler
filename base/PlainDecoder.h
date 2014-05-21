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

#include "BaseDecoder.h"

class PlainDecoder : public BaseDecoder
{
public:
	PlainDecoder(const stream_t &stream);
	virtual ~PlainDecoder();

	virtual streampos read(Buffer &buffer, streampos size) override;
	
	virtual void seekg(streampos offset) override;
	virtual streampos tellg() const override;
	
	virtual bool eof() const override;
};