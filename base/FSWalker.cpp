/*
    Disk Crawler Library.
    Copyright (C) 2013  Jtalk <me@jtalk.me>

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


#include "FSWalker.h"

#include "FSFileStream.h"
#include "BaseDecoder.h"

#include "ZipDecoder.h"

#include "utility.h"

#include <cstdio>

const FSWalker::signatures_t FSWalker::signatures = {
	"meanwhile"_us
};

FSWalker::FSWalker(const std::string &device_name)
{
	this->device = fopen(device_name.c_str(), "rb");
}

FSWalker::~FSWalker()
{
	fclose(this->device);
}

FSWalker::decoders_t FSWalker::decode(FSFileStream* stream)
{
	decoders_t decoders;
	BaseDecoder::stream_t to_decode(stream);
	
	decoders.push_front(new ZipDecoder(to_decode));
	
	return decoders;
}

FSWalker::results_t FSWalker::find(FSFileStream *stream, const byte_array_t &to_find)
{
	auto &&decoders = this->decode(stream);
	
	results_t results;
	
	for (auto &decoder : decoders)
	{
		auto found = utility::find(*decoder, to_find);
		
		if (found != BaseDecoder::npos)
			results.emplace_front(decoder, found);
		else 
			delete decoder;
	}
	
	return results;
}

FSWalker::results_t FSWalker::find(const byte_array_t& to_find)
{
	auto signature_matches = this->find_by_signatures();

	if (signature_matches.empty())
		utility::log("No signatures detected");

	results_t found;

	for (auto & match : signature_matches) {
		auto file_stream = this->traceback(match);

		if (file_stream == nullptr) {
			continue;
		}

		found.splice(found.end(), this->find(file_stream, to_find));
	}

	return found;
}

bool FSWalker::operator !() const
{
	return !this->device;
}

