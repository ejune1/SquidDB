#ifndef SKIPLISTNODE_H
#define SKIPLISTNODE_H

#include <cstddef>
#include <cstdint>

namespace squiddb { namespace core {

template<typename K>
class SkipListNode {
	public:
		SkipListNode(const K key, void* data, std::uint16_t size, const std::uint8_t height);
		~SkipListNode();

		K getKey() const;

		SkipListNode<K>* getNext(const std::uint8_t level) const;
		void setNext(const std::uint8_t level, SkipListNode<K>* next);

		size_t getWidth(const std::uint8_t level) const;
		void setWidth(const std::uint8_t level, const size_t width);

		std::uint8_t getHeight() const;

		void* getData();
		void setData(void* data, std::uint16_t size);

		std::uint16_t getSize() const;

	private:
		const K m_key;

		void* m_data;
		std::uint16_t m_size;

		const std::uint8_t m_height;
		SkipListNode<K>** m_next;
		size_t* m_width;
};

}} // namespace

#endif
