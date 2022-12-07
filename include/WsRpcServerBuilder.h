/*
	* If not stated otherwise in this file or this component's LICENSE file the
	* following copyright and licenses apply:
	*
	* Copyright 2021 Liberty Global Service BV
	*
	* Licensed under the Apache License, Version 2.0 (the "License");
	* you may not use this file except in compliance with the License.
	* You may obtain a copy of the License at
	*
	* http://www.apache.org/licenses/LICENSE-2.0
	*
	* Unless required by applicable law or agreed to in writing, software
	* distributed under the License is distributed on an "AS IS" BASIS,
	* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	* See the License for the specific language governing permissions and
	* limitations under the License.
	*/
#ifndef WS_RPC_SERVER_BUILDER_H_
#define WS_RPC_SERVER_BUILDER_H_

#include <cstdint>
#include <string>

#include "IRpcServerBuilder.h"

namespace rpcserver {

/***************************************************************
 *
 * *************************************************************/
class WsRpcServerBuilder : IRpcServerBuilder
{
public:
  WsRpcServerBuilder(uint16_t port, bool ipv4only = false);

  WsRpcServerBuilder(const std::string& host, uint16_t port);

  virtual ~WsRpcServerBuilder();

  WsRpcServerBuilder& enableServerEvents(const std::string& registerMethodName,
                                   const std::string& unregisterMethodName,
                                   const std::string& getListenersMethodName);

  WsRpcServerBuilder& numThreads(std::size_t numThreads);

  WsRpcServerBuilder& sock_reuse_addr(bool sock_reuse_addr);

  virtual IAbstractRpcServer* build() const override;

protected:
  std::string m_host;
  uint16_t m_port = 0;
  bool m_ipv4only = false;

  std::string m_registerMethodName;
  std::string m_unregisterMethodName;
  std::string m_getListenersMethodName;
  std::size_t m_numThreads = 1;
  bool m_sock_reuse_addr = true;
};

} // namespace rpcserver

#endif // WS_RPC_SERVER_BUILDER_H_
