#pragma once

#include "net_common.h"

namespace olc {
	namespace net {
		template <typename T>
		struct message_header
		{
			T id{};
			uint32_t message_size = 0;
		};
		template <typename T>
		struct message
		{
			message_header<T> header{};
			std::vector<uint8_t> body;

			size_t size() const {
				return sizeof(message_header<T>) + body.size();
			}

			friend std::ostream& operator << (std::ostream &os, const message<T> &msg) {
				os << "ID : " << msg.header.id << "Size : " << msg.header.message_size;
			}

			template <typename DataType>
			friend message<T>& operator << (message<T> &msg, const DataType &data ) {
				static_assert(std::is_standard_layout<DataType>::value,"Data is Too big" );
				
				size_t prev_body_size = msg.body.size();

				// the data is adding to message body, Thus, the body size should be bigger
				msg.body.resize(msg.body.size() + sizeof(DataType) );
				std::memcpy(msg.body.data() + prev_body_size, &data, sizeof(DataType));

				msg.header.message_size = msg.size();
				return msg;

			}
			template <typename DataType>
			friend message<T>& operator >> (message<T>& msg, DataType& data) {
				static_assert(std::is_standard_layout<DataType>::value, "Data is Too big");

				size_t post_body_size = msg.body.size() - sizeof(DataType);

				//msg << "ID : " << msg.header.id << "Size : " << msg.header.message_size;

				std::memcpy(&data, msg.body.data() + post_body_size, sizeof(DataType));

				msg.body.resize(post_body_size);

				msg.header.message_size = msg.size();
				return msg;

			}
		};

		template <typename T>
		class connection {

		};

		template <typename T>
		struct owned_message {
			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> _msg;
			
			friend std::ostream& operator << std::ostream& os, const owned_message<T>& msg) {

				os << msg.msg;
				return os;
			}
		};
	}
}
