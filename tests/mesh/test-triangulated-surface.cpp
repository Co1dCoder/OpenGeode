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

#include <geode/basic/attribute_manager.h>
#include <geode/basic/logger.h>

#include <geode/geometry/point.h>

#include <geode/mesh/builder/geode_triangulated_surface_builder.h>
#include <geode/mesh/core/geode_triangulated_surface.h>
#include <geode/mesh/io/triangulated_surface_input.h>
#include <geode/mesh/io/triangulated_surface_output.h>

#include <geode/tests/common.h>

void test_create_vertices( const geode::TriangulatedSurface3D& surface,
    geode::TriangulatedSurfaceBuilder3D& builder )
{
    builder.create_point( { { 0.1, 0.2, 0.3 } } );
    builder.create_point( { { 2.1, 9.4, 6.7 } } );
    builder.create_point( { { 7.5, 5.2, 6.3 } } );
    builder.create_point( { { 8.1, 1.4, 4.7 } } );
    builder.create_point( { { 4.7, 2.1, 1.3 } } );
    OPENGEODE_EXCEPTION( surface.nb_vertices() == 5,
        "[Test] TriangulatedSurface should have 5 vertices" );
}

void test_create_polygons( const geode::TriangulatedSurface3D& surface,
    geode::TriangulatedSurfaceBuilder3D& builder )
{
    builder.create_triangles( 1 );
    builder.create_triangle( { 1, 3, 2 } );
    builder.create_polygon( { 3, 4, 2 } );
    OPENGEODE_EXCEPTION( surface.nb_polygons() == 3,
        "[Test] TriangulatedSurface should have 3 triangles" );
    builder.set_polygon_vertex( { 0, 0 }, 0 );
    builder.set_polygon_vertex( { 0, 1 }, 1 );
    builder.set_polygon_vertex( { 0, 2 }, 2 );
    builder.delete_isolated_edges();
    OPENGEODE_EXCEPTION( surface.nb_edges() == 7,
        "[Test] TriangulatedSurface should have 7 edges" );
}

void test_polygon_adjacencies( const geode::TriangulatedSurface3D& surface,
    geode::TriangulatedSurfaceBuilder3D& builder )
{
    builder.compute_polygon_adjacencies();
    OPENGEODE_EXCEPTION( !surface.polygon_adjacent( { 0, 0 } ),
        "[Test] TriangulatedSurface adjacent index is not correct" );
    OPENGEODE_EXCEPTION( surface.polygon_adjacent( { 0, 1 } ) == 1,
        "[Test] TriangulatedSurface adjacent index is not correct" );
    OPENGEODE_EXCEPTION( surface.polygon_adjacent( { 1, 2 } ) == 0,
        "[Test] TriangulatedSurface adjacent index is not correct" );
    OPENGEODE_EXCEPTION(
        surface.polygon_adjacent_edge( { 0, 1 } ) == geode::PolygonEdge( 1, 2 ),
        "[Test] TriangulatedSurface adjacent index is not correct" );

    OPENGEODE_EXCEPTION( surface.polygon_adjacent( { 2, 2 } ) == 1,
        "[Test] TriangulatedSurface adjacent index is not correct" );
    OPENGEODE_EXCEPTION( !surface.polygon_adjacent( { 2, 0 } ),
        "[Test] TriangulatedSurface adjacent index is not correct" );
}

void test_delete_vertex( const geode::TriangulatedSurface3D& surface,
    geode::TriangulatedSurfaceBuilder3D& builder )
{
    std::vector< bool > to_delete( surface.nb_vertices(), false );
    to_delete.front() = true;
    builder.delete_vertices( to_delete );
    OPENGEODE_EXCEPTION( surface.nb_vertices() == 4,
        "[Test] TriangulatedSurface should have 4 vertices" );
    const geode::Point3D answer{ { 2.1, 9.4, 6.7 } };
    OPENGEODE_EXCEPTION( surface.point( 0 ) == answer,
        "[Test] TriangulatedSurface vertex coordinates are not correct" );
    OPENGEODE_EXCEPTION( surface.nb_polygons() == 2,
        "[Test] TriangulatedSurface should have 2 polygons" );
    OPENGEODE_EXCEPTION( surface.polygon_adjacent( { 1, 2 } ) == 0,
        "[Test] TriangulatedSurface adjacent index is not correct" );
    OPENGEODE_EXCEPTION( surface.nb_edges() == 5,
        "[Test] TriangulatedSurface should have 5 edges" );
}

void test_delete_polygon( const geode::TriangulatedSurface3D& surface,
    geode::TriangulatedSurfaceBuilder3D& builder )
{
    std::vector< bool > to_delete( surface.nb_polygons(), false );
    to_delete.front() = true;
    builder.delete_polygons( to_delete );
    OPENGEODE_EXCEPTION( surface.nb_polygons() == 1,
        "[Test] TriangulatedSurface should have 1 polygon" );
    OPENGEODE_EXCEPTION( surface.polygon_vertex( { 0, 0 } ) == 2,
        "[Test] TriangulatedSurface edge vertex index is not correct" );
    OPENGEODE_EXCEPTION( surface.polygon_vertex( { 0, 1 } ) == 3,
        "[Test] TriangulatedSurface edge vertex index is not correct" );
    OPENGEODE_EXCEPTION( surface.polygon_vertex( { 0, 2 } ) == 1,
        "[Test] TriangulatedSurface edge vertex index is not correct" );
    OPENGEODE_EXCEPTION( surface.nb_edges() == 3,
        "[Test] TriangulatedSurface should have 3 edges" );
}

