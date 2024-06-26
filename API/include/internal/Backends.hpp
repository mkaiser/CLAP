/*
 *  File: Backends.hpp
 *   Copyright (c) 2023 Florian Porrmann
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
 */

#pragma once

#ifdef _WIN32
#include "backends/PCIeBackend.hpp"
#else
#ifdef EMBEDDED_XILINX
#include "backends/BareMetalBackend.hpp"
#else
#include "backends/PCIeBackend.hpp"
#include "backends/PetaLinuxBackend.hpp"
#endif // EMBEDDED_XILINX
#endif // _WIN32

namespace clap
{
#ifdef _WIN32
namespace internal
{
namespace backends
{
using PetaLinuxBackend = PCIeBackend;
} // namespace backends
} // namespace internal
#endif

// TODO: This also adds the Interrupt handler to the backend namespace, bad idea
namespace backends = internal::backends;
} // namespace clap
