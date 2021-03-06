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

add_geode_library(
    NAME basic
    FOLDER "geode/basic"
    SOURCES
        "assert.cpp"
        "attribute_manager.cpp"
        "bitsery_archive.cpp"
        "common.cpp"
        "filename.cpp"
        "logger.cpp"
        "singleton.cpp"
        "uuid.cpp"
        "zip_file.cpp"
    PUBLIC_HEADERS
        "algorithm.h"
        "assert.h"
        "attribute_manager.h"
        "attribute_utils.h"
        "attribute.h"
        "bitsery_archive.h"
        "common.h"
        "factory.h"
        "filename.h"
        "logger.h"
        "mapping.h"
        "named_type.h"
        "passkey.h"
        "pimpl.h"
        "pimpl_impl.h"
        "range.h"
        "singleton.h"
        "types.h"
        "uuid.h"
        "zip_file.h"
    ADVANCED_HEADERS
        "detail/mapping_after_deletion.h"
    PUBLIC_DEPENDENCIES
        absl::flat_hash_map
        absl::strings
        ghcFilesystem::ghc_filesystem
        Bitsery::bitsery
    PRIVATE_DEPENDENCIES
        spdlog::spdlog_header_only
        MINIZIP::minizip
)
if(WIN32)
    set_target_properties(basic
        PROPERTIES
            LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRTD"
    )
endif()
