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

#pragma once

#include <functional>

#include <absl/container/fixed_array.h>

#include <bitsery/adapter/stream.h>
#include <bitsery/bitsery.h>
#include <bitsery/ext/compact_value.h>
#include <bitsery/ext/inheritance.h>
#include <bitsery/ext/pointer.h>

#include <geode/basic/common.h>
#include <geode/basic/range.h>

namespace geode
{
    using PContext =
        bitsery::ext::PolymorphicContext< bitsery::ext::StandardRTTI >;
    using TContext = std::tuple< PContext,
        bitsery::ext::PointerLinkingContext,
        bitsery::ext::InheritanceContext >;
    using Serializer =
        bitsery::Serializer< bitsery::OutputBufferedStreamAdapter, TContext >;
    using Deserializer =
        bitsery::Deserializer< bitsery::InputStreamAdapter, TContext >;

    /*!
     * Register all the information needed by Bitsery to serialize the objects
     * in the basic library.
     * @param[in] context The context where to register this information.
     * @warning The context can be used only once per archive.
     */
    void opengeode_basic_api register_basic_serialize_pcontext(
        PContext &context );

    /*!
     * Register all the information needed by Bitsery to deserialize the objects
     * in the basic library.
     * @param[in] context The context where to register this information.
     * @warning The context can be used only once per archive.
     */
    void opengeode_basic_api register_basic_deserialize_pcontext(
        PContext &context );

    template < typename Archive, typename T >
    class DefaultGrowable
    {
    public:
        template < typename Fnc >
        void serialize( Archive &ser, const T &obj, Fnc &&fnc ) const
        {
            constexpr index_t FIRST_VERSION{ 1 };
            ser.ext4b( FIRST_VERSION, bitsery::ext::CompactValue{} );
            fnc( ser, const_cast< T & >( obj ) );
        }

        template < typename Fnc >
        void deserialize( Archive &des, T &obj, Fnc &&fnc ) const
        {
            index_t current_version;
            des.ext4b( current_version, bitsery::ext::CompactValue{} );
            fnc( des, obj );
        }
    };

    template < typename Archive, typename T >
    class Growable
    {
        static constexpr index_t FIRST_VERSION{ 1 };

    public:
        Growable( absl::FixedArray< std::function< void( Archive &, T & ) > >
                serializers )
            : Growable( std::move( serializers ), {} )
        {
        }
        Growable( absl::FixedArray< std::function< void( Archive &, T & ) > >
                      serializers,
            absl::FixedArray< std::function< void( T & ) > > initializers )
            : version_( serializers.size() ),
              serializers_( std::move( serializers ) ),
              initializers_( std::move( initializers ) )
        {
            OPENGEODE_EXCEPTION( version_ > FIRST_VERSION,
                "[Growable] Provide at least 2 serializers or use "
                "DefaultGrowable" );
            OPENGEODE_EXCEPTION(
                initializers_.empty() || initializers_.size() == version_ - 1,
                "[Growable] Should have as many initializers than the version "
                "number minus one (or none)" );
        }

        template < typename Fnc >
        void serialize( Archive &ser, const T &obj, Fnc &&fnc ) const
        {
            geode_unused( fnc );
            ser.ext4b( version_, bitsery::ext::CompactValue{} );
            serializers_.back()( ser, const_cast< T & >( obj ) );
        }

        template < typename Fnc >
        void deserialize( Archive &des, T &obj, Fnc &&fnc ) const
        {
            geode_unused( fnc );
            index_t current_version;
            des.ext4b( current_version, bitsery::ext::CompactValue{} );
            serializers_.at( current_version - 1 )( des, obj );
            if( !initializers_.empty() && current_version < version_ )
            {
                initializers_.at( current_version - 1 )( obj );
            }
        }

    private:
        index_t version_{ FIRST_VERSION };
        absl::FixedArray< std::function< void( Archive &, T & ) > >
            serializers_;
        absl::FixedArray< std::function< void( T & ) > > initializers_;
    };
} // namespace geode

namespace bitsery
{
    namespace traits
    {
        template < typename Archive, typename T >
        struct ExtensionTraits< geode::DefaultGrowable< Archive, T >, T >
        {
            using TValue = T;
            static constexpr bool SupportValueOverload = false;
            static constexpr bool SupportObjectOverload = true;
            static constexpr bool SupportLambdaOverload = true;
        };
        template < typename Archive, typename T >
        struct ExtensionTraits< geode::Growable< Archive, T >, T >
        {
            using TValue = T;
            static constexpr bool SupportValueOverload = false;
            static constexpr bool SupportObjectOverload = true;
            static constexpr bool SupportLambdaOverload = true;
        };
    } // namespace traits
} // namespace bitsery

#define SERIALIZE_BITSERY_ARCHIVE( EXPORT, TYPE )                              \
    template EXPORT void TYPE::serialize< geode::Serializer >(                 \
        geode::Serializer & );                                                 \
    template EXPORT void TYPE::serialize< geode::Deserializer >(               \
        geode::Deserializer & )
