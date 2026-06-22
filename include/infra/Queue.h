#ifndef QUEUE_H
#define QUEUE_H

#include "infra/QueueNode.h"

#include <atomic>
#include <stdexcept>
#include <type_traits>

namespace squiddb { namespace infra {

template<typename T>
class Queue {
	public:
		Queue() {
			m_first = nullptr;
			m_last = nullptr;
			m_size = 0;
			m_consumed = 0;
		}

		virtual ~Queue() {
			QueueNode<T>* node = m_first;

			while (node != nullptr) {
				QueueNode<T>* nodeToDelete = node;
				node = node->getNext();

				T data = nodeToDelete->getData();

				if constexpr (std::is_pointer_v<T>) {
					if (data != nullptr) {
						delete data;
					}
				}
				delete nodeToDelete;
			}

			m_first = nullptr;
			m_last = nullptr;
			m_size = 0;
		}

		void enqueue(const T& data) {
			if (m_first == nullptr) {
				m_first = new QueueNode<T>(data);
				m_last = m_first;

			} else {
				QueueNode<T>* node = new QueueNode<T>(data);
				m_last->setNext(node);
				m_last = node;
			}

			m_size++;
		}

		T dequeue() {
			if (empty()) {
				throw std::runtime_error("Queue::dequeue queue is empty");
			}

			T data = m_first->getData();
			QueueNode<T>* nodeToDelete = m_first;

			if (m_first == m_last) {
				m_first = nullptr;
				m_last = nullptr;

			} else {
				m_first = m_first->getNext();
			}

			delete nodeToDelete;
			m_size--;
			m_consumed++;

			return data;
		}

		bool empty() const {
			return (m_size == 0);
		}
	
		size_t size() const {
			return m_size;
		}

		size_t consumed() const {
			return m_consumed;
		}

	private:
		QueueNode<T>* m_first;
		QueueNode<T>* m_last;

		std::atomic<size_t> m_size;
		std::atomic<size_t> m_consumed;
};

}} // namespace

#endif
