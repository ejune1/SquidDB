#ifndef TRAVERSE_CONTEXT_H
#define TRAVERSE_CONTEXT_H

#include "core/LockType.h"
#include "core/SkipListNode.h"

#include <cstddef>
#include <cstdint>
#include <unordered_set>

namespace squiddb { namespace core {

template<typename K>
class TraverseContext {
	public:
		TraverseContext(const std::uint8_t size, LockType lockType);
		~TraverseContext();

		std::uint8_t getSize() const;

		bool containsNode(SkipListNode<K>* node) const;
		SkipListNode<K>* getPrevNode(const std::uint8_t index) const;
		void setPrevNode(const std::uint8_t index, SkipListNode<K>* prevNode);

		size_t getPrevRank(const std::uint8_t index) const;
		void setPrevRank(const std::uint8_t index, const size_t prevRank);

		size_t getRank() const;
		void setRank(const size_t rank);

		void unlockAll();

	private:
		const std::uint8_t m_size;

		SkipListNode<K>** m_prevNode;
		size_t* m_prevRank;

		size_t m_rank;

		const LockType m_lockType;
		std::unordered_set<SkipListNode<K>*> m_lockNodes;
};

}} // namespace

#endif
