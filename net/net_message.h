#pragma once
#include "net_common.h"

namespace olc
{
	namespace net
	{
		// Message Header is sent at the start of all messages. The template allows us
		// to use "enum class" to ensure that the messages are valid at compile time
		template <typename T>
		struct message_header {

			T id{};
			uint32_t size = 0;
		};

		template <typename T>
		struct message {

			message_header<T> header{};
			std::vector<uint8_t> body;

			// Returns the size of the entire message packet in bytes
			size_t size() const {

				return body.size();
			}

			// Overriding << operator for std::cout compatibility
			friend std::ostream& operator << (std::ostream& os, const message<T>& msg) {

				os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size;
				return os;
			}

			// Convenience Operator overloads - These allow us to add and remove stuff from
			// the body vector as if it were a stack, so First in, Last Out. These are a 
			// template in itself, because we dont know what data type the user is pushing or 
			// popping, so lets allow them all. NOTE: It assumes the data type is fundamentally
			// Plain Old Data (POD). TLDR: Serialise & Deserialise into/from a vector

			// Pushes any POD-like data into message buffer
			template <typename DataType>
			friend message<T>& operator << (message<T>& msg, const DataType& data) {

				// Check that the type of the data being push is trivially copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

				// Cache current size of vector, as this will be the point we insert the data
				size_t i = msg.body.size();

				// Resize the vector by the size of the data being pushed 
				msg.body.resize(msg.body.size() + sizeof(DataType));

				// Physically copying the data into the newly allocated vector space (in bytes)
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

				// Recalculate the message size
				msg.header.size = (uint32_t) msg.size();

				// Return the target message so it can be chained as with std::cout
				return msg;
			}

            // handle specific string data type
			friend message<T>& operator << (message<T>& msg, std::string &data) {

				// Cache current size of vector, as this will be the point we insert the data
				size_t i = msg.body.size();

				// Resize the vector by the size of the data being pushed 
				msg.body.resize(msg.body.size() + data.size());

				// Physically copying the data into the newly allocated vector space (in bytes)
				std::memcpy(msg.body.data() + i, &data, data.size());

				// Recalculate the message size
				msg.header.size = (uint32_t) msg.size();

				// Return the target message so it can be chained as with std::cout
				return msg;
			}

			// Pulls any POD-like data from a message buffer
			template <typename DataType>
			friend message<T>& operator >> (message<T>& msg, DataType& data) {

				// Check that the type of the data being push is trivially copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

				// Cache the location towards the end of the vector where the pulled data starts
				size_t i = msg.body.size() - sizeof(DataType);

				// Phisically copy the data from the vector into the user variable
				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

				// Shrink the vector to remove the read bytes and reset end position
				msg.body.resize(i);

				// Recalculate the message size
				msg.header.size = (uint32_t) msg.size();

				// Return the target message so it can be chained as with std::cout
				return msg;
			}

            // handle specific string data type
			friend message<T>& operator >> (message<T>& msg, std::string &data) {

				// Cache the location towards the end of the vector where the pulled data starts
				size_t i = msg.body.size() - data.size();

				// Phisically copy the data from the vector into the user variable
				std::memcpy(&data, msg.body.data() + i, data.size());

				// Shrink the vector to remove the read bytes and reset end position
				msg.body.resize(i);

				// Recalculate the message size
				msg.header.size = (uint32_t) msg.size();

				// Return the target message so it can be chained as with std::cout
				return msg;
			} 
		};

		// Forward declare the connection
		template <typename T>
		class connection;

		template <typename T>
		struct owned_message {

			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> msg;

			// Again, a friendly operator for std::cout
			friend std::ostream& operator << (std::ostream& os, const owned_message<T>& msg) {

				os << msg.msg;
				return os;
			}


		};
	}
}