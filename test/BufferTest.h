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

#include "../base/Buffer.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstring>

class BufferTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(BufferTest);
	CPPUNIT_TEST(test_buffer_size);
	CPPUNIT_TEST(test_capture);
	CPPUNIT_TEST(test_capture_overflow);
	CPPUNIT_TEST(test_capture_offset);
	CPPUNIT_TEST(test_move);
	CPPUNIT_TEST(test_move_empty);
	CPPUNIT_TEST(test_shrink);
	CPPUNIT_TEST_SUITE_END();

public:
	void test_buffer_size() {
		const size_t SIZE = 10;
		Buffer buffer(SIZE);
		CPPUNIT_ASSERT(buffer.empty());
		
		const size_t NEW = 100;
		buffer.reset(NEW);
		CPPUNIT_ASSERT(buffer.empty());
		
		const char text[] = "abcdefghijklmnop";
		buffer.capture((const uint8_t*)text, sizeof(text));
		CPPUNIT_ASSERT(buffer.size() == sizeof(text));
	}
	
	void test_capture() {
		Buffer buffer(10);
		char text[] = "abcd";
		buffer.capture((const uint8_t*)text, sizeof(text));
		CPPUNIT_ASSERT(not memcmp(buffer.begin(), text, sizeof(text)));
	}

	void test_capture_overflow() {
		Buffer buffer(10);
		char text[] = "abcdefghklmnopqrstuvwxyz";
		buffer.capture((const uint8_t*)text, sizeof(text));
		CPPUNIT_ASSERT(buffer.size() == sizeof(text));
		CPPUNIT_ASSERT(not memcmp(buffer.begin(), text, sizeof(text)));
	}

	void test_capture_offset() {
		const size_t SIZE = 10;
		Buffer buffer(SIZE);
		const char text1[] = "abcdefghij";
		buffer.capture((const uint8_t*)text1, sizeof(text1));
		
		const char text[] = "abcdefghklmnopqrstuvwxyz";
		buffer.capture((const uint8_t*)text, sizeof(text));
		CPPUNIT_ASSERT(buffer.size() == sizeof(text) + sizeof(text1));
		CPPUNIT_ASSERT(not memcmp(buffer.begin() + sizeof(text1), text, sizeof(text)));
	}
	
	void test_move() {
		const size_t SIZE = 10;
		Buffer buffer(SIZE);
		char text[] = "abcdefghijklmnop";
		buffer.capture((const uint8_t*)text, sizeof(text));
		CPPUNIT_ASSERT(buffer.move_front(SIZE, sizeof(text) - SIZE));
		CPPUNIT_ASSERT(not memcmp(buffer.begin(), text + SIZE, sizeof(text) - SIZE));
	}
	
	void test_move_empty() {
		const size_t SIZE = 10;
		Buffer buffer(SIZE);
		const char text[] = "abcdefghij";
		buffer.capture((const uint8_t*)text, sizeof(text));
		CPPUNIT_ASSERT(buffer.move_front(SIZE, 0));
		CPPUNIT_ASSERT(buffer.size() == 0);
	}
	
	void test_shrink() {
		size_t NEW = 9;
		Buffer buffer(100);
		const char text[] = "abcdefghijklmnop";
		buffer.capture((const uint8_t*)text, sizeof(text));
		buffer.shrink(NEW);
		CPPUNIT_ASSERT(buffer.size() == NEW);
	}
};
