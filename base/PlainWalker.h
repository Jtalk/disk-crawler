/*
 * Disk Crawler Library.
 * Copyright (C) 2014  Jtalk <me@jtalk.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "SignatureWalker.h"

#include "utility.h"

struct PlainWalker : public SignatureWalker {
	PlainWalker(const std::string &device_name, size_t size = utility::MAX_DEVICE_SIZE, const utility::progress_callback_t &callback = utility::progress_callback_t());

	virtual results_t find(const byte_array_t& to_find) override;
	
private:	
	virtual FSFileStream* traceback(size_t) const override
	{return nullptr;}
};
