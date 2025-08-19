/*
 *  File: AlignmentAllocator.hpp
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

namespace clap
{
namespace internal
{
#include <cstdlib>
#include <malloc.h>

inline void* alignedMalloc(size_t alignment, size_t size) noexcept
{
#ifdef WIN32
	return _aligned_malloc(size, alignment);
#else
	return aligned_alloc(alignment, size);
#endif
}

inline void alignedFree(void* ptr) noexcept
{
#ifdef WIN32
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

template<typename T, std::size_t Alignment = sizeof(void*)>
class AlignmentAllocator
{
public:
	using value_type      = T;
	using size_type       = std::size_t;
	using difference_type = std::ptrdiff_t;

	using void_pointer       = void*;
	using const_void_pointer = const void*;

	using pointer       = T*;
	using const_pointer = const T*;

	using reference       = T&;
	using const_reference = const T&;

	template<typename U>
	struct rebind
	{
		using other = AlignmentAllocator<U, Alignment>;
	};

	AlignmentAllocator() = default;

	template<typename U>
	explicit AlignmentAllocator(const AlignmentAllocator<U, Alignment>&) noexcept
	{
	}

	pointer address(reference value) noexcept
	{
		return std::addressof(value);
	}

	const_pointer address(const_reference value) const noexcept
	{
		return std::addressof(value);
	}

	pointer allocate(size_type size, const_void_pointer = 0)
	{
		if (size == 0)
			return nullptr;

		// If the the size is not a multiple of the alignment, increase the size to the next multiple
		if (size % Alignment != 0)
			size += Alignment - (size % Alignment);

		void* p = alignedMalloc(Alignment, size * sizeof(T));

		if (!p)
			throw std::bad_alloc();

		return static_cast<T*>(p);
	}

	void deallocate(pointer ptr, size_type)
	{
		alignedFree(static_cast<void*>(ptr));
	}

	size_type max_size() const noexcept
	{
		return (~static_cast<std::size_t>(0) / sizeof(T));
	}

	template<class U, class... Args>
	void construct(U* ptr, Args&&... args)
	{
		::new (static_cast<void*>(ptr)) U(std::forward<Args>(args)...);
	}

	template<class U>
	void construct(U* ptr)
	{
		::new (static_cast<void*>(ptr)) U();
	}

	template<class U>
	void destroy(U* ptr)
	{
		ptr->~U();
	}
};

template<std::size_t Alignment>
class AlignmentAllocator<void, Alignment>
{
public:
	using pointer       = void*;
	using const_pointer = const void*;
	using value_type    = void;

	template<class U>
	struct rebind
	{
		using other = AlignmentAllocator<U, Alignment>;
	};
};

template<class T, class U, std::size_t Alignment>
inline bool operator==(const AlignmentAllocator<T, Alignment>&, const AlignmentAllocator<U, Alignment>&) noexcept
{
	return true;
}

template<class T, class U, std::size_t Alignment>
inline bool operator!=(const AlignmentAllocator<T, Alignment>&, const AlignmentAllocator<U, Alignment>&) noexcept
{
	return false;
}
} // namespace internal
} // namespace clap
