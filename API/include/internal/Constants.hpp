/*
 *  File: Constants.hpp
 *  Copyright (c) 2023 Florian Porrmann
 *
 *  MIT License
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace clap
{
inline constexpr std::size_t ALIGNMENT     = 4096;
inline constexpr uint64_t USE_VECTOR_SIZE  = 0; // Flag to automatically calculate the byte size based on the vector size and type
inline constexpr int32_t WAIT_INFINITE     = -1;
inline constexpr uint8_t SAME_AS_START_BIT = 0xFF;
inline constexpr uint64_t USE_MEMORY_SIZE  = 0;

namespace internal
{
inline constexpr uint64_t XDMA_STREAM_OFFSET         = 0;   // For streams data is always write/read from/to the offset 0
inline constexpr std::size_t XDMA_AXI_DATA_WIDTH_BIT = 512; // 512-Bit
inline constexpr std::size_t XDMA_AXI_DATA_WIDTH     = XDMA_AXI_DATA_WIDTH_BIT / 8;

inline constexpr uint64_t XDMA_CTRL_BASE = 0x0;
inline constexpr uint64_t XDMA_CTRL_SIZE = 0x100;

inline constexpr int32_t MINUS_ONE_S = -1;
inline constexpr uint32_t MINUS_ONE_U = 0xFFFFFFFF;

/*
 * man 2 write:
 * On Linux, write() (and similar system calls) will transfer at most
 * 	0x7ffff000 (2,147,479,552) bytes, returning the number of bytes
 *	actually transferred.  (This is true on both 32-bit and 64-bit
 *	systems.)
 */
inline constexpr uint32_t RW_MAX_SIZE = 0x7ffff000;
} // namespace internal

inline constexpr uint32_t USE_AUTO_DETECT = internal::MINUS_ONE_U;
inline constexpr int32_t INTR_UNDEFINED  = internal::MINUS_ONE_S;
inline constexpr uint32_t UNSET_INTR_MASK = 0;

} // namespace clap