#pragma once

#include <functional>
#include <mutex>
#include <vector>

namespace djinn::core::detail {
	template <typename T>
	class MediatorQueue {
	private:
		MediatorQueue() = default;

	public:
		static MediatorQueue& instance();

		template <typename H>
		void add(H* handler);

		// [NOTE] possibly the H template is not needed here, but I keep it for symmetry
		template <typename H>
		void remove(H* handler);

		void removeAll();

		void broadcast(const T& message);

	private:
		using Mutex     = std::mutex;
		using LockGuard = std::lock_guard<Mutex>;
		using Handler   = std::function<void(const T&)>;

		Mutex                m_Mutex;
		std::vector<Handler> m_Handlers;
		std::vector<void*>   m_Originals;
	};
}  // namespace djinn::core::detail

#include "mediator_queue.inl"
