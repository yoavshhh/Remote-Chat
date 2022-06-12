#pragma once


#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace olc
{
	namespace net
	{
		template <typename T>
		class connection : public std::enable_shared_from_this<connection<T>> {

		public:
			const static bool debugWrite = false;
			const static bool debugRead = false;

		public:
			enum class owner {

				server,
				client
			};

			connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
				: m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn) {

				m_nOwnerType = parent;
			}

			virtual ~connection() {}
			
			uint32_t GetID() const {

				return id;
			}

		public:
			void ConnectToClient(uint32_t uid) {
				
				if (m_nOwnerType == owner::server) {

					if (m_socket.is_open()) {

						id = uid;
						ReadHeader();
					}
				}
			}

			void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {

				// Only clients can connect to server
				if (m_nOwnerType == owner::client) {

					std::cout << "Connecting\n";

					// Request asio attempts to connect to endpoint
					asio::async_connect(m_socket, endpoints,
						[this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {

							if (!ec) {

								ReadHeader();
							}
							else {

								// Couldn't connect to server
								std::cout << "Establishing Connection Failed due to: " << ec.message() << "\n";
							}
						});
				}
			}

			void Disconnect() {

				if (IsConnected())
					asio::post(m_asioContext, [this]() { m_socket.close(); });
			}

			bool IsConnected() const {

				return m_socket.is_open();
			}

		public:
			// ASYNC - Send a message, connections are one-to-one so no need to specifiy
			// the target, for a client, the target is the server and vice versa
			void Send(const message<T>& msg) {

				asio::post(m_asioContext,
					[this, msg]() {

						// If the queue has a message in it, then we must 
						// assume that it is in the process of asynchronously being written.
						// Either way add the message to the queue to be output. If no messages
						// were available to be written, then start the process of writing the
						// message at the front of the queue.
						bool bWritingMessage = !m_qMessagesOut.empty();
						m_qMessagesOut.push_back(msg);
						if (!bWritingMessage) {

							WriteHeader();
						}
					});
			}

		private:
			// ASYNC - Prime context ready to read a message header
			void ReadHeader() {


				// If this function is called, we are expecting asio to wait until it receives
				// enough bytes to form a header of a message. We know the headers are a fixed
				// size, so allocate a transmission buffer large enough to store it. In fact, 
				// we will construct the message in a "temporary" message object as it's 
				// convenient to work with.
				asio::async_read(m_socket, asio::buffer(&m_msgTempIn.header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length) {

						if (debugRead) std::cout << "Reading a Header" << std::endl;
						
						if (!ec) {

							// A complete message header has been read, check if this message
							// has a body to follow...
							if (m_msgTempIn.header.size > 0) {

								// ...it does, so allocate enough space in the messages' body
								// vector, and issue asio with the task to read the body.
								m_msgTempIn.body.resize(m_msgTempIn.header.size);
								ReadBody();
							}
							else {

								// it doesn't, so add this bodyless message to the connections
								// incoming message queue
								AddToIncomingMessageQueue();
							}
						}
						else {

							// Reading form the client went wrong, most likely a disconnect
							// has occurred. Close the socket and let the system tidy it up later.
							std::cout << "[" << id << "] Read Header Fail.\n";
							m_socket.close();
						}
					});
			}

			// ASYNC - Prime context ready to read a message body
			void ReadBody() {



				// If this function is called, a header has already been read, and that header
				// request we read a body, The space for that body has already been allocated
				// in the temporary message object, so just wait for the bytes to arrive...
				asio::async_read(m_socket, asio::buffer(m_msgTempIn.body.data(), m_msgTempIn.body.size()),
					[this](std::error_code ec, std::size_t length) {

						if (debugRead) std::cout << "Reading a Body" << std::endl;

						if (!ec) {

							// ...and they have! The message is now complete, so add
							// the whole message to incoming queue
							AddToIncomingMessageQueue();
						}
						else {
							
							// As above
							std::cout << "[" << id << "] Read Body Fail.\n";
							m_socket.close();
						}
					});
			}

			// ASYNC - Prime context ready to write a message header
			void WriteHeader() {

				// If this function is called, we know the outgoing message queue must have 
				// at least one message to send. So allocate a transmission buffer to hold
				// the message, and issue the work - asio, send these bytes
				asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length) {

						if(debugWrite) std::cout << "Writing a Header" << std::endl;

						// asio has now sent the bytes - if there was a problem
						// an error would be available...
						if (!ec) {

							// ... no error, so check if the message header just sent also
							// has a message body...
							if (m_qMessagesOut.front().body.size() > 0) {

								// ...it does, so issue the task to write the body bytes
								WriteBody();
							}
							else {

								// ...it didnt, so we are done with this message. Remove it from 
								// the outgoing message queue
								m_qMessagesOut.pop_front();

								// If the queue is not empty, there are more messages to send, so
								// make this happen by issuing the task to send the next header.
								if (!m_qMessagesOut.empty()) {

									WriteHeader();
								}
							}
						}
						else {

							// ...asio failed to write the message, we could analyse why but 
							// for now simply assume the connection has died by closing the
							// socket. When a future attempt to write to this client fails due
							// to the closed socket, it will be tidied up.
							std::cout << "[" << id << "] Write Header Fail.\n";
							m_socket.close();
						}
					});
			}

			// ASYNC - Prime context ready to write a message body
			void WriteBody() {

				// If this function is called, a header has just been sent, and that header
				// indicated a body existed for this message. Fill a transmission buffer
				// with the body data, and send it!
				asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
					[this](std::error_code ec, std::size_t length) {

						if (debugWrite) std::cout << "Writing a Body" << std::endl;

						if (!ec) {

							// Sending was successful, so we are done with the message
							// and remove it from the queue
							m_qMessagesOut.pop_front();

							// If the queue still has messages in it, then issue the task to 
							// send the next messages' header.
							if (!m_qMessagesOut.empty()) {

								WriteHeader();
							}
						}
						else {

							// Sending failed, see WriteHeader() equivalent for description :P
							std::cout << "[" << id << "] Write Body Fail.\n";
							m_socket.close();
						}
					});
			}

			// Once a full message is received, add it to the incoming queue
			void AddToIncomingMessageQueue() {

				// Shove it in queue, converting it to an "owned message", by initialising
				// with the a shared pointer from this connection object
				if (m_nOwnerType == owner::server)
					m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTempIn });
				else
					m_qMessagesIn.push_back({ nullptr, m_msgTempIn });

				// We must now prime the asio context to receive the next message. It 
				// wil just sit and wait for bytes to arrive, and the message construction
				// process repeats itself. Clever huh?
				ReadHeader();
			}

			uint64_t scramble(uint64_t nInput) {
				uint64_t out = nInput ^ 0xDFE;
                return out;
			}

		protected:
			// Each connection has a unique socket to a remote
			asio::ip::tcp::socket m_socket;

			// This context is shared with the whole asio instance
			asio::io_context& m_asioContext;

			// This queue holds all messages to be sent to the remote side
			// of this connection
			tsqueue<message<T>> m_qMessagesOut;

			// This queue holds all messages that have been recieved from
			// the remote side of this connection. Note it is a reference
			// as the "owner" of this connection is expected to provide a queue
			tsqueue<owned_message<T>>& m_qMessagesIn;
			message<T> m_msgTempIn;

			// The "owner" decides how some of the connection behaves
			owner m_nOwnerType = owner::server;

			uint32_t id = 0;
		};
	}
}