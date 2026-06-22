#ifndef QUEUENODE_H
#define QUEUENODE_H

namespace squiddb { namespace infra {

template<typename T>
class QueueNode {
	public:
		QueueNode(const T& data) {
			m_data = data;
			m_next = nullptr;
		}

		virtual ~QueueNode() = default;

		T getData() const {
			return m_data;
		}

		void setData(const T& data) {
			m_data = data;
		}

		QueueNode<T>* getNext() const {
			return m_next;
		}

		void setNext(QueueNode<T>* next) {
			m_next = next;
		}

	private:
		T m_data;
		QueueNode<T>* m_next;
};

}} // namespace

#endif
