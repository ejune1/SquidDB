#ifndef SKIPLISTITERATOR_H
#define SKIPLISTITERATOR_H

#include "engine/TableIterator.h"
#include "core/SkipListNode.h"
#include "core/Transaction.h"

#include <cstdint>
#include <cstddef>
#include <iterator>
#include <optional>

namespace squiddb { namespace core {

template<typename K>
class SkipList;

template<typename K>
class SkipListIterator : public engine::TableIterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type        = K;
		using difference_type   = std::ptrdiff_t;
		using pointer           = const K*;
		using reference         = K;

		SkipListIterator();
		SkipListIterator(
			const SkipList<K>* skipList,
			const SkipListNode<K>* startNode, 
			std::optional<int> endKey, 
			Transaction* transaction, 
			std::size_t sequence
		);
		~SkipListIterator();

		reference operator*() const;
		SkipListIterator<K>& operator++();
		SkipListIterator<K> operator++(int);

		bool operator==(const SkipListIterator<K>& other) const;
		bool operator!=(const SkipListIterator<K>& other) const;	

		// TableIterator
		bool valid() const override;
		void next() override;
		
		const void* getKey() const override;
		const void* getData() const override;

	private:
		const SkipList<K>* m_skipList;
		const SkipListNode<K>* m_current;
		std::optional<K> m_endKey;

		void advance(bool next);
		Transaction* m_transaction;

		std::size_t m_sequence;
};

}} // namespace

#endif
