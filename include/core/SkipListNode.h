#ifndef SKIPLISTNODE_H
#define SKIPLISTNODE_H

#include "core/RowInfo.h"

#include <cstddef>
#include <cstdint>
#include <shared_mutex>

namespace squiddb { namespace core {

template<typename K>
class SkipListNode {
	public:
		SkipListNode(const K key, const std::uint8_t height);
		~SkipListNode();

		K getKey() const;
		// TODO fix with opaque keys (return internal data)
		const K* getKeyRef() const;

		RowInfo* getRowInfo() const;
		void setRowInfo(RowInfo* rowInfo);

		SkipListNode<K>* getNext(const std::uint8_t level) const;
		void setNext(const std::uint8_t level, SkipListNode<K>* next);

		size_t getWidth(const std::uint8_t level) const;
		void setWidth(const std::uint8_t level, const size_t width);

		std::uint8_t getHeight() const;

		void writeLock() const;
		void writeUnlock() const;
		void readLock() const;
		void readUnlock() const;

	private:
		const K m_key;
		RowInfo* m_rowInfo;

		const std::uint8_t m_height;
		SkipListNode<K>** m_next;
		size_t* m_width;

		// TODO performance - this is too heavy
		mutable std::shared_mutex m_mutex;
};

}} // namespace

#endif
