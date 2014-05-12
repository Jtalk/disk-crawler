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

#include "INode.h"

#include <utility>

#include <cstdlib>
#include <cstring>

INode::INode(): file_size(0) {
	this->blocks = (uint32_t*)calloc(FILE_BLOCKS_MAX, sizeof(uint32_t));
	memset(this->blocks, 0, FILE_BLOCKS_MAX * sizeof(uint32_t)); // TODO: If this necessary?
}

INode::INode(INode && other) {
	this->file_size = other.file_size;
	this->blocks = other.blocks;
	other.file_size = 0;
	other.blocks = nullptr;
}

INode::~INode() {
	free(this->blocks);
}

