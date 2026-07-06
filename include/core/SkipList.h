#ifndef SKIPLIST_H
#define SKIPLIST_H

#include "core/SkipListIterator.h"
#include "core/SkipListNode.h"
#include "core/TraverseContext.h"
#include "engine/Index.h"
#include "engine/TableIterator.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>

namespace squiddb { namespace core {

template<typename K>
class SkipList : public engine::Index {
	public:
		SkipList(utils::Logger& logger, const bool primaryIndex, const uint8_t maxNodeHeight);
		~SkipList();

		// must be called before anything else
		void initialize();

		// data modification (execution engine)
		
		// note size of data limited to 65,535
		bool insert(const K key, std::byte* data, const std::uint16_t size, std::uint8_t nodeHeight = 0);
		bool remove(const K key);
		bool update(const K key, std::byte* data, const std::uint16_t size);

		// point lookup and verification
		std::byte* find(const K key) const;
		bool contains(const K key) const;

		// iteration and scan (execution engine)
		SkipListIterator<K> begin() const;
		SkipListIterator<K> seek(const K key, const std::optional<K> endKey = std::nullopt) const;
		SkipListIterator<K> end() const;

		// statistical (query planner)
		bool empty() const;
		size_t size(const bool calculate = false) const;
		size_t estimateRangeCardinality(const K lowKey, const K highKey) const;
		size_t memoryUsageMB() const;

		// Index methods
		bool insertRow(const void* key, void* row, const std::uint16_t size) override;
		bool deleteRow(const void* key) override;
		bool updateRow(const void* key, void* row, const std::uint16_t size) override;

		engine::TableIterator* scan() const;
		engine::TableIterator* rangeScan(const void* startKey, const void* endKey) const;

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

		// test support
		bool validate() const;

	private:
		SkipListNode<K>* findNode(const K key) const;
		TraverseContext<K>* traversePrevNodes(const K key, bool& duplicateKey) const;

		utils::Logger& m_logger;

		const bool m_primaryIndex;
		const std::uint8_t m_maxNodeHeight;

		SkipListNode<K>* m_head;

		std::atomic<bool> m_initialized;
		std::atomic<size_t> m_size;
};

}} // namespace

#endif
