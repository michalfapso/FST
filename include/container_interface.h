#ifndef CONTAINER_INTERFACE_H
#define CONTAINER_INTERFACE_H
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include "dbg.h"

template <class TContainer>
class ContainerInterface
{
	protected:
		typedef TContainer Container;
		Container mContainer;
	public:
		typedef typename Container::iterator       iterator;
		typedef typename Container::const_iterator const_iterator;
		inline const_iterator begin() const { return mContainer.begin(); }
		inline iterator       begin()       { return mContainer.begin(); }
		inline const_iterator end()   const { return mContainer.end(); }
		inline iterator       end()         { return mContainer.end(); }
		inline size_t         size()  const { return mContainer.size(); }
		inline bool           empty() const { return mContainer.empty(); }
};

template <class TContainer>
class IndexedContainerInterface : public ContainerInterface< TContainer >
{
	protected:
		typedef typename TContainer::value_type ValType;
	public:
		ValType& operator[](size_t idx) { return this->mContainer[idx]; }
		const ValType& operator[](size_t idx) const { return this->mContainer[idx]; }
};

template <class TMap>
class MapInterface : public ContainerInterface< TMap >
{
	protected:
		typedef ContainerInterface< TMap > Base;
		typedef typename TMap::mapped_type ValType;
		typedef typename TMap::key_type KeyType;
		BOOST_STATIC_ASSERT(boost::is_pointer<ValType>::value);
	public:
		// Find() non-const
		ValType Find(const KeyType& key) {
			typename Base::iterator it = this->mContainer.find(key);
			return it != this->mContainer.end() ? it->second : NULL;
		}

		// Find() const
		const ValType Find(const KeyType& key) const {
			typename Base::const_iterator it = this->mContainer.find(key);
			return it != this->mContainer.end() ? it->second : NULL;
		}
};
#endif
