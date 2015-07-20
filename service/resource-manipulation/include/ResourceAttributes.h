//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef RES_MANIPULATION_RESOURCEATTRIBUTES_H
#define RES_MANIPULATION_RESOURCEATTRIBUTES_H

// To avoid conflict using different boost::variant configuration with OC.
// It causes compile errors.
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 30
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30

#include <functional>
#include <unordered_map>

#include <boost/variant.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/scoped_ptr.hpp>

#include <RCSException.h>
/**
 * @file
 *
 * This file contains the "ResourceAttributes" class & its helper classes
 */
namespace OIC
{
    namespace Service
    {
        /**
         * @class   BadGetException
         * @brief   This class is used to throw exception to the upper layer if Get request for a particular
         *              attribute is invalid. It is  inherited from RCSException class.
         *
         */
        class BadGetException: public RCSException
        {
        public:
            BadGetException(const std::string& what) : RCSException{ what } {}
            BadGetException(std::string&& what) : RCSException{ std::move(what) } {}
        };
        /**
        * @class   InvalidKeyException
        * @brief   This class is used to throw exception to the upper layer if key is invalid.
        *              It is inherited from RCSException class.
        *
        */
        class InvalidKeyException: public RCSException
        {
        public:
            InvalidKeyException(const std::string& what) : RCSException{ what } {}
            InvalidKeyException(std::string&& what) : RCSException{ std::move(what) } {}
        };

        /**
        * @class   ResourceAttributes
        * @brief   This class represents the attributes for a resource.
        * It overloaded the various operators like ==, [], = etc. It provides the APIs that a std::Map provides
        * like begin, end, size etc. Also provides two kinds of iterator to iterate over the attributes.
        *
        *  It has helper classes:
        *  Value - For value of the Attribute
        *  Type - For data type of the Attribute
        *  iterator - For iterating over attributes
        *  const_iterator - const iterator for attributes
        *
        *
        * @see Value
        * @see Type
        * @see iterator
        * @see const_iterator
        *
        * NOTE:  If Developer wants to get the ResourceAttributes for the resource of interest following
        *            are the steps:
        *            - first call the discover API of DiscoveryManager class.
        *            - After getting the RemoteResourceObject, call getRemoteAttributes() API
        *               of RemoteResourceObject class
        *
        * @see DiscoveryManager
        * @see RemoteResourceObject
        */
        class ResourceAttributes
        {
        private:
            template< typename T > struct IsSupportedTypeHelper;

            typedef boost::variant<
                std::nullptr_t,
                int,
                double,
                bool,
                std::string,
                ResourceAttributes
            > ValueVariant;

            template< typename T, typename V = void,
                    typename = typename std::enable_if<
                        IsSupportedTypeHelper< T >::type::value, V >::type >
            struct enable_if_supported
            {
                typedef V type;
            };

            template< typename VISITOR >
            class KeyValueVisitorHelper: public boost::static_visitor< >
            {
            public:
                KeyValueVisitorHelper(VISITOR& visitor) :
                        m_visitor( visitor )
                {
                }

                template< typename T >
                void operator()(const std::string& key, const T& value) const
                {
                    m_visitor(key, value);
                }

            private:
                VISITOR& m_visitor;
            };

            template <typename T> struct IndexOfType;

        public:
            template< typename T >
                /**
                  * For checking whether the provided type is supported or not for a attribute.
                */
            struct is_supported_type: public std::conditional<
                IsSupportedTypeHelper< T >::type::value, std::true_type, std::false_type>::type { };

                /**
                * enum class for the different supported types for Attributes
                */
            enum class TypeId
            {
                NULL_T,
                INT,
                DOUBLE,
                BOOL,
                STRING,
                ATTRIBUTES,
                VECTOR
            };

                /**
                * This class contains APIs for "Type" of the attribute.
                *  All Types are specified in enum TypeId.
                *
                * @see TypeId
                */
            class Type
            {
            public:
                Type(const Type&) = default;
                Type(Type&&) = default;

                Type& operator=(const Type&) = default;
                Type& operator=(Type&&) = default;

                TypeId getId() const;

                template < typename T >
                static Type typeOf(const T& value)
                {
                    return Type(value);
                }

                friend bool operator==(const Type&, const Type&);

            private:
                template < typename T >
                explicit Type(const T&) :
                    m_which{ IndexOfType< T >::value }
                {
                }

