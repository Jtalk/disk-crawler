/*
 *  Disk Crawler library.
 *  Copyright (C) 2013  Jtalk <me@jtalk.me>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

class Log
{
	friend Log* logger();
	
	FILE *descriptor;
	static Log *logger;
	
public:
	Log(): descriptor(stdout) {
		Log::logger = this;
	}
	
	Log(const std::string &name): descriptor(fopen(name.c_str(), "a")) {
		Log::logger = this;
	}
	
	~Log() {
		if (this->descriptor != stdout)
			fclose(this->descriptor);
	}
	
	void warning(const char *fmt, ...);
	void error(const char *fmt, ...);
	void info(const char *fmt, ...);
	void verbose(const char *fmt, ...);
	
#ifdef DEBUG
	void debug(const char *fmt, ...);
#else
	void debug(const char*, ...) 
	{}
#endif
};

inline Log *logger() {
	return Log::logger;
}