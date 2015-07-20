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

#include <ResourceAttributes.h>

#include <ResourceAttributesUtils.h>
#include <ResourceAttributesConverter.h>

#include <boost/lexical_cast.hpp>
#include <boost/mpl/advance.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/deref.hpp>

namespace
{

    using namespace OIC::Service;

    class ToStringVisitor: public boost::static_visitor< std::string >
    {
    public:
        ToStringVisitor() = default;
        ToStringVisitor(const ToStringVisitor&) = delete;
        ToStringVisitor(ToStringVisitor&&) = delete;

        ToStringVisitor& operator=(const ToStringVisitor&) = delete;
        ToStringVisitor& operator=(ToStringVisitor&&) = delete;

        template < typename T >
        std::string operator()(const T& value) const
        {
            return boost::lexical_cast<std::string>(value);
        }

        std::string operator()(std::nullptr_t) const
        {
            return "";
        }

        std::string operator()(bool value) const
        {
            return value ? "true" : "false";
        }

        std::string operator()(const std::string& value) const
        {
            return value;
        }

        std::string operator()(const OIC::Service::ResourceAttributes&) const
        {
            return "Attributes";
        }
    };

    class TypeVisitor: public boost::static_visitor< ResourceAttributes::Type >
    {
    public:
        TypeVisitor() = default;
        TypeVisitor(const TypeVisitor&) = delete;
        TypeVisitor(TypeVisitor&&) = delete;

        TypeVisitor& operator=(const TypeVisitor&) = delete;
        TypeVisitor& operator=(TypeVisitor&&) = delete;

        template< typename T >
        ResourceAttributes::Type operator()(const T& value) const
        {
            return ResourceAttributes::Type::typeOf(value);
        }

    };

    template< int >
    struct Int2Type {};

    template< typename T >
    struct TypeInfoConverter;

    template< >
    struct TypeInfoConverter< int >
    {
        static constexpr ResourceAttributes::TypeId typeId = ResourceAttributes::TypeId::INT;
    };

    template< >
    struct TypeInfoConverter< std::nullptr_t >
    {
        static constexpr ResourceAttributes::TypeId typeId = ResourceAttributes::TypeId::NULL_T;
    };

    template< >
    struct TypeInfoConverter< double >
    {
        static constexpr ResourceAttributes::TypeId typeId = ResourceAttributes::TypeId::DOUBLE;
    };

    template< >
    struct TypeInfoConverter< bool >
    {
        static constexpr ResourceAttributes::TypeId typeId = ResourceAttributes::TypeId::BOOL;
    };

    template< >
    struct TypeInfoConverter< std::string >
    {
        static constexpr ResourceAttributes::TypeId typeId = ResourceAttributes::TypeId::STRING;
    };

    template< >
    struct TypeInfoConverter< ResourceAttributes >
    {
        static constexpr ResourceAttributes::TypeId typeId = ResourceAttributes::TypeId::ATTRIBUTES;
    };

    struct TypeInfo
    {
        ResourceAttributes::TypeId typeId;

        template< typename TRAIT >
        constexpr TypeInfo(TRAIT) :
                typeId{ TRAIT::typeId }
        {
        }

        template< typename VARIANT, int POS >
        static constexpr TypeInfo get()
        {
            return TypeInfo(
                    TypeInfoConverter<
                            typename boost::mpl::deref<
                                    typename boost::mpl::advance<
                                            typename boost::mpl::begin< typename VARIANT::types >::type,
                                            boost::mpl::int_< POS > >::type >::type >{ });
        }
    };

    template< typename VARIANT, int POS >
    constexpr inline std::vector< TypeInfo > getTypeInfo(Int2Type< POS >)
    {
        auto&& vec = getTypeInfo< VARIANT >(Int2Type< POS - 1 >{ });
        vec.push_back(TypeInfo::get< VARIANT, POS >());
        return vec;
    }

    template< typename VARIANT >
    constexpr inline std::vector< TypeInfo > getTypeInfo(Int2Type< 0 >)
    {
        return { TypeInfo::get< VARIANT, 0 >() };
    }

    template< typename VARIANT >
    inline TypeInfo getTypeInfo(int which)
    {
        static constexpr int variantEnd = boost::mpl::size< typename VARIANT::types >::value - 1;
        static const std::vector< TypeInfo > typeInfos = getTypeInfo< VARIANT >(
                Int2Type< variantEnd >{ });

        return typeInfos[which];
    }

} // unnamed namespace


namespace OIC
{
    namespace Service
    {

        bool operator==(const ResourceAttributes::Type& lhs, const ResourceAttributes::Type& rhs)
        {
            return lhs.m_which == rhs.m_which;
        }

        bool operator!=(const ResourceAttributes::Type& lhs, const ResourceAttributes::Type& rhs)
        {
            return !(lhs == rhs);
        }

        bool operator!=(const ResourceAttributes::Value& lhs, const ResourceAttributes::Value& rhs)
        {
            return !(lhs == rhs);
        }

        bool operator!=(const char* lhs, const ResourceAttributes::Value& rhs)
        {
            return !(rhs == lhs);
        }

