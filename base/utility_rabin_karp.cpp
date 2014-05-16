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

static rk_powers(powers_cache_t &cache, size_t chunk_size) {
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
	typedef std::pair<byte_array_t::const_iterator, byte_array_t::const_iterator> iterators_t;
	powers_cache_t cache;
	bool reuse;
	bool new_round;
	size_t old_hash;
	
	size_t operator ()(const iterators_t &iterators) const {
		if (this->reuse) {
			return this->reuse_hash(iterators);
		}
		if (not this->new_round) {
			return this->old_hash;
		}
		uint32_t hash = 0;
		for (int32_t i = this->cache.size() - 1; i >= 0; i++) {
			hash = (hash + *(iterators.first++) * this->cache[i]) % RK_MODULUS;
		}
		this->old_hash = hash;
		this->new_round = false;
		return hash;
	}
	
	size_t reuse_hash(const iterators_t &iterators) const {
		auto removing = *(iterators.first - 1);
	}
	
	rk_hash(const rk_hash&) = delete;
	rk_hash(rk_hash&&) = delete;
	rk_hash &operator=(const rk_hash&) = delete;
	rk_hash &operator=(rk_hash&&) = delete;
};
	
namespace utility {
	
SearchResult rabin_karp(const Buffer &string, const search_terms_t &to_find) {
	size_t chunk_size = std::min_element(CONTAINER(to_find), rk_chunk_size_compare)->size();
	
	rk_hash hasher;
	hasher.reuse = false;
	hasher.new_round = true;
	rk_powers(hasher.cache, chunk_size);
	
	std::unordered_map<rk_hash::iterators_t, size_t, rk_hash> patterns(to_find.size() * RK_HASH_TABLE_FACTOR, hasher);
	size_t i = 0;
	for (auto &pattern : to_find) {
		patterns.insert({{pattern.cbegin(), pattern.cend()}, i++});
	}
	
	size_t current_pos = 0;
	while (true) {
		rk_hash::iterators_t iters{string.cbegin() + current_pos, string.cbegin() + current_pos + chunk_size};
		if (iters.second >= string.cend()) {
			break;
		}
		if (hasher.reuse) {
			hasher.removing = *(iters.first - 1);
			hasher.adding = *(iters.second - 1);
			hasher.new_round = true;
		}
		auto found = patterns.find(iters);
		if (found != patterns.cend()) {
			return {found->second, current_pos};
		}
		current_pos++;
		hasher.reuse = true;
	}
	return {-1, Buffer::npos};
}

}
	