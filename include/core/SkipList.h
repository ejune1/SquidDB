#ifndef SKIPLIST_H
#define SKIPLIST_H

#include "core/SkipListNode.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <cstdint>
#include <random>

namespace squiddb { namespace core {

template<typename K>
class SkipList {
	public:
		SkipList(const utils::Configuration& configuration, utils::Logger& logger, const bool primaryIndex);
		~SkipList();

		// must be called before anything else
		void initialize();

		// data modification (execution engine)
		
		// note size of data limited to 65,535
		bool insert(const K key, void* data, std::uint16_t size);
		bool remove(const K key);
		bool update(const K key, void* data);

		// point lookup and verification
		void* find(const K key) const;
		bool contains(const K key) const;

		// iteration and scan (execution engine)
		// TODO build an iterator

		// statistical (query planner)
		size_t size(bool calculate = false) const;
		size_t estimateRangeCardinality(const K lowKey, const K highKey) const;
		size_t memoryUsageMB() const;

		// this needs to be fast - called on every new node
		static inline std::uint8_t generateNodeHeight(std::uint8_t maxNodeHeight) {
			// TODO use a faster way of generating seed
			thread_local std::minstd_rand0 gen(std::random_device{}());

			std::uint8_t nodeHeight = 1;
			for (; nodeHeight < maxNodeHeight; ++nodeHeight) {
				if (gen() & 1) {
					break;
				}
			}

			return nodeHeight;
		}

		// not allowed
		SkipList(const SkipList<K>& other) = delete;
		SkipList<K>& operator=(const SkipList<K>& other) = delete;
		SkipList(SkipList<K>&& other) = delete;
		SkipList<K>& operator=(SkipList<K>&& other) = delete;

	private:
		const utils::Configuration& m_configuration;
		utils::Logger& m_logger;

		std::uint8_t m_maxNodeHeight;
		SkipListNode<K>* m_head;

		const bool m_primaryIndex;
};

}} // namespace

#endif
