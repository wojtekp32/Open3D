# ----------------------------------------------------------------------------
# -                        Open3D: www.open3d.org                            -
# ----------------------------------------------------------------------------
# The MIT License (MIT)
#
# Copyright (c) 2018-2021 www.open3d.org
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
# ----------------------------------------------------------------------------

import open3d as o3d
import numpy as np
import pytest


def test_clip_plane():
    cube = o3d.t.geometry.TriangleMesh.from_legacy(
        o3d.geometry.TriangleMesh.create_box())
    clipped_cube = cube.clip_plane(point=[0.5, 0, 0], normal=[1, 0, 0])
    assert clipped_cube.vertex['positions'].shape == (12, 3)
    assert clipped_cube.triangle['indices'].shape == (14, 3)


def test_create_sphere():
    print("hello")
    # Legacy sphere creation
    #
    # In build/ directory, run:
    # ```
    # make install-pip-package -j10
    # pytest ../python/test/t/geometry/test_trianglemesh.py::test_create_sphere -s
    # ```

    # Test legacy
    # sphere = o3d.geometry.TriangleMesh.create_sphere(radius=1)
    # sphere.compute_vertex_normals()
    # o3d.visualization.draw([sphere], raw_mode=True)

    # To implement
    sphere = o3d.t.geometry.TriangleMesh.create_sphere(radius=1)
    o3d.visualization.draw([sphere], raw_mode=True)

    legacy_sphere = sphere.to_legacy()
    legacy_sphere.compute_vertex_normals()
    o3d.visualization.draw([legacy_sphere], raw_mode=True)
