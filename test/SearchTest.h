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

#include "../base/utility.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstring>

class SearchTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(SearchTest);
	CPPUNIT_TEST(test_search_empty);
	CPPUNIT_TEST(test_search_false);
	CPPUNIT_TEST(test_search);
	CPPUNIT_TEST(test_search_str_less);
	CPPUNIT_TEST_SUITE_END();

public:
	static void copy(Buffer &to, const byte_array_t &from) {
		to.shrink(from.size());
		std::copy(from.cbegin(), from.cend(), to.begin());
	}
	
	search_terms_t to_find;
	
	SearchTest() : CppUnit::TestFixture() {
		to_find = {
			{0, 1, 2, 3, 4},
			{0, 1, 5, 6},
			{2, 3, 5, 6, 7, 5},
			{5, 5, 5, 6, 7},
			{1, 3, 5, 6, 7},
		};
	}
	
	void test_search() {
		Buffer str(100);
		
		byte_array_t _str1 = {4, 5, 6, 3, 2, 0, 1, 5, 6, 7, 3, 4, 5, 6};
		copy(str, _str1);
				
		auto exp_result = utility::SearchResult{1, 5};
		CPPUNIT_ASSERT(utility::rabin_karp(str, this->to_find) == exp_result);
		
		_str1 = {1, 3, 5, 6, 7, 5, 5, 5, 6, 7, 3, 5, 6, 7, 8, 0};
		copy(str, _str1);
		
		exp_result = utility::SearchResult{4, 0};
		CPPUNIT_ASSERT(utility::rabin_karp(str, this->to_find) == exp_result);
	}
	
	void test_search_false() {
		Buffer str(100);
		
		byte_array_t _str1 = {3, 4, 5, 7, 5, 7, 8, 5, 7, 8, 8,7, 8, 8, 7};
		copy(str, _str1);
				
		auto exp_result = utility::SearchResult{-1, Buffer::npos};
		CPPUNIT_ASSERT(utility::rabin_karp(str, this->to_find) == exp_result);
	}
	
	void test_search_empty() {
		Buffer str(100);
		auto exp_result = utility::SearchResult{-1, Buffer::npos};
		CPPUNIT_ASSERT(utility::rabin_karp(str, this->to_find) == exp_result);
	}
	
	void test_search_str_less() {
		Buffer str(100);
		
		byte_array_t _str1 = {3, 4, 5};
		copy(str, _str1);
				
		auto exp_result = utility::SearchResult{-1, Buffer::npos};
		CPPUNIT_ASSERT(utility::rabin_karp(str, this->to_find) == exp_result);		
	}
};