        bool operator==(const char* lhs, const ResourceAttributes::Value& rhs)
        {
            return rhs == lhs;
        }

        bool operator==(const ResourceAttributes::Value& lhs, const ResourceAttributes::Value& rhs)
        {
            return *lhs.m_data == *rhs.m_data;
        }

        bool operator==(const ResourceAttributes& lhs, const ResourceAttributes& rhs)
        {
            return lhs.m_values == rhs.m_values;
        }

        bool operator!=(const ResourceAttributes& lhs, const ResourceAttributes& rhs)
        {
            return !(lhs == rhs);
        }

        auto ResourceAttributes::Type::getId() const -> TypeId
        {
            return ::getTypeInfo< ValueVariant >(m_which).typeId;
        }

        ResourceAttributes::Value::Value() :
                m_data{ new ValueVariant{} }
        {
        }

        ResourceAttributes::Value::Value(const Value& from) :
                m_data{ new ValueVariant{ *from.m_data } }
        {
        }

        ResourceAttributes::Value::Value(Value&& from) :
                m_data{ new ValueVariant{} }
        {
            m_data->swap(*from.m_data);
        }

        ResourceAttributes::Value::Value(const char* value) :
                m_data{ new ValueVariant{ std::string{ value } } }
        {
        }

        auto ResourceAttributes::Value::operator=(const Value& rhs) -> Value&
        {
            *m_data = *rhs.m_data;
            return *this;
        }

        auto ResourceAttributes::Value::operator=(Value&& rhs) -> Value&
        {
            *m_data = ValueVariant{};
            m_data->swap(*rhs.m_data);
            return *this;
        }

        auto ResourceAttributes::Value::operator=(const char* rhs) -> Value&
        {
            *m_data = std::string{ rhs };
            return *this;
        }

        auto ResourceAttributes::Value::operator=(std::nullptr_t) -> Value&
        {
            *m_data = nullptr;
            return *this;
        }

        bool ResourceAttributes::Value::operator==(const char* rhs) const
        {
            return equals< std::string >(rhs);
        }

        auto ResourceAttributes::Value::getType() const -> Type
        {
            return boost::apply_visitor(TypeVisitor(), *m_data);
        }

        std::string ResourceAttributes::Value::toString() const
        {
            return boost::apply_visitor(ToStringVisitor(), *m_data);
        }

        void ResourceAttributes::Value::swap(Value& rhs)
        {
            m_data.swap(rhs.m_data);
        }

        auto ResourceAttributes::KeyValuePair::KeyVisitor::operator() (iterator* iter) const
                -> result_type {
            return iter->m_cur->first;
        }

        auto ResourceAttributes::KeyValuePair::KeyVisitor::operator() (const_iterator* iter) const
                -> result_type {
            return iter->m_cur->first;
        }

        auto ResourceAttributes::KeyValuePair::ValueVisitor::operator() (iterator* iter)
                -> result_type {
            return iter->m_cur->second;
        }

        auto ResourceAttributes::KeyValuePair::ValueVisitor::operator() (const_iterator* iter)
                -> result_type {
            // should not reach here.
            throw BadGetException("");
        }

        auto ResourceAttributes::KeyValuePair::ConstValueVisitor::operator() (iterator*iter) const
                -> result_type {
            return iter->m_cur->second;
        }

        auto ResourceAttributes::KeyValuePair::ConstValueVisitor::operator() (const_iterator* iter)
            const -> result_type {
            return iter->m_cur->second;
        }

        auto ResourceAttributes::KeyValuePair::key() const -> const std::string&
        {
            return boost::apply_visitor(m_keyVisitor, m_iterRef);
        }

        auto ResourceAttributes::KeyValuePair::value() const -> const Value&
        {
            return boost::apply_visitor(m_constValueVisitor, m_iterRef);
        }

        auto ResourceAttributes::KeyValuePair::value() -> Value&
        {
            return boost::apply_visitor(m_valueVisitor, m_iterRef);
        }

        ResourceAttributes::KeyValuePair::KeyValuePair(boost::variant<iterator*,
                const_iterator*>&& ref) :
                m_iterRef{ ref }
        {
        }


        ResourceAttributes::iterator::iterator() :
                m_cur{ base_iterator{ } },
                m_keyValuePair{ this }
        {
        }

        ResourceAttributes::iterator::iterator(base_iterator&& iter) :
                m_cur{ std::move(iter) },
                m_keyValuePair{ this }
        {
        }

        auto ResourceAttributes::iterator::operator*() -> KeyValuePair&
        {
            return m_keyValuePair;
        }

        auto ResourceAttributes::iterator::iterator::operator->() -> KeyValuePair*
        {
            return &m_keyValuePair;
        }

        auto ResourceAttributes::iterator::operator++() -> iterator&
        {
            ++m_cur;
            return *this;
        }

        auto ResourceAttributes::iterator::operator++(int) -> iterator
        {
            iterator iter(*this);
            ++(*this);
            return iter;
        }

        bool ResourceAttributes::iterator::operator==(const iterator& rhs) const
        {
            return m_cur == rhs.m_cur;
        }

