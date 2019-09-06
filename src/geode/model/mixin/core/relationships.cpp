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

#include <geode/model/mixin/core/relationships.h>

#include <fstream>

#include <geode/basic/attribute.h>
#include <geode/basic/attribute_manager.h>
#include <geode/basic/bitsery_archive.h>
#include <geode/basic/pimpl_impl.h>
#include <geode/basic/uuid.h>

#include <geode/model/mixin/core/bitsery_archive.h>
#include <geode/model/mixin/core/detail/uuid_to_index.h>

#include <geode/mesh/builder/geode_graph_builder.h>
#include <geode/mesh/core/bitsery_archive.h>
#include <geode/mesh/core/geode_graph.h>
#include <geode/mesh/io/graph_input.h>
#include <geode/mesh/io/graph_output.h>

namespace geode
{
    class Relationships::Impl
    {
    public:
        using Iterator = typename std::vector< EdgeVertex >::const_iterator;
        using RelationType = index_t;
        static const index_t BOUNDARY_RELATION = index_t( 0 );
        static const index_t ITEM_RELATION = index_t( 1 );
        static const index_t BOUNDARY_EDGE_VERTEX = index_t( 0 );
        static const index_t INCIDENT_EDGE_VERTEX = index_t( 1 );
        static const index_t ITEM_EDGE_VERTEX = index_t( 0 );
        static const index_t COLLECTION_EDGE_VERTEX = index_t( 1 );

        Impl()
            : relation_type_(
                  graph_.edge_attribute_manager()
                      .find_or_create_attribute< VariableAttribute,
                          RelationType >( "relation_type", NO_ID ) ),
              uuids_( graph_.vertex_attribute_manager()
                          .find_or_create_attribute< VariableAttribute, uuid >(
                              "uuid", uuid{} ) )
        {
        }

        RelationType relation_type( index_t edge_id ) const
        {
            return relation_type_->value( edge_id );
        }

        bool is_boundary_relation( index_t edge_id ) const
        {
            return relation_type_->value( edge_id ) == BOUNDARY_RELATION;
        }

        bool is_item_relation( index_t edge_id ) const
        {
            return relation_type_->value( edge_id ) == ITEM_RELATION;
        }

        Iterator begin_edge( index_t id ) const
        {
            return graph_.edges_around_vertex( id ).begin();
        }

        Iterator end_edge( index_t id ) const
        {
            return graph_.edges_around_vertex( id ).end();
        }

        Iterator begin_edge( const uuid& id ) const
        {
            return begin_edge( vertex_id( id ) );
        }

        Iterator end_edge( const uuid& id ) const
        {
            return end_edge( vertex_id( id ) );
        }

        const uuid& vertex_uuid( const EdgeVertex& edge_vertex ) const
        {
            return uuids_->value( graph_.edge_vertex( edge_vertex ) );
        }

        void add_component( const uuid& id )
        {
            auto index = OpenGeodeGraphBuilder{ graph_ }.create_vertex();
            uuid2index_.set_new_mapping( id, index );
            uuids_->value( index ) = id;
        }

        void remove_component( const uuid& id )
        {
            auto index = vertex_id( id );
            std::vector< bool > to_delete( graph_.nb_vertices(), false );
            to_delete[index] = true;
            OpenGeodeGraphBuilder{ graph_ }.delete_vertices( to_delete );
            uuid2index_.erase( id );
            uuid2index_.decrement_indices_larger_than( index );
        }

        bool check_relation_exists(
            const uuid& from, const uuid& to, RelationType type )
        {
            const auto& edges_around =
                graph_.edges_around_vertex( vertex_id( from ) );
            for( const auto& edge_vertex : edges_around )
            {
                const auto& other = this->vertex_uuid( { edge_vertex.edge_id,
                    ( edge_vertex.vertex_id + 1 ) % 2 } );
                if( to == other
                    && relation_type( edge_vertex.edge_id ) == type )
                {
                    return true;
                }
            }
            return false;
        }

        void add_relation( const uuid& from, const uuid& to, RelationType type )
        {
            if( check_relation_exists( from, to, type ) )
            {
                Logger::warn( "Relation already exists" );
                return;
            }

            auto index = OpenGeodeGraphBuilder{ graph_ }.create_edge(
                vertex_id( from ), vertex_id( to ) );

            relation_type_->value( index ) = type;
        }

