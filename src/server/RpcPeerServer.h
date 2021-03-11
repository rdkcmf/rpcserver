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
#ifndef RPC_PEER_SERVER_H_
#define RPC_PEER_SERVER_H_

#include <json/json.h>
#include <jsonrpccpp/common/errors.h>
#include <jsonrpccpp/common/exception.h>

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../interface/IPeerServerConnector.h"
#include "../interface/IPeerServerHandler.h"
#include "../interface/IRpcEventManagerListener.h"
#include "RpcEventManager.h"

namespace rpcserver {

/***************************************************************
 *
 * *************************************************************/
template<typename H, typename C>
class RpcPeerServer : public IPeerServerHandler<H>
{
private:
  static constexpr auto RPC_METHOD_PARAM_EVENT = "event";
  static constexpr auto RPC_METHOD_PARAM_ID = "id";

public:
  /***************************************************************
   *
   * *************************************************************/
  RpcPeerServer(AbstractPeerServerConnector<H>& connector,
                const std::string& registerMethodName,
                const std::string& unregisterMethodName)
    : m_connector(connector)
    , m_registerMethodName(registerMethodName)
    , m_unregisterMethodName(unregisterMethodName)
  {
    if (!m_registerMethodName.empty() && m_unregisterMethodName.empty()) {
      throw std::invalid_argument("'register' method is provided but 'unregister' is not");
    } else if (m_registerMethodName.empty() && !m_unregisterMethodName.empty()) {
      throw std::invalid_argument("'unregister method is provided but 'register' is not");
    }

    m_connector.setPeerHandler(this);

    // Initialize event manager
    m_eventManager = std::unique_ptr<RpcEventManager<H, C>>(new RpcEventManager<H, C>());
    m_eventManager->setEventListener(std::make_shared<EventManagerListener<H>>(this));
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual ~RpcPeerServer() = default;

  /***************************************************************
   *
   * *************************************************************/
  virtual std::vector<std::string> getMethodList() override
  {
    std::vector<std::string> result;
    if (!m_registerMethodName.empty()) {
      result.push_back(m_registerMethodName);
    }
    if (!m_unregisterMethodName.empty()) {
      result.push_back(m_unregisterMethodName);
    }
    for (auto& pair : m_methodTable) {
      result.push_back(pair.first);
    }
    return result;
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual std::vector<std::string> getNotificationsList() override
  {
    std::vector<std::string> result;
    for (auto& pair : m_notificationsTable) {
      result.push_back(pair.first);
    }
    return result;
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual bool bindMethod(const std::string& name,
                          std::function<void(const Json::Value&, Json::Value&)> method)
  {
    if (name.empty()) {
      throw std::invalid_argument("Method name should not be empty");
    }
    if (!symbolExists(name)) {
      m_methodTable[name] = method;
      return true;
    }
    return false;
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual bool bindNotification(const std::string& name,
                                std::function<void(const Json::Value&)> method)
  {
    if (name.empty()) {
      throw std::invalid_argument("Notification name should not be empty");
    }
    if (!symbolExists(name)) {
      m_notificationsTable[name] = method;
      return true;
    }
    return false;
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void HandleMethodCall(const std::string& name,
                                const H& connHdl,
                                const Json::Value& input,
                                Json::Value& output) override
  {
    if (name == m_registerMethodName) {
      HandleRegister(connHdl, input, output);
    } else if (name == m_unregisterMethodName) {
      HandleUnregister(connHdl, input, output);
    } else {
      m_methodTable.at(name)(input, output);
    }
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void HandleNotificationCall(const std::string& name,
                                      const H& clientCtx,
                                      const Json::Value& input) override
  {
    m_notificationsTable.at(name)(input);
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void onPeerConnected(const H& clientCtx) override {}

  /***************************************************************
   *
   * *************************************************************/
  virtual void onPeerDisconnected(const H& clientCtx) override
  {
    m_eventManager->unregisterConn(clientCtx);
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void sendNotification(const H& clientCtx,
                                const std::string& name,
                                const Json::Value& params)
  {
    m_connector.sendNotification(clientCtx, name, params);
  }

  /***************************************************************
   *
   * *************************************************************/
  void onEvent(const std::string& name, const Json::Value& eventInfo)
  {
    m_eventManager->onEvent(name, eventInfo);
  }

protected:
  /***************************************************************
   *
   * *************************************************************/
  template<typename X>
  class EventManagerListener : public IRpcEventManagerListener<X>
  {
  private:
    RpcPeerServer* m_parent;

  public:
    /***************************************************************
     *
     * *************************************************************/
    explicit EventManagerListener(RpcPeerServer* parent)
      : m_parent(parent)
    {}

    /***************************************************************
     *
     * *************************************************************/
    virtual ~EventManagerListener() {}

    /***************************************************************
     *
     * *************************************************************/
    virtual void onConnEvent(const X& clientCtx,
                             const std::string& name,
                             const Json::Value& eventData) override
    {
      m_parent->sendNotification(clientCtx, name, eventData);
    }
  };

  /***************************************************************
   *
   * *************************************************************/
  virtual void HandleRegister(const H& clientCtx, const Json::Value& input, Json::Value& output)
  {
    if (input.size() >= 2 && input.isMember(RPC_METHOD_PARAM_EVENT) &&
        input.isMember(RPC_METHOD_PARAM_ID) && input[RPC_METHOD_PARAM_EVENT].isString() &&
        input[RPC_METHOD_PARAM_ID].isString()) {
      std::string event = input[RPC_METHOD_PARAM_EVENT].asString();
      std::string id = input[RPC_METHOD_PARAM_ID].asString();

      int result = m_eventManager->registerToEvent(clientCtx, event, id);
      if (!result) {
        throw jsonrpc::JsonRpcException(-1,
                                        "Only one registration per event type per each connection");
      } else {
        output = 0;
      }
    } else {
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_RPC_INVALID_PARAMS,
                                      "Wrong parameters");
    }
  }

  /***************************************************************
   *
   * *************************************************************/
  virtual void HandleUnregister(const H& clientCtx, const Json::Value& input, Json::Value& output)
  {
    if (input.size() >= 2 && input.isMember(RPC_METHOD_PARAM_EVENT) &&
        input.isMember(RPC_METHOD_PARAM_ID) && input[RPC_METHOD_PARAM_EVENT].isString() &&
        input[RPC_METHOD_PARAM_ID].isString()) {
      std::string event = input[RPC_METHOD_PARAM_EVENT].asString();
      std::string id = input[RPC_METHOD_PARAM_ID].asString();

      bool result = m_eventManager->unregisterFromEvent(clientCtx, event, id);
      if (!result) {
        throw jsonrpc::JsonRpcException(-1, "Registration info not found");
      } else {
        output = 0;
      }
    } else {
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_RPC_INVALID_PARAMS,
                                      "Wrong parameters");
    }
  }

  /***************************************************************
   *
   * *************************************************************/
  bool symbolExists(const std::string& name)
  {
    if (name == m_registerMethodName || name == m_unregisterMethodName) {
      return true;
    }
    if (m_methodTable.find(name) != m_methodTable.end()) {
      return true;
    }
    if (m_notificationsTable.find(name) != m_notificationsTable.end()) {
      return true;
    }
    return false;
  }

  AbstractPeerServerConnector<H>& m_connector;
  std::string m_registerMethodName;
  std::string m_unregisterMethodName;
  std::unique_ptr<RpcEventManager<H, C>> m_eventManager;
  std::map<std::string, std::function<void(const Json::Value&, Json::Value&)>> m_methodTable;
  std::map<std::string, std::function<void(const Json::Value&)>> m_notificationsTable;
};

} // namespace rpcserver

#endif // RPC_PEER_SERVER_H_
