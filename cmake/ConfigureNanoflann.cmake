# Copyright (c) 2019 - 2020 Geode-solutions
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
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set(NANOFLANN_PATH ${PROJECT_BINARY_DIR}/third_party/nanoflann)
set(NANOFLANN_INSTALL_PREFIX ${NANOFLANN_PATH}/install)
ExternalProject_Add(nanoflann
    PREFIX ${NANOFLANN_PATH}
    GIT_REPOSITORY https://github.com/jlblancoc/nanoflann
    GIT_TAG 3b2065ebb7386f249a8d48865662b6fb2e960985
    GIT_PROGRESS ON
    CMAKE_GENERATOR ${CMAKE_GENERATOR}
    CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
    CMAKE_ARGS
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_MESSAGE=LAZY
    CMAKE_CACHE_ARGS
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_TESTS:BOOL=OFF
        -DCMAKE_INSTALL_PREFIX:PATH=${NANOFLANN_INSTALL_PREFIX}
)