        std::string save( const std::string& directory ) const
        {
            auto filename = directory + "/relationships";
            std::ofstream file{ filename, std::ofstream::binary };
            TContext context{};
            register_basic_serialize_pcontext( std::get< 0 >( context ) );
            register_mesh_serialize_pcontext( std::get< 0 >( context ) );
            register_georepresentation_serialize_pcontext(
                std::get< 0 >( context ) );
            Serializer archive{ context, file };
            archive.object( *this );
            archive.adapter().flush();
            OPENGEODE_EXCEPTION( std::get< 1 >( context ).isValid(),
                "Error while writing file: " + filename );
            return filename;
        }

        void load( const std::string& directory )
        {
            auto filename = directory + "/relationships";
            std::ifstream file{ filename, std::ifstream::binary };
            TContext context{};
            register_basic_deserialize_pcontext( std::get< 0 >( context ) );
            register_mesh_deserialize_pcontext( std::get< 0 >( context ) );
            register_georepresentation_deserialize_pcontext(
                std::get< 0 >( context ) );
            Deserializer archive{ context, file };
            archive.object( *this );
            auto& adapter = archive.adapter();
            OPENGEODE_EXCEPTION(
                adapter.error() == bitsery::ReaderError::NoError
                    && adapter.isCompletedSuccessfully()
                    && std::get< 1 >( context ).isValid(),
                "Error while reading file: " + filename );
        }

    private:
        friend class bitsery::Access;
        template < typename Archive >
        void serialize( Archive& archive )
        {
            archive.object( graph_ );
            archive.object( uuid2index_ );
            archive.ext( relation_type_, bitsery::ext::StdSmartPtr{} );
            archive.ext( uuids_, bitsery::ext::StdSmartPtr{} );
        }

        index_t vertex_id( const uuid& id ) const
        {
            return uuid2index_.index( id );
        }

    private:
        OpenGeodeGraph graph_;
        detail::UuidToIndex uuid2index_;
        std::shared_ptr< VariableAttribute< RelationType > > relation_type_;
        std::shared_ptr< VariableAttribute< uuid > > uuids_;
    };

    Relationships::Relationships() {} // NOLINT
    Relationships::~Relationships() {} // NOLINT

    void Relationships::add_component( const uuid& id )
    {
        impl_->add_component( id );
    }

    void Relationships::remove_component( const uuid& id )
    {
        impl_->remove_component( id );
    }

    index_t Relationships::nb_boundaries( const uuid& id ) const
    {
        index_t nb{ 0 };
        for( const auto& unused : boundaries( id ) )
        {
            geode_unused( unused );
            nb++;
        }
        return nb;
    }

    Relationships::BoundaryRange Relationships::boundaries(
        const uuid& id ) const
    {
        return { *this, id };
    }

    index_t Relationships::nb_incidences( const uuid& id ) const
    {
        index_t nb{ 0 };
        for( const auto& unused : incidences( id ) )
        {
            geode_unused( unused );
            nb++;
        }
        return nb;
    }

    Relationships::IncidenceRange Relationships::incidences(
        const uuid& id ) const
    {
        return { *this, id };
    }

    void Relationships::add_boundary_relation(
        const uuid& boundary, const uuid& incidence )
    {
        impl_->add_relation(
            boundary, incidence, Relationships::Impl::BOUNDARY_RELATION );
    }

    index_t Relationships::nb_items( const uuid& id ) const
    {
        index_t nb{ 0 };
        for( const auto& unused : items( id ) )
        {
            geode_unused( unused );
            nb++;
        }
        return nb;
    }

    Relationships::ItemRange Relationships::items( const uuid& id ) const
    {
        return { *this, id };
    }

    index_t Relationships::nb_collections( const uuid& id ) const
    {
        index_t nb{ 0 };
        for( const auto& unused : collections( id ) )
        {
            geode_unused( unused );
            nb++;
        }
        return nb;
    }

    Relationships::CollectionRange Relationships::collections(
        const uuid& id ) const
    {
        return { *this, id };
    }

    void Relationships::add_item_in_collection(
        const uuid& item, const uuid& collection )
    {
        impl_->add_relation(
            item, collection, Relationships::Impl::ITEM_RELATION );
    }

    std::string Relationships::save_relationships(
        const std::string& directory ) const
    {
        return impl_->save( directory );
    }

