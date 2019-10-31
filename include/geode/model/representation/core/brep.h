/*
 * Copyright (c) 2019 Geode-solutions
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

#pragma once

#include <geode/model/common.h>
#include <geode/model/mixin/core/add_components.h>
#include <geode/model/mixin/core/blocks.h>
#include <geode/model/mixin/core/corners.h>
#include <geode/model/mixin/core/lines.h>
#include <geode/model/mixin/core/model_boundaries.h>
#include <geode/model/mixin/core/surfaces.h>
#include <geode/model/mixin/core/topology.h>

namespace geode
{
    ALIAS_3D( Block );
    ALIAS_3D( ModelBoundary );
    ALIAS_3D( Corner );
    ALIAS_3D( Line );
    ALIAS_3D( Surface );
} // namespace geode

namespace geode
{
    /*!
     * A BRep (for Boundary Representation) is a 3D model composed of
     * Corners, Lines, Surfaces and Blocks.
     * This class provides classes for range-based iteration on Component
     * boundaries and incidences.
     */
    class opengeode_model_api BRep : public Topology,
                                     public AddComponents< 3,
                                         Corners,
                                         Lines,
                                         Surfaces,
                                         Blocks,
                                         ModelBoundaries >
    {
    public:
        class opengeode_model_api LineBoundaryRange
            : public Relationships::BoundaryRangeIterator,
              public BeginEnd< LineBoundaryRange >
        {
        public:
            LineBoundaryRange( const BRep& brep, const Line3D& line );

            const Corner3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api SurfaceBoundaryRange
            : public Relationships::BoundaryRangeIterator,
              public BeginEnd< SurfaceBoundaryRange >
        {
        public:
            SurfaceBoundaryRange( const BRep& brep, const Surface3D& surface );

            const Line3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api BlockBoundaryRange
            : public Relationships::BoundaryRangeIterator,
              public BeginEnd< BlockBoundaryRange >
        {
        public:
            BlockBoundaryRange( const BRep& brep, const Block3D& block );

            const Surface3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api CornerIncidenceRange
            : public Relationships::IncidenceRangeIterator,
              public BeginEnd< CornerIncidenceRange >
        {
        public:
            CornerIncidenceRange( const BRep& brep, const Corner3D& corner );

            const Line3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api LineIncidenceRange
            : public Relationships::IncidenceRangeIterator,
              public BeginEnd< LineIncidenceRange >
        {
        public:
            LineIncidenceRange( const BRep& brep, const Line3D& line );

            const Surface3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api SurfaceIncidenceRange
            : public Relationships::IncidenceRangeIterator,
              public BeginEnd< SurfaceIncidenceRange >
        {
        public:
            SurfaceIncidenceRange( const BRep& brep, const Surface3D& surface );

            const Block3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api SurfaceInternalRange
            : public Relationships::InternalRangeIterator,
              public BeginEnd< SurfaceInternalRange >
        {
        public:
            SurfaceInternalRange( const BRep& brep, const Surface3D& surface );

            const Line3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api BlockInternalRange
            : public Relationships::InternalRangeIterator,
              public BeginEnd< BlockInternalRange >
        {
        public:
            BlockInternalRange( const BRep& brep, const Block3D& block );

            const Surface3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api LineEmbeddingRange
            : public Relationships::EmbeddingRangeIterator,
              public BeginEnd< LineEmbeddingRange >
        {
        public:
            LineEmbeddingRange( const BRep& brep, const Line3D& line );

            const Surface3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api SurfaceEmbeddingRange
            : public Relationships::EmbeddingRangeIterator,
              public BeginEnd< SurfaceEmbeddingRange >
        {
        public:
            SurfaceEmbeddingRange( const BRep& brep, const Surface3D& surface );

            const Block3D& operator*() const;

        private:
            const BRep& brep_;
        };

        class opengeode_model_api ModelBoundaryItemRange
            : public Relationships::ItemRangeIterator,
              public BeginEnd< ModelBoundaryItemRange >
        {
        public:
            ModelBoundaryItemRange(
                const BRep& brep, const ModelBoundary3D& boundary );

            const Surface3D& operator*() const;

        private:
            const BRep& brep_;
        };

    public:
        LineBoundaryRange boundaries( const Line3D& line ) const;

        SurfaceBoundaryRange boundaries( const Surface3D& surface ) const;

        BlockBoundaryRange boundaries( const Block3D& block ) const;

        CornerIncidenceRange incidences( const Corner3D& corner ) const;

        LineIncidenceRange incidences( const Line3D& line ) const;

        SurfaceIncidenceRange incidences( const Surface3D& surface ) const;

        SurfaceInternalRange internals( const Surface3D& surface ) const;

        BlockInternalRange internals( const Block3D& block ) const;

        LineEmbeddingRange embeddings( const Line3D& line ) const;

        SurfaceEmbeddingRange embeddings( const Surface3D& surface ) const;

        ModelBoundaryItemRange items( const ModelBoundary3D& boundary ) const;

        static std::string native_extension_static()
        {
            return "og_brep";
        }

        std::string native_extension() const
        {
            return native_extension_static();
        }
    };
} // namespace geode