void test_io(
    const geode::TriangulatedSurface3D& surface, absl::string_view filename )
{
    geode::save_triangulated_surface( surface, filename );
    geode::load_triangulated_surface< 3 >( filename );
    geode::load_triangulated_surface< 3 >(
        geode::OpenGeodeTriangulatedSurface3D::impl_name_static(), filename );
}

void test_clone( const geode::TriangulatedSurface3D& surface )
{
    auto attr_from = surface.edge_attribute_manager()
                         .find_or_create_attribute< geode::VariableAttribute,
                             geode::index_t >( "edge_id", 0 );
    for( const auto e : geode::Range{ surface.nb_edges() } )
    {
        attr_from->set_value( e, e );
    }
    auto surface2 = surface.clone();
    OPENGEODE_EXCEPTION( surface2->nb_vertices() == 4,
        "[Test] TriangulatedSurface2 should have 4 vertices" );
    OPENGEODE_EXCEPTION( surface2->nb_edges() == 3,
        "[Test] TriangulatedSurface2 should have 3 edges" );
    OPENGEODE_EXCEPTION( surface2->nb_polygons() == 1,
        "[Test] TriangulatedSurface2 should have 1 polygon" );
    auto attr_to =
        surface2->edge_attribute_manager().find_attribute< geode::index_t >(
            "edge_id" );
    for( const auto e : geode::Range{ surface.nb_edges() } )
    {
        OPENGEODE_EXCEPTION( attr_from->value( e ) == attr_to->value( e ),
            "[Test] Error in edge attribute transfer during cloning" );
    }
}

void test_delete_all( const geode::TriangulatedSurface3D& triangulated_surface,
    geode::TriangulatedSurfaceBuilder3D& builder )
{
    builder.delete_isolated_vertices();
    OPENGEODE_EXCEPTION( triangulated_surface.nb_vertices() == 3,
        "[Test]TriangulatedSurface should have 3 vertices" );
    OPENGEODE_EXCEPTION( triangulated_surface.nb_edges() == 3,
        "[Test]TriangulatedSurface should have 3 edges" );
    OPENGEODE_EXCEPTION( triangulated_surface.nb_polygons() == 1,
        "[Test]TriangulatedSurface should have 1 polygon" );

    std::vector< bool > to_delete( triangulated_surface.nb_polygons(), true );
    builder.delete_polygons( to_delete );
    OPENGEODE_EXCEPTION( triangulated_surface.nb_vertices() == 3,
        "[Test]TriangulatedSurface should have 3 vertices" );
    OPENGEODE_EXCEPTION( triangulated_surface.nb_edges() == 0,
        "[Test]TriangulatedSurface should have 0 edge" );
    OPENGEODE_EXCEPTION( triangulated_surface.nb_polygons() == 0,
        "[Test]TriangulatedSurface should have 0 polygon" );
    OPENGEODE_EXCEPTION(
        triangulated_surface.polygons_around_vertex( 0 ).empty(),
        "[Test] No more polygon around vertices" );

    builder.delete_isolated_vertices();
    OPENGEODE_EXCEPTION( triangulated_surface.nb_vertices() == 0,
        "[Test]TriangulatedSurface should have 0 vertex" );
}

void test_backward_io( const std::string& filename )
{
    const auto new_triangulated_surface = geode::load_triangulated_surface< 3 >(
        geode::OpenGeodeTriangulatedSurface3D::impl_name_static(), filename );

    OPENGEODE_EXCEPTION( new_triangulated_surface->nb_vertices() == 5,
        "[Test] Reloaded TriangulatedSurface should have 5 vertices" );
    OPENGEODE_EXCEPTION( new_triangulated_surface->nb_edges() == 7,
        "[Test] Reloaded TriangulatedSurface should have 7 edges" );
    OPENGEODE_EXCEPTION( new_triangulated_surface->nb_polygons() == 3,
        "[Test] Reloaded TriangulatedSurface should have 3 polygons" );
}

void test()
{
    auto surface = geode::TriangulatedSurface3D::create(
        geode::OpenGeodeTriangulatedSurface3D::impl_name_static() );
    auto builder = geode::TriangulatedSurfaceBuilder3D::create( *surface );

    test_create_vertices( *surface, *builder );
    test_create_polygons( *surface, *builder );
    test_polygon_adjacencies( *surface, *builder );
    test_io( *surface, absl::StrCat( "test.", surface->native_extension() ) );
    test_backward_io( absl::StrCat(
        geode::data_path, "/test_v4.", surface->native_extension() ) );

    test_delete_vertex( *surface, *builder );
    test_delete_polygon( *surface, *builder );
    test_clone( *surface );
}

OPENGEODE_TEST( "triangulated-surface" )