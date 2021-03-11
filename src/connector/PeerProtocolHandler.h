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
#ifndef PEER_PROTOCOL_HANDLER_H_
#define PEER_PROTOCOL_HANDLER_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <json/json.h>
#include <jsonrpccpp/common/procedure.h>
#include <jsonrpccpp/server/iprocedureinvokationhandler.h>
#include <jsonrpccpp/server/requesthandlerfactory.h>

#include "../interface/IClientPeerConnectionHandler.h"
#include "../interface/IPeerServerHandler.h"

namespace rpcserver {

/***************************************************************
 *
 * *************************************************************/
template<typename H>
class PeerProtocolHandler
  : public IClientPeerConnectionHandler<H>
  , public jsonrpc::IProcedureInvokationHandler
{
public:
  /***************************************************************
   *
   * *************************************************************/
  explicit PeerProtocolHandler(IPeerServerHandler<H>* peerHandler,
                               jsonrpc::serverVersion_t type = jsonrpc::JSONRPC_SERVER_V2)
    : m_peerHandler(peerHandler)
  {
    m_handler = std::unique_ptr<jsonrpc::IProtocolHandler>(
      jsonrpc::RequestHandlerFactory::createProtocolHandler(type, *this));

    // Populate methods in real protocol handler
    if (m_peerHandler) {
      auto methods = m_peerHandler->getMethodList();
      for (auto& name : methods) {
        m_handler->AddProcedure(
          jsonrpc::Procedure(name, jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT, NULL));
      }

      auto notifications = m_peerHandler->getNotificationsList();
      for (auto& name : notifications) {
        m_handler->AddProcedure(jsonrpc::Procedure(name, jsonrpc::PARAMS_BY_NAME, NULL));
      }
    }
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual ~PeerProtocolHandler() = default;

  /***************************************************************
   *
   * *************************************************************/
  virtual void HandlePeerRequest(const H& clientCtx,
                                 const std::string& request,
                                 std::string& retValue) override
  {
    std::lock_guard<std::mutex> lock(m_clientLock);
    m_clientCtx = clientCtx;
    m_handler->HandleRequest(request, retValue);
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void HandleMethodCall(jsonrpc::Procedure& proc,
                                const Json::Value& input,
                                Json::Value& output)
  {
    if (m_peerHandler) {
      m_peerHandler->HandleMethodCall(proc.GetProcedureName(), m_clientCtx, input, output);
    }
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void HandleNotificationCall(jsonrpc::Procedure& proc, const Json::Value& input)
  {
    if (m_peerHandler) {
      m_peerHandler->HandleNotificationCall(proc.GetProcedureName(), m_clientCtx, input);
    }
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void onPeerConnected(const H& clientCtx) override
  {
    if (m_peerHandler) {
      m_peerHandler->onPeerConnected(clientCtx);
    }
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void onPeerDisconnected(const H& clientCtx) override
  {
    if (m_peerHandler) {
      m_peerHandler->onPeerDisconnected(clientCtx);
    }
  }

protected:
  IPeerServerHandler<H>* m_peerHandler;
  std::unique_ptr<jsonrpc::IProtocolHandler> m_handler;
  std::mutex m_clientLock;
  H m_clientCtx;
};

} // namespace rpcserver

#endif // PEER_PROTOCOL_HANDLER_H_