    void Relationships::load_relationships( const std::string& directory )
    {
        return impl_->load( directory );
    }

    class Relationships::BoundaryRange::Impl
        : public BaseRange< typename Relationships::Impl::Iterator >
    {
        using Iterator = typename Relationships::Impl::Iterator;

    public:
        Impl( const Relationships::Impl& manager, Iterator begin, Iterator end )
            : BaseRange< Iterator >( begin, end ), manager_( manager )
        {
            next_boundary_iterator();
        }

        void next()
        {
            this->operator++();
            next_boundary_iterator();
        }

        const uuid& vertex_uuid() const
        {
            const auto iterator = this->current();
            return manager_.vertex_uuid(
                { iterator->edge_id, ( iterator->vertex_id + 1 ) % 2 } );
        }

    private:
        bool is_boundary_edge_vertex()
        {
            return this->current()->vertex_id
                   == Relationships::Impl::BOUNDARY_EDGE_VERTEX;
        }

        void next_boundary_iterator()
        {
            while(
                this->operator!=( *this )
                && ( !manager_.is_boundary_relation( this->current()->edge_id )
                       || is_boundary_edge_vertex() ) )
            {
                this->operator++();
            }
        }

    private:
        const Relationships::Impl& manager_;
    };

    Relationships::BoundaryRange::BoundaryRange(
        const Relationships& manager, const uuid& id )
        : impl_( *manager.impl_,
              manager.impl_->begin_edge( id ),
              manager.impl_->end_edge( id ) )
    {
    }

    Relationships::BoundaryRange::BoundaryRange(
        BoundaryRange&& other ) noexcept
        : impl_( *other.impl_ )
    {
    }

    Relationships::BoundaryRange::BoundaryRange( const BoundaryRange& other )
        : impl_( *other.impl_ )
    {
    }

    Relationships::BoundaryRange::~BoundaryRange() {} // NOLINT

    bool Relationships::BoundaryRange::operator!=(
        const BoundaryRange& /*unused*/ ) const
    {
        return impl_->operator!=( *impl_ );
    }

    void Relationships::BoundaryRange::operator++()
    {
        return impl_->next();
    }

    const uuid& Relationships::BoundaryRange::operator*() const
    {
        return impl_->vertex_uuid();
    }

    class Relationships::IncidenceRange::Impl
        : public BaseRange< typename Relationships::Impl::Iterator >
    {
        using Iterator = typename Relationships::Impl::Iterator;

    public:
        Impl( const Relationships::Impl& manager, Iterator begin, Iterator end )
            : BaseRange< Iterator >( begin, end ), manager_( manager )
        {
            next_incidence_iterator();
        }

        void next()
        {
            this->operator++();
            next_incidence_iterator();
        }

        const uuid& vertex_uuid() const
        {
            const auto iterator = this->current();
            return manager_.vertex_uuid(
                { iterator->edge_id, ( iterator->vertex_id + 1 ) % 2 } );
        }

    private:
        bool is_incident_edge_vertex()
        {
            return this->current()->vertex_id
                   == Relationships::Impl::INCIDENT_EDGE_VERTEX;
        }

        void next_incidence_iterator()
        {
            while(
                this->operator!=( *this )
                && ( !manager_.is_boundary_relation( this->current()->edge_id )
                       || is_incident_edge_vertex() ) )
            {
                this->operator++();
            }
        }

    private:
        const Relationships::Impl& manager_;
    };

    Relationships::IncidenceRange::IncidenceRange(
        const Relationships& manager, const uuid& id )
        : impl_( *manager.impl_,
              manager.impl_->begin_edge( id ),
              manager.impl_->end_edge( id ) )
    {
    }

    Relationships::IncidenceRange::IncidenceRange(
        IncidenceRange&& other ) noexcept
        : impl_( *other.impl_ )
    {
    }

    Relationships::IncidenceRange::IncidenceRange( const IncidenceRange& other )
        : impl_( *other.impl_ )
    {
    }

    Relationships::IncidenceRange::~IncidenceRange() {} // NOLINT

    bool Relationships::IncidenceRange::operator!=(
        const IncidenceRange& /*unused*/ ) const
    {
        return impl_->operator!=( *impl_ );
    }

    void Relationships::IncidenceRange::operator++()
    {
        return impl_->next();
    }

