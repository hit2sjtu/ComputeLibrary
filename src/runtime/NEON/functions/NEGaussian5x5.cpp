/*
 * Copyright (c) 2016, 2017 ARM Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/runtime/NEON/functions/NEGaussian5x5.h"

#include "arm_compute/core/ITensor.h"
#include "arm_compute/core/NEON/kernels/NEGaussian5x5Kernel.h"
#include "arm_compute/core/PixelValue.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/runtime/NEON/NEScheduler.h"
#include "arm_compute/runtime/TensorAllocator.h"

using namespace arm_compute;

NEGaussian5x5::NEGaussian5x5()
    : _kernel_hor(), _kernel_vert(), _tmp(), _border_handler()
{
}

void NEGaussian5x5::configure(ITensor *input, ITensor *output, BorderMode border_mode, uint8_t constant_border_value)
{
    // Init temporary buffer
    TensorInfo tensor_info(input->info()->tensor_shape(), Format::S16);
    _tmp.allocator()->init(tensor_info);

    // Create and configure kernels for the two passes
    _kernel_hor.configure(input, &_tmp, border_mode == BorderMode::UNDEFINED);
    _kernel_vert.configure(&_tmp, output, border_mode == BorderMode::UNDEFINED);

    _tmp.allocator()->allocate();

    _border_handler.configure(input, _kernel_hor.border_size(), border_mode, PixelValue(constant_border_value));
}

void NEGaussian5x5::run()
{
    _border_handler.run(_border_handler.window());
    NEScheduler::get().multithread(&_kernel_hor);
    NEScheduler::get().multithread(&_kernel_vert);
}
