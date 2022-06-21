// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018-2021 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

/// \file SYCLQueue.h
/// \brief SYCL queue manager.
///
/// Unlike from SYCLUtils.h, SYCLQueue.h shall only be included by source files
/// that are compiled with SYCL flags. Other generic source files (e.g.,
/// Device.cpp) shall not include this file.

#pragma once

#include <CL/sycl.hpp>
#include <unordered_map>

#include "open3d/core/Device.h"

namespace open3d {
namespace core {
namespace sycl_utils {

using namespace sycl;

/// Singleton SYCL context manager. It maintains:
/// - A default queue for each SYCL device
class SYCLQueue {
public:
    SYCLQueue(SYCLQueue const&) = delete;
    void operator=(SYCLQueue const&) = delete;
    static SYCLQueue& GetInstance();
    sycl::queue GetDefaultQueue(const Device& device);
    sycl::device DeviceToSYCLDevice(const Device& device);

private:
    SYCLQueue();
    std::unordered_map<Device, sycl::queue> device_to_default_queue_;
    std::unordered_map<Device, sycl::device> device_to_sycl_device_;
};

}  // namespace sycl_utils
}  // namespace core
}  // namespace open3d
