#ifndef SKIPLISTNODE_H
#define SKIPLISTNODE_H

#include <cstdint>

namespace squiddb { namespace core {

template<typename K>
class SkipListNode {
	public:
		SkipListNode(K key, std::uint8_t height);
		~SkipListNode();

		K getKey() const;
		SkipListNode<K>* getNext(const std::uint8_t level) const;
		std::uint8_t getHeight() const;

		void* getData() const;
		void setData(void* data);

	private:
		K m_key;
		SkipListNode<K>** m_next;
		std::uint8_t m_height;

		void* m_data;
		unsigned int m_size;
};

}} // namespace

#endif
