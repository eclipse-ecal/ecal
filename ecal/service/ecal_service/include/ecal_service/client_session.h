/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal_service/client_session_types.h>
#include <ecal_service/error.h>
#include <ecal_service/logger.h>
#include <ecal_service/state.h>

namespace ecal_service
{
  // Forward declarations
  class ClientSessionBase;

  /**
   * @brief The eCAL::sercice::ClientSession class represents the binary service client session.
   * 
   * The ClientSession can connect to exactly one service server. For multiple
   * connections, multiple ClientSession instances are required.
   * 
   * The ClientSession needs an io_context to run. The io_context is not owned
   * by the ClientSession and must be kept alive by the user, e.g. by a work
   * object.
   * 
   * Upon creation, the ClientSession will connect to a server via the given
   * address and port. Both synchronous and asynchronous service calls are
   * available. The async service callbacks are executed by the io_context
   * thread. Therefore, the callbacks must not block the io_context thread for
   * too long.
   * 
   * =========================================================================
   * _Important_: Do not stop the io_context while the client session is
   *              running. This may cause undefined behavior. Instead, call
   *              stop() and wait for the io_context to run out of work on its
   *              own.
   * 
   * _Tip_:       Use the ecal_service::ClientManager class to manage the
   *              client's lifecycle. This enables you to stop all client
   *              sessions from a single place when performing an application
   *              shutdown.
   * =========================================================================
   * 
   * Sample conde:
   * 
   * @code{.cpp}
   * 
   * // Create an io_context and a work object to keep it alive
   * auto io_context = std::make_shared<asio::io_context>();
   * asio::io_context::work work(*io_context);
   * 
   * // Creat a thread for the io_context
   * std::thread io_context_thread([&io_context]() { io_context->run(); });
   * 
   * // Create client sessions
   * auto client_session = ecal_service::ClientSession::create(io_context, ...)
   * 
   * // call a service asynchronously
   * client_session->async_call_service(...);
   * 
   * // DO STUFF
   * 
   * // Stop the client session
   * client_session->stop();
   * 
   * // Wait for the io_context to run out of work
   * io_context_thread.join();
   * 
   * @endcode
   */
  class ClientSession
  {
  //////////////////////////////////////////////
  // Internal types for better consistency
  //////////////////////////////////////////////
  public:
    using EventCallbackT    = ClientEventCallbackT;
    using ResponseCallbackT = ClientResponseCallbackT;
    using DeleteCallbackT   = std::function<void(ClientSession*)>;

  //////////////////////////////////////////////
  // Constructor, Destructor, Create
  //////////////////////////////////////////////
  public:
    /**
     * @brief Creates a new ClientSession instance.
     * 
     * A new ClientSession will immediatelly start connecting to a server on
     * the given address and port.
     * 
     * =========================================================================
     * _Important_: Do not stop the io_context while the client session is
     *              running. This may cause undefined behavior. Instead, call
     *              stop() and wait for the io_context to run out of work on its
     *              own.
     * 
     * _Tip_:       Use the ecal_service::ClientManager class to manage the
     *              client's lifecycle. This enables you to stop all client
     *              sessions from a single place when performing an application
     *              shutdown.
     * =========================================================================
     * 
     * @param io_context        The io_context to use for the session and all callbacks.
     * @param protocol_version  The protocol version to use for the session. When this is 0, the legacy buggy protocol is used.
     * @param server_list       A list of endpoints to connect to. Must not be empty. The endpoints will be tried in the given order until a working endpoint is found.
     * @param event_callback    The callback to be called when the session's state changes, i.e. when the session successfully connected to a server or disconnected from it.
     * @param logger            The logger to use for logging.
     * @param delete_callback   The callback to be called when the session is deleted. This is useful for the ecal_service::ClientManager to keep track of the number of active sessions.
     * 
     * @return The new ClientSession instance as a shared_ptr.
     */
    static std::shared_ptr<ClientSession> create(const std::shared_ptr<asio::io_context>&                   io_context
                                                , std::uint8_t                                              protocol_version
                                                , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                                                , const EventCallbackT&                                     event_callback
                                                , const LoggerT&                                            logger
                                                , const DeleteCallbackT&                                    delete_callback);

    static std::shared_ptr<ClientSession> create(const std::shared_ptr<asio::io_context>&                   io_context
                                                , std::uint8_t                                              protocol_version
                                                , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                                                , const EventCallbackT&                                     event_callback
                                                , const LoggerT&                                            logger = default_logger("Service Client"));

