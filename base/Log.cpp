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

#include "Log.h"
	
#include "Config.h"
	
Log *Log::logger;
	
void Log::warning(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(this->descriptor, "Warning: ");
	vfprintf(this->descriptor, fmt, args);
	fputs("\n", this->descriptor);
	va_end(args);		
}

void Log::error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(this->descriptor, "Error: ");
	vfprintf(this->descriptor, fmt, args);
	fputs("\n", this->descriptor);
	va_end(args);
	abort();
}

void Log::info(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(this->descriptor, fmt, args);
	fputs("\n", this->descriptor);
	va_end(args);
}

#ifdef DEBUG
void Log::debug(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(this->descriptor, "Debug: ");
	vfprintf(this->descriptor, fmt, args);
	fputs("\n", this->descriptor);
	va_end(args);		
}
#endif
void Log::verbose(const char *fmt, ...) {
	if (not config()->VERBOSE) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	fprintf(this->descriptor, "Verbose: ");
	vfprintf(this->descriptor, fmt, args);
	fputs("\n", this->descriptor);
	va_end(args);		
}