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

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "utility.h"

#include <algorithm>
#include <vector>

// Rabin-Karp hash primes
static const uint32_t RK_MODULUS = 59999;
static const uint32_t RK_COEFF = 56999;
static const uint32_t RK_INVERSE = 59979;

static const uint32_t RK_HASH_TABLE_FACTOR = 20;

typedef std::vector<uint32_t> powers_cache_t;

static uint32_t rk_pow_mod(uint32_t base, uint32_t exp, uint32_t modulus) {
	base %= modulus;
	uint32_t result = 1;
	while (exp > 0) {
		if (exp & 1) {
			result = (result * base) % modulus;
		}
		base = (base * base) % modulus;
		exp >>= 1;
	}
	return result;
}

static void rk_powers(powers_cache_t &cache, size_t chunk_size) {
	cache.clear();
	cache.reserve(chunk_size);
	for (size_t i = 0; i < chunk_size; i++) {
		cache.push_back(rk_pow_mod(RK_COEFF, i, RK_MODULUS));
	}
}

static bool rk_chunk_size_compare(const search_terms_t::value_type &fst, const search_terms_t::value_type &snd) {
	return fst.size() < snd.size();
}

struct rk_hash {
	powers_cache_t cache;
	
	size_t new_hash(Buffer::const_iterator first) const {
		uint32_t hash = 0;
		for (int32_t i = this->cache.size() - 1; i >= 0; i--) {
			hash = (hash + *(first++) * this->cache[i]) % RK_MODULUS;
		}
		return hash;
	}
	
	size_t reuse_hash(size_t old_hash, uint8_t removing, uint8_t adding, size_t chunk_size) {
		uint32_t removing_value = removing * this->cache[chunk_size - 1] % RK_MODULUS;
		if (removing_value > old_hash) {
			old_hash = old_hash += RK_MODULUS;
		}
		old_hash -= removing_value;
		uint32_t adding_value = adding * this->cache[0] % RK_MODULUS;
		old_hash = (old_hash + adding_value) % RK_MODULUS;
		return old_hash;
	}
};
	
namespace utility {
	
SearchResult rabin_karp(const Buffer &string, const search_terms_t &to_find) {
	size_t chunk_size = std::min_element(CONTAINER(to_find), rk_chunk_size_compare)->size();
	
	if (string.size() < chunk_size) {
		return {-1, Buffer::npos};
	}
	
	rk_hash hasher;
	rk_powers(hasher.cache, chunk_size);
	
	typedef std::vector<size_t> collisions_t;
	typedef std::pair<Buffer::const_iterator, Buffer::const_iterator> iterators_t;
	
	std::unordered_map<size_t, collisions_t> patterns(to_find.size() * RK_HASH_TABLE_FACTOR);
	size_t i = 0;
	for (auto &pattern : to_find) {
		uint32_t hash = hasher.new_hash(pattern.data());
		patterns[hash].push_back(i++);
	}
	
	uint32_t hash = 0;
	for (size_t current_pos = 0; string.cbegin() + current_pos < string.cend() - chunk_size + 1; current_pos++) {
		iterators_t iters{string.cbegin() + current_pos, string.cbegin() + current_pos + chunk_size};
		if (current_pos == 0) {
			hash = hasher.new_hash(string.cbegin());
		} else {
			hash = hasher.reuse_hash(hash, *(iters.first - 1), *(iters.second - 1), chunk_size);
		}
		auto found = patterns.find(hash);
		if (found != patterns.cend()) {
			for (const auto &index : found->second) {
				const auto &pattern = to_find[index];
				size_t buffer_left = string.cend() - string.cbegin() - current_pos;
				if (buffer_left < pattern.size()) {
					continue;
				}
				bool matched = true;
				for (size_t i = 0; i < pattern.size(); i++) {
					if (pattern[i] != iters.first[i]) {
						matched = false;
						break;
					} 
				}
				if (matched) {
					return {int64_t(index), current_pos};
				}
			}
		}
	}
	return {-1, Buffer::npos};
}

}
	