        bool ResourceAttributes::iterator::operator!=(const iterator& rhs) const
        {
            return !(*this == rhs);
        }


        ResourceAttributes::const_iterator::const_iterator() :
                m_cur{ base_iterator{} }, m_keyValuePair{ this }
        {
        }

        ResourceAttributes::const_iterator::const_iterator(base_iterator&& iter) :
                m_cur{ iter }, m_keyValuePair{ this }
        {
        }

        ResourceAttributes::const_iterator::const_iterator(
                const ResourceAttributes::iterator& iter) :
                m_cur{ iter.m_cur }, m_keyValuePair{ this }
        {
        }

        auto ResourceAttributes::const_iterator::operator=(const ResourceAttributes::iterator& iter)
            -> const_iterator& {
            m_cur = iter.m_cur;
            return *this;
        }

        auto ResourceAttributes::const_iterator::operator*() const -> reference
        {
            return m_keyValuePair;
        }
        auto ResourceAttributes::const_iterator::operator->() const -> pointer
        {
            return &m_keyValuePair;
        }

        auto ResourceAttributes::const_iterator::operator++() -> const_iterator&
        {
            ++m_cur;
            return *this;
        }

        auto ResourceAttributes::const_iterator::operator++(int) -> const_iterator
        {
            const_iterator iter(*this);
            ++(*this);
            return iter;
        }

        bool ResourceAttributes::const_iterator::operator==(const const_iterator& rhs) const
        {
            return m_cur == rhs.m_cur;
        }

        bool ResourceAttributes::const_iterator::operator!=(const const_iterator& rhs) const
        {
            return !(*this == rhs);
        }

        auto ResourceAttributes::begin() -> iterator
        {
            return iterator{ m_values.begin() };
        }

        auto ResourceAttributes::end() -> iterator
        {
            return iterator{ m_values.end() };
        }

        auto ResourceAttributes::begin() const -> const_iterator
        {
            return const_iterator{ m_values.begin() };
        }

        auto ResourceAttributes::end() const -> const_iterator
        {
            return const_iterator{ m_values.end() };
        }

        auto ResourceAttributes::cbegin() const -> const_iterator
        {
            return const_iterator{ m_values.begin() };
        }

        auto ResourceAttributes::cend() const -> const_iterator
        {
            return const_iterator{ m_values.end() };
        }

        auto ResourceAttributes::operator[](const std::string& key) -> Value&
        {
            return m_values[key];
        }

        auto ResourceAttributes::operator[](std::string&& key) -> Value&
        {
            return m_values[std::move(key)];
        }

        auto ResourceAttributes::at(const std::string& key) -> Value&
        {
            try
            {
                return m_values.at(key);
            }
            catch (const std::out_of_range&)
            {
                throw InvalidKeyException{ "No attribute named '" + key + "'" };
            }
        }

        auto ResourceAttributes::at(const std::string& key) const -> const Value&
        {
            try
            {
                return m_values.at(key);
            }
            catch (const std::out_of_range&)
            {
                throw InvalidKeyException{ "No attribute named '" + key + "'" };
            }
        }

        void ResourceAttributes::clear()
        {
            return m_values.clear();
        }

        bool ResourceAttributes::erase(const std::string& key)
        {
            return m_values.erase(key) == 1U;
        }

        bool ResourceAttributes::contains(const std::string& key) const
        {
            return m_values.find(key) != m_values.end();
        }

        bool ResourceAttributes::empty() const
        {
            return m_values.empty();
        }

        size_t ResourceAttributes::size() const
        {
            return m_values.size();
        }


        bool acceptableAttributeValue(const ResourceAttributes::Value& dest,
                const ResourceAttributes::Value& value)
        {
            if (dest.getType() != value.getType())
            {
                return false;
            }

            static_assert(ResourceAttributes::is_supported_type< ResourceAttributes >::value,
                    "ResourceAttributes doesn't have ResourceAttributes recursively.");
            if (dest.getType().getId() == ResourceAttributes::TypeId::ATTRIBUTES
                    && !acceptableAttributes(dest.get< ResourceAttributes >(),
                            value.get< ResourceAttributes >()))
            {
                return false;
            }

            return true;
        }

        bool acceptableAttributes(const ResourceAttributes& dest, const ResourceAttributes& attr)
        {
            for (const auto& kv : attr)
            {
                if (!dest.contains(kv.key()))
                {
                    return false;
                }

                if (!acceptableAttributeValue(dest.at(kv.key()), kv.value()))
                {
                    return false;
                }
            }

            return true;
        }

        AttrKeyValuePairs replaceAttributes(ResourceAttributes& dest,
                const ResourceAttributes& newAttrs)
        {
            AttrKeyValuePairs replacedList;

            for (const auto& kv : newAttrs)
            {
                if (dest[kv.key()] != kv.value())
                {
                    ResourceAttributes::Value replacedValue;
                    replacedValue.swap(dest[kv.key()]);
                    dest[kv.key()] = kv.value();

                    replacedList.push_back(AttrKeyValuePair{ kv.key(), std::move(replacedValue) });
                }
            }

            return replacedList;
        }
    }
}
