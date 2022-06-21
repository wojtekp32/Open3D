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

// Contains code from:
// https://github.com/intel/llvm/blob/sycl/sycl/tools/sycl-ls/sycl-ls.cpp
// ----------------------------------------------------------------------------
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// ----------------------------------------------------------------------------

#include "open3d/core/SYCLUtils.h"

#ifdef BUILD_SYCL_MODULE
#include <CL/sycl.hpp>
#endif

#include <array>
#include <cstdlib>
#include <sstream>

#include "open3d/utility/Logging.h"

namespace open3d {
namespace core {
namespace sycl_utils {

using namespace cl;

class SYCLContext {
public:
    SYCLContext(SYCLContext const &) = delete;
    void operator=(SYCLContext const &) = delete;
    static SYCLContext &GetInstance() {
        static thread_local SYCLContext instance;
        return instance;
    }
    static bool IsAvailable() {
#ifdef BUILD_SYCL_MODULE
        return GetAvailableSYCLDevices().size() > 0;
#else
        return false;
#endif
    }
    static bool IsDeviceAvailable(const Device &device) {
        bool rc = false;
        for (const Device &device_ : GetInstance().devices_) {
            if (device == device_) {
                rc = true;
                break;
            }
        }
        return rc;
    }
    static std::vector<Device> GetAvailableSYCLDevices() {
        return GetInstance().devices_;
    }

private:
    SYCLContext() {
#ifdef BUILD_SYCL_MODULE
        // This function will ony be called once as SYCLContext is a singleton.
        try {
            const sycl::device &device = sycl::device(sycl::gpu_selector());
            devices_.push_back(Device("SYCL:0"));
        } catch (const sycl::exception &e) {
        }
        // Fall back to SYCL host device if SYCL GPU device is not available.
        // This could happen if the Intel GPGPU driver is not installed.
        try {
            const sycl::device &device = sycl::device(sycl::host_selector());
            devices_.push_back(Device("SYCL:0"));
            utility::LogWarning(
                    "SYCL GPU device is not available, falling back to SYCL "
                    "host device. Typically, SYCL host is only used for "
                    "debugging.");
        } catch (const sycl::exception &e) {
        }
#else
        devices_ = {};
#endif
    }
    std::vector<Device> devices_;
};

int SYCLDemo() {
#ifdef BUILD_SYCL_MODULE
    // Ref: https://intel.github.io/llvm-docs/GetStartedGuide.html
    // Creating buffer of 4 ints to be used inside the kernel code.
    sycl::buffer<sycl::cl_int, 1> buffer(4);

    // Creating SYCL queue.
    sycl::queue q;

    // Size of index space for kernel.
    sycl::range<1> num_workloads{buffer.size()};

    // Submitting command group(work) to q.
    q.submit([&](sycl::handler &cgh) {
        // Getting write only access to the buffer on a device.
        auto accessor = buffer.get_access<sycl::access::mode::write>(cgh);
        // Execute kernel.
        cgh.parallel_for<class FillBuffer>(
                num_workloads, [=](sycl::id<1> WIid) {
                    // Fill buffer with indexes.
                    accessor[WIid] = (sycl::cl_int)WIid.get(0);
                });
    });

    // Getting read only access to the buffer on the host.
    // Implicit barrier waiting for q to complete the work.
    const auto host_accessor = buffer.get_access<sycl::access::mode::read>();

    // Check the results.
    bool mismatch_found = false;
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (host_accessor[i] != i) {
            utility::LogInfo("Mismatch found at index {}: expected {}, got {}.",
                             i, i, host_accessor[i]);
            mismatch_found = true;
        }
    }