            private:
                int m_which;
            };

                /**
                * This class provides APIs for the Value of the attributes.
                */
            class Value
            {
            public:
                Value();
                Value(const Value&);
                Value(Value&&);

                template< typename T, typename = typename enable_if_supported< T >::type >
                Value(T&& value) :
                        m_data{ new ValueVariant{ std::forward< T >(value) } }
                {
                }

                Value(const char* value);

                Value& operator=(const Value&);
                Value& operator=(Value&&);

                template< typename T, typename = typename enable_if_supported< T >::type >
                Value& operator=(T&& rhs)
                {
                    *m_data = std::forward< T >(rhs);
                    return *this;
                }

                Value& operator=(const char*);
                Value& operator=(std::nullptr_t);

                template< typename T >
                typename std::add_lvalue_reference< const T >::type get() const
                {
                    return checkedGet< T >();
                }

                template< typename T >
                typename std::add_lvalue_reference< T >::type get()
                {
                    return checkedGet< T >();
                }

                Type getType() const;

                std::string toString() const;

                void swap(Value&);

                friend bool operator==(const Value&, const Value&);

                template< typename T >
                friend typename std::enable_if< ResourceAttributes::is_supported_type< T >::value,
                    bool >::type operator==(const Value&, const T&);

                bool operator==(const char*) const;

            private:
                template< typename T, typename = typename enable_if_supported< T >::type >
                typename std::add_lvalue_reference< T >::type checkedGet() const
                {
                    try
                    {
                        return boost::get< T >(*m_data);
                    }
                    catch (const boost::bad_get&)
                    {
                        throw BadGetException{ "Wrong type" };
                    }
                }

                template< typename T, typename U >
                bool equals(const U& rhs) const
                {
                    try
                    {
                        return get< T >() == rhs;
                    }
                    catch (const BadGetException&)
                    {
                        return false;
                    }
                }

            private:
                boost::scoped_ptr< ValueVariant > m_data;

                friend class ResourceAttributes;
            };

            class KeyValuePair;
            class iterator;
            class const_iterator;

        public:
            ResourceAttributes() = default;
            ResourceAttributes(const ResourceAttributes&) = default;
            ResourceAttributes(ResourceAttributes&&) = default;

            ResourceAttributes& operator=(const ResourceAttributes&) = default;
            ResourceAttributes& operator=(ResourceAttributes&&) = default;

            iterator begin();
            iterator end();

            const_iterator begin() const;
            const_iterator end() const;

            const_iterator cbegin() const;
            const_iterator cend() const;

            Value& operator[](const std::string&);
            Value& operator[](std::string&&);

            Value& at(const std::string&);
            const Value& at(const std::string&) const;

            void clear();
            bool erase(const std::string&);

            bool contains(const std::string&) const;
            bool empty() const;
            size_t size() const;

            friend bool operator==(const ResourceAttributes&, const ResourceAttributes&);

        private:
            template< typename VISITOR >
            void visit(VISITOR& visitor) const
            {
                KeyValueVisitorHelper< VISITOR > helper{ visitor };

                for (const auto& i : m_values)
                {
                    boost::variant< const std::string& > key{ i.first };
                    boost::apply_visitor(helper, key, *i.second.m_data);
                }
            }

        private:
            std::unordered_map< std::string, Value > m_values;

            friend class ResourceAttributesConverter;
        };

        template< typename T >
        struct ResourceAttributes::IsSupportedTypeHelper
        {
            typedef boost::mpl::contains<ValueVariant::types, typename std::decay< T >::type> type;
        };

        template <typename T>
        struct ResourceAttributes::IndexOfType
        {
            typedef typename boost::mpl::find< ValueVariant::types, T >::type iter;
            typedef typename boost::mpl::begin< ValueVariant::types >::type mpl_begin;

            static constexpr int value = boost::mpl::distance< mpl_begin, iter >::value;
        };

        bool operator==(const ResourceAttributes::Type&, const ResourceAttributes::Type&);
        bool operator!=(const ResourceAttributes::Type&, const ResourceAttributes::Type&);

        bool operator!=(const ResourceAttributes::Value&, const ResourceAttributes::Value&);