    static std::shared_ptr<ClientSession> create(const std::shared_ptr<asio::io_context>&                   io_context
                                                , std::uint8_t                                              protocol_version
                                                , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                                                , const EventCallbackT&                                     event_callback
                                                , const DeleteCallbackT&                                    delete_callback);

  protected:
    ClientSession(const std::shared_ptr<asio::io_context>&                    io_context
                  , std::uint8_t                                              protocol_version
                  , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                  , const EventCallbackT&                                     event_callback
                  , const LoggerT&                                            logger);

  public:
    // Delete copy constructor and assignment operator
    ClientSession(const ClientSession&)            = delete;
    ClientSession& operator=(const ClientSession&) = delete;

    // Delete move constructor and assignment operator
    ClientSession(ClientSession&&)            = delete;
    ClientSession& operator=(ClientSession&&) = delete;

    ~ClientSession();

  //////////////////////////////////////////////
  // Public API
  //////////////////////////////////////////////
  public:
    /**
     * @brief Calls the server asynchronously.
     * 
     * This function will call the server asynchronously. The response_callback
     * will be called when the server responds. The response_callback will be
     * called from the io_context thread. Therefore, the response_callback must
     * not block the io_context thread for too long.
     * 
     * When an error occurs, the response_callback will be called with an error.
     * 
     * When the client has been stopped manually, this function will return
     * false. In that case, the response_callback will not be called, as there
     * is no way to make sure, that the io_context is still running.
     * 
     * @param request           The request to send to the server.
     * @param response_callback The callback to be called when the server responds or an error occurs.
     * 
     * @return true if the request was sent enqueued successfully, false otherwise. If this returns false, the response_callback will not be called.
     */
    bool async_call_service(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_callback);

    /**
     * @brief Calls the server synchronously.
     * 
     * This function will call the server synchronously. The function will block
     * until the server responds or an error occurs. The response will be
     * written to the response parameter
     * 
     * If this function deadlocks, you must check that the io_context is
     * running, as that is required for the this function to un-block. The
     * io_context must therefore never been stopped. Instead, call stop() and
     * wait for the io_context to run out of work on its own.
     * 
     * @param request   The request to send to the server.
     * @param response  The server's response
     * 
     * @return The error that occured or ecal_service::Error::OK if no error occured.
     */
    ecal_service::Error call_service(const std::shared_ptr<const std::string>& request, std::shared_ptr<std::string>& response);

    /**
     * @brief Get the host that this client is connected to.
     * 
     * Get the host that this client is connected to.
     * If the client is not connected, this function will return an empty
     * string. Otherwise, it will return the hostname from the list
     * server_list that the client is connected to.
     * 
     * The host is not resolved to an IP address. Use get_remote_endpoint()
     * to get the actual IP address.
     * 
     * @return The host that this client is connected to.
     */
    std::string get_host() const;

    /**
     * @brief Get the port that this client session is connected to.
     * 
     * Get the port that this client session is connected to. If the client
     * is not connected, this function will return 0. Otherwise, it will
     * return the port from the list server_list that the client is connected
     * to.
     * 
     * @return The port that this client is connected to
     */
    std::uint16_t get_port()                     const;

    /**
     * @brief Get the remote endpoint that this client session is connected to.
     * 
     * Get the remote endpoint that this client session is connected to. Only
     * valid, if the client session is actually connected to a server. If a
     * hostname was given, this function will return the resolved IP address.
     */
    asio::ip::tcp::endpoint get_remote_endpoint() const;

    /**
     * @brief Get the state of this client session.
     * 
     * @return the state of this client session.
     */
    State         get_state()                     const;

    /**
     * @brief Get the accepted protocol version that the server and client have agreed on.
     * 
     * If the connection hasn't been established yet, this function will return 0.
     * 
     * @return The accepted protocol version that the server and client have agreed on.
     */
    std::uint8_t  get_accepted_protocol_version() const;

    /**
     * @brief Get the number of pending requests
     * 
     * @return The number of pending requests
     */
    int           get_queue_size()                const;

    /**
     * @brief Stops the client session.
     * 
     * This function will stop the client session. The client session will
     * disconnect from the server and will not accept any new requests. Any
     * further calls to async_call_service() will fail with an error.
     * 
     * Once the client session has been stopped, it cannot be restarted. You
     * must create a new client session instead.
     */
    void          stop();

  //////////////////////////////////////////////
  // Member Variables
  //////////////////////////////////////////////
  private:
    std::shared_ptr<ClientSessionBase> impl_;       //!< The implementation of the client session
  };
} // namespace ecal_service