    if (mismatch_found) {
        utility::LogInfo("SYCLDemo failed!");
        return -1;
    } else {
        utility::LogInfo("SYCLDemo passed!");
        return 0;
    }
#else
    utility::LogInfo("SYCLDemo is not compiled with BUILD_SYCL_MODULE=ON.");
    return -1;
#endif
}

#ifdef BUILD_SYCL_MODULE

static std::string GetDeviceTypeName(const sycl::device &device) {
    auto device_type = device.get_info<sycl::info::device::device_type>();
    switch (device_type) {
        case sycl::info::device_type::cpu:
            return "cpu";
        case sycl::info::device_type::gpu:
            return "gpu";
        case sycl::info::device_type::host:
            return "host";
        case sycl::info::device_type::accelerator:
            return "acc";
        default:
            return "unknown";
    }
}

static std::string GetBackendName(const sycl::device &device) {
    sycl::platform platform = device.get_info<sycl::info::device::platform>();
    sycl::backend backend = platform.get_backend();
    std::ostringstream os;
    os << backend;
    return os.str();
}

static std::string SYCLDeviceToString(const sycl::device &device) {
    std::ostringstream os;
    os << "[" << GetBackendName(device) << ":" << GetDeviceTypeName(device)
       << "] " << device.get_info<sycl::info::device::name>();
    return os.str();
}
#endif

void PrintSYCLDevices(bool print_all) {
#ifdef BUILD_SYCL_MODULE
    const char *filter = std::getenv("SYCL_DEVICE_FILTER");
    if (filter) {
        utility::LogWarning(
                "SYCL_DEVICE_FILTER environment variable is set to {}. To see "
                "the correct device id, please unset SYCL_DEVICE_FILTER.",
                filter);
    }

    if (print_all) {
        utility::LogInfo("# All SYCL devices");
        const std::vector<sycl::platform> &platforms =
                sycl::platform::get_platforms();
        for (const sycl::platform &platform : platforms) {
            sycl::backend backend = platform.get_backend();
            const std::vector<sycl::device> &devices = platform.get_devices();
            for (const sycl::device &device : devices) {
                utility::LogInfo("- {}", SYCLDeviceToString(device));
            }
        }

        utility::LogInfo("# Default SYCL selectors");
        try {
            const sycl::device &device = sycl::device(sycl::default_selector());
            utility::LogInfo("- sycl::default_selector()    : {}",
                             SYCLDeviceToString(device));
        } catch (const sycl::exception &e) {
            utility::LogInfo("- sycl::default_selector()    : N/A");
        }
        try {
            const sycl::device &device = sycl::device(sycl::host_selector());
            utility::LogInfo("- sycl::host_selector()       : {}",
                             SYCLDeviceToString(device));
        } catch (const sycl::exception &e) {
            utility::LogInfo("- sycl::host_selector()       : N/A");
        }
        try {
            const sycl::device &device = sycl::device(sycl::cpu_selector());
            utility::LogInfo("- sycl::cpu_selector()        : {}",
                             SYCLDeviceToString(device));
        } catch (const sycl::exception &e) {
            utility::LogInfo("- sycl::cpu_selector()        : N/A");
        }
        try {
            const sycl::device &device = sycl::device(sycl::gpu_selector());
            utility::LogInfo("- sycl::gpu_selector()        : {}",
                             SYCLDeviceToString(device));
        } catch (const sycl::exception &e) {
            utility::LogInfo("- sycl::gpu_selector()        : N/A");
        }
        try {
            const sycl::device &device =
                    sycl::device(sycl::accelerator_selector());
            utility::LogInfo("- sycl::accelerator_selector(): {}",
                             SYCLDeviceToString(device));
        } catch (const sycl::exception &e) {
            utility::LogInfo("- sycl::accelerator_selector(): N/A");
        }

        utility::LogInfo("# Open3D SYCL device");
        try {
            const sycl::device &device = sycl::device(sycl::gpu_selector());
            utility::LogInfo("- Device(\"SYCL:0\"): {}",
                             SYCLDeviceToString(device));
        } catch (const sycl::exception &e) {
            utility::LogInfo("- Device(\"SYCL:0\"): N/A");
        }
    } else {
        utility::LogInfo("# Open3D SYCL device");
        try {
            const sycl::device &device = sycl::device(sycl::gpu_selector());
            utility::LogInfo("- Device(\"SYCL:0\"): {}",
                             SYCLDeviceToString(device));
        } catch (const sycl::exception &e) {
            utility::LogInfo("- Device(\"SYCL:0\"): N/A");
        }
    }

#else
    utility::LogInfo(
            "PrintSYCLDevices is not compiled with BUILD_SYCL_MODULE=ON.");
#endif
}

bool IsAvailable() { return SYCLContext::GetInstance().IsAvailable(); }

bool IsDeviceAvailable(const Device &device) {
    return SYCLContext::GetInstance().IsDeviceAvailable(device);
}

std::vector<Device> GetAvailableSYCLDevices() {
    return SYCLContext::GetInstance().GetAvailableSYCLDevices();
}

}  // namespace sycl_utils
}  // namespace core
}  // namespace open3d
