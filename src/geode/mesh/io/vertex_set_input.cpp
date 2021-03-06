/*
 * Copyright (c) 2019 - 2020 Geode-solutions
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <geode/mesh/io/vertex_set_input.h>

#include <fstream>

#include <geode/mesh/core/bitsery_archive.h>
#include <geode/mesh/core/mesh_factory.h>
#include <geode/mesh/core/vertex_set.h>

namespace geode
{
    std::unique_ptr< VertexSet > load_vertex_set( absl::string_view filename )
    {
        return load_vertex_set(
            MeshFactory::default_impl( VertexSet::type_name_static() ),
            filename );
    }

    std::unique_ptr< VertexSet > load_vertex_set(
        const MeshImpl& impl, absl::string_view filename )
    {
        try
        {
            auto vertex_set = VertexSet::create( impl );
            auto input = VertexSetInputFactory::create(
                extension_from_filename( filename ).data(), *vertex_set,
                filename );
            input->read();
            return vertex_set;
        }
        catch( const OpenGeodeException& e )
        {
            Logger::error( e.what() );
            throw OpenGeodeException{ "Cannot load VertexSet from file: ",
                filename };
        }
    }

    VertexSetInput::VertexSetInput(
        VertexSet& vertex_set, absl::string_view filename )
        : Input( filename ), vertex_set_( vertex_set )
    {
    }

    void VertexSetInput::read()
    {
        check_emptiness();
        do_read();
    }

    void VertexSetInput::check_emptiness()
    {
        OPENGEODE_EXCEPTION( vertex_set_.nb_vertices() == 0,
            "[VertexSetInput::check_emptiness] "
            "The given mesh should be empty." );
    }
} // namespace geode