        template< typename T >
        typename std::enable_if< ResourceAttributes::is_supported_type< T >::value, bool >::type
        operator==(const ResourceAttributes::Value& lhs, const T& rhs)
        {
            return lhs.equals< T >(rhs);
        }

        template< typename T >
        typename std::enable_if< ResourceAttributes::is_supported_type< T >::value, bool >::type
        operator!=(const T& lhs, const ResourceAttributes::Value& rhs)
        {
            return !(rhs == lhs);
        }

        bool operator!=(const char*, const ResourceAttributes::Value&);

        template< typename T >
        bool operator==(const T& lhs, const ResourceAttributes::Value& rhs)
        {
            return rhs == lhs;
        }

        bool operator==(const char* lhs, const ResourceAttributes::Value& rhs);

        bool operator!=(const ResourceAttributes&, const ResourceAttributes&);

        /**
         * This class is for the static visitors of key-value for a attribute.
         *
         * It has three sub-classes
         * - KeyVisitor : for key visitor
         * - ValueVisitor : for value visitor
         * - ConstValueVisitor : for const value visitor
         * All these 3 sub-classes inheriting the boost::static_visitor which allows invocation as a function
         * by overloading operator(),  unambiguously accepting any value of type T. All the 3 classes are
         * inherting the  boost::static_visitor with same type  i.e. String.
         *
         */
        class ResourceAttributes::KeyValuePair
        {
        private:
            class KeyVisitor: public boost::static_visitor< const std::string& >
            {
            public:
                result_type operator()(iterator*) const;
                result_type operator()(const_iterator*) const;
            };

            class ValueVisitor: public boost::static_visitor< Value& >
            {
            public:
                result_type operator()(iterator*);
                result_type operator()(const_iterator*);
            };

            class ConstValueVisitor: public boost::static_visitor< const Value& >
            {
            public:
                result_type operator()(iterator*) const;
                result_type operator()(const_iterator*) const;
            };

        public:
            const std::string& key() const;
            const ResourceAttributes::Value& value() const;
            ResourceAttributes::Value& value();

        private:
            KeyValuePair(const KeyValuePair&) = default;
            KeyValuePair(boost::variant< iterator*, const_iterator* >&&);

            KeyValuePair& operator=(const KeyValuePair&) = default;

        private:
            boost::variant< iterator*, const_iterator* > m_iterRef;

            KeyVisitor m_keyVisitor;
            ValueVisitor m_valueVisitor;
            ConstValueVisitor m_constValueVisitor;

            friend class iterator;
            friend class const_iterator;
        };

        /**
        * This class is an Iterator for the ResourceAttributes.
        */
        class ResourceAttributes::iterator: public std::iterator< std::forward_iterator_tag,
                ResourceAttributes::KeyValuePair >
        {
        private:
            typedef std::unordered_map< std::string, Value >::iterator base_iterator;

        public:
            iterator();
            iterator(const iterator&) = default;

            iterator& operator=(const iterator&) = default;

            reference operator*();
            pointer operator->();

            iterator& operator++();
            iterator operator++(int);

            bool operator==(const iterator&) const;
            bool operator!=(const iterator&) const;

        private:
            explicit iterator(base_iterator&&);

        private:
            base_iterator m_cur;
            ResourceAttributes::KeyValuePair m_keyValuePair;

            friend class ResourceAttributes;
        };
        /**
        * This class is an Const Iterator for the ResourceAttributes.
        */
        class ResourceAttributes::const_iterator: public std::iterator < std::forward_iterator_tag,
                const ResourceAttributes::KeyValuePair >
        {
        private:
            typedef std::unordered_map< std::string, Value >::const_iterator base_iterator;

        public:
            const_iterator();
            const_iterator(const const_iterator&) = default;
            const_iterator(const ResourceAttributes::iterator&);

            const_iterator& operator=(const const_iterator&) = default;
            const_iterator& operator=(const ResourceAttributes::iterator&);

            reference operator*() const;
            pointer operator->() const;

            const_iterator& operator++();
            const_iterator operator++(int);

            bool operator==(const const_iterator&) const;
            bool operator!=(const const_iterator&) const;

        private:
            explicit const_iterator(base_iterator&&);

        private:
            base_iterator m_cur;
            ResourceAttributes::KeyValuePair m_keyValuePair;

            friend class ResourceAttributes;
        };

    }
}

#endif // RES_MANIPULATION_RESOURCEATTRIBUTES_H
