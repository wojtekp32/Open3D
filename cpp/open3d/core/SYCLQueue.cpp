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

#include "open3d/core/SYCLQueue.h"

#include <CL/sycl.hpp>
#include <array>
#include <cstdlib>
#include <sstream>

#include "open3d/core/SYCLUtils.h"
#include "open3d/utility/Logging.h"

namespace open3d {
namespace core {
namespace sycl_utils {

sycl::queue SYCLQueue::GetDefaultQueue(const Device &device) {
    auto it = device_to_default_queue_.find(device);
    if (it == device_to_default_queue_.end()) {
        device_to_default_queue_[device] =
                sycl::queue(DeviceToSYCLDevice(device));
    }
    return device_to_default_queue_[device];
}

sycl::device SYCLQueue::DeviceToSYCLDevice(const Device &device) {
    auto it = device_to_sycl_device_.find(device);
    if (it == device_to_sycl_device_.end()) {
        if (!sycl_utils::IsDeviceAvailable(device)) {
            utility::LogError("SYCL Device {} is not available.",
                              device.ToString());
        }
        try {
            return sycl::device(sycl::gpu_selector());
        } catch (const sycl::exception &e) {
            utility::LogError("Failed to create SYCL queue for device: {}.",
                              device.ToString());
        }
    }
    return device_to_sycl_device_[device];
}

SYCLQueue &SYCLQueue::GetInstance() {
    static thread_local SYCLQueue instance;
    return instance;
}

SYCLQueue::SYCLQueue() {}

}  // namespace sycl_utils
}  // namespace core
}  // namespace open3d
