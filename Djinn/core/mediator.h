#pragma once

#include "mediator_queue.h"

namespace djinn {
	template <typename T, typename H>
	void add_handler(H* handler);

	template <typename T, typename H>
	void remove_handler(H* handler);

	template <typename T>
	void remove_all_handlers();

	template <typename T>
	void broadcast(const T& message);

	// [NOTE] this *can* be used as a base class, but it's not required
	template <typename T>
	class MessageHandler {
public:
		MessageHandler();
		virtual ~MessageHandler();

		MessageHandler(const MessageHandler&) = default;
		MessageHandler& operator=(const MessageHandler&) = default;
		MessageHandler(MessageHandler&&)                 = default;
		MessageHandler& operator=(MessageHandler&&) = default;

		virtual void operator()(const T& message) = 0;
	};
}  // namespace djinn

#include "mediator.inl"