    const uuid& Relationships::IncidenceRange::operator*() const
    {
        return impl_->vertex_uuid();
    }

    class Relationships::ItemRange::Impl
        : public BaseRange< typename Relationships::Impl::Iterator >
    {
        using Iterator = typename Relationships::Impl::Iterator;

    public:
        Impl( const Relationships::Impl& manager, Iterator begin, Iterator end )
            : BaseRange< Iterator >( begin, end ), manager_( manager )
        {
            next_item_iterator();
        }

        void next()
        {
            this->operator++();
            next_item_iterator();
        }

        const uuid& vertex_uuid() const
        {
            const auto iterator = this->current();
            return manager_.vertex_uuid(
                { iterator->edge_id, ( iterator->vertex_id + 1 ) % 2 } );
        }

    private:
        bool is_item_edge_vertex()
        {
            return this->current()->vertex_id
                   == Relationships::Impl::ITEM_EDGE_VERTEX;
        }

        void next_item_iterator()
        {
            while( this->operator!=( *this )
                   && ( !manager_.is_item_relation( this->current()->edge_id )
                          || is_item_edge_vertex() ) )
            {
                this->operator++();
            }
        }

    private:
        const Relationships::Impl& manager_;
    };

    Relationships::ItemRange::ItemRange(
        const Relationships& manager, const uuid& id )
        : impl_( *manager.impl_,
              manager.impl_->begin_edge( id ),
              manager.impl_->end_edge( id ) )
    {
    }

    Relationships::ItemRange::ItemRange( ItemRange&& other ) noexcept
        : impl_( *other.impl_ )
    {
    }

    Relationships::ItemRange::ItemRange( const ItemRange& other )
        : impl_( *other.impl_ )
    {
    }

    Relationships::ItemRange::~ItemRange() {} // NOLINT

    bool Relationships::ItemRange::operator!=(
        const ItemRange& /*unused*/ ) const
    {
        return impl_->operator!=( *impl_ );
    }

    void Relationships::ItemRange::operator++()
    {
        return impl_->next();
    }

    const uuid& Relationships::ItemRange::operator*() const
    {
        return impl_->vertex_uuid();
    }

    class Relationships::CollectionRange::Impl
        : public BaseRange< typename Relationships::Impl::Iterator >
    {
        using Iterator = typename Relationships::Impl::Iterator;

    public:
        Impl( const Relationships::Impl& manager, Iterator begin, Iterator end )
            : BaseRange< Iterator >( begin, end ), manager_( manager )
        {
            next_collection_iterator();
        }

        void next()
        {
            this->operator++();
            next_collection_iterator();
        }

        const uuid& vertex_uuid() const
        {
            const auto iterator = this->current();
            return manager_.vertex_uuid(
                { iterator->edge_id, ( iterator->vertex_id + 1 ) % 2 } );
        }

    private:
        bool is_collection_edge_vertex()
        {
            return this->current()->vertex_id
                   == Relationships::Impl::COLLECTION_EDGE_VERTEX;
        }

        void next_collection_iterator()
        {
            while( this->operator!=( *this )
                   && ( !manager_.is_item_relation( this->current()->edge_id )
                          || is_collection_edge_vertex() ) )
            {
                this->operator++();
            }
        }

    private:
        const Relationships::Impl& manager_;
    };

    Relationships::CollectionRange::CollectionRange(
        const Relationships& manager, const uuid& id )
        : impl_( *manager.impl_,
              manager.impl_->begin_edge( id ),
              manager.impl_->end_edge( id ) )
    {
    }

    Relationships::CollectionRange::CollectionRange(
        CollectionRange&& other ) noexcept
        : impl_( *other.impl_ )
    {
    }

    Relationships::CollectionRange::CollectionRange(
        const CollectionRange& other )
        : impl_( *other.impl_ )
    {
    }

    Relationships::CollectionRange::~CollectionRange() {} // NOLINT

    bool Relationships::CollectionRange::operator!=(
        const CollectionRange& /*unused*/ ) const
    {
        return impl_->operator!=( *impl_ );
    }

    void Relationships::CollectionRange::operator++()
    {
        return impl_->next();
    }

    const uuid& Relationships::CollectionRange::operator*() const
    {
        return impl_->vertex_uuid();
    }
} // namespace geode