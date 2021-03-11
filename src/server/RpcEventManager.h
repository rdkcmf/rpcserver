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
#ifndef RPC_EVENT_MANAGER_H_
#define RPC_EVENT_MANAGER_H_

#include <json/json.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../interface/IRpcEventManagerListener.h"

namespace rpcserver {

/***************************************************************
 *
 * *************************************************************/
template<typename H, typename C>
class RpcEventManager
{
public:
  /***************************************************************
   *
   * *************************************************************/
  explicit RpcEventManager() = default;

  /***************************************************************
   *
   * *************************************************************/
  virtual ~RpcEventManager() = default;

  /***************************************************************
   *
   * *************************************************************/
  void onEvent(const std::string& name, const Json::Value& eventInfo)
  {
    doEmitEvent(name, eventInfo);
  }

  /***************************************************************
   *
   * *************************************************************/
  void setEventListener(std::shared_ptr<IRpcEventManagerListener<H>> eventListener)
  {
    m_eventListener = eventListener;
  }

  /***************************************************************
   * Register event listener
   *      event - event name
   *      id    - client id
   * *************************************************************/
  bool registerToEvent(const H& clientCtx, std::string event, std::string id)
  {
    std::lock_guard<std::mutex> lock(m_regsLock);

    auto& regs = m_registrations[event];
    for (auto regIt = regs.begin(); regIt != regs.end(); regIt++) {
      if (areCtxEqual(clientCtx, regIt->m_clientCtx)) {
        return false;
      }
    }
    RegistrationInfo subInfo = { id, clientCtx };
    regs.push_back(subInfo);
    return true;
  }

  /***************************************************************
   * Unregister event listener
   *      event - event name
   *      id    - client id
   * *************************************************************/
  bool unregisterFromEvent(const H& clientCtx, std::string event, const std::string& id)
  {
    std::lock_guard<std::mutex> lock(m_regsLock);

    bool found = false;
    auto& regs = m_registrations[event];
    for (auto regIt = regs.begin(); regIt != regs.end(); regIt++) {
      if (id == regIt->m_id && areCtxEqual(clientCtx, regIt->m_clientCtx)) {
        regIt = regs.erase(regIt);
        found = true;
        break;
      }
    }
    if (regs.empty()) {
      m_registrations.erase(event);
    }

    return found;
  }

  /***************************************************************
   * Unregister connection
   * *************************************************************/
  void unregisterConn(const H& clientCtx)
  {
    std::lock_guard<std::mutex> lock(m_regsLock);

    for (auto it = m_registrations.begin(); it != m_registrations.end();) {
      auto& regs = it->second;
      for (auto regIt = regs.begin(); regIt != regs.end();) {
        if (areCtxEqual(clientCtx, regIt->m_clientCtx)) {
          regIt = regs.erase(regIt);
        } else {
          ++regIt;
        }
      }
      if (regs.empty()) {
        it = m_registrations.erase(it);
      } else {
        ++it;
      }
    }
  }

protected:
  /***************************************************************
   *
   * *************************************************************/
  struct RegistrationInfo
  {
    std::string m_id;
    H m_clientCtx;
  };

  /***************************************************************
   *
   * *************************************************************/
  bool areCtxEqual(const H& t, const H& u)
  {
    return !m_ctxComparator(t, u) && !m_ctxComparator(u, t);
  }

  /***************************************************************
   *
   * *************************************************************/
  Json::Value createEventParams(const Json::Value& eventInfo)
  {
    if (eventInfo.isObject() || eventInfo.isArray() || eventInfo.isNull()) {
      return eventInfo;
    } else {
      auto params{ Json::Value(Json::arrayValue) };
      params[0u] = eventInfo;
      return params;
    }
  }

  /***************************************************************
   *
   * *************************************************************/
  void doEmitEvent(const std::string& eventName, const Json::Value& eventInfo)
  {
    std::lock_guard<std::mutex> lock(m_regsLock);

    if (m_eventListener) {
      if (m_registrations.count(eventName) > 0) {
        auto& subs = m_registrations[eventName];
        for (auto it = subs.begin(); it != subs.end(); it++) {
          std::string method = it->m_id + "." + eventName;
          m_eventListener->onConnEvent(it->m_clientCtx, method, createEventParams(eventInfo));
        }
      }
    }
  }

  C m_ctxComparator;
  std::mutex m_regsLock;
  std::map<std::string, std::vector<RegistrationInfo>> m_registrations;
  std::shared_ptr<IRpcEventManagerListener<H>> m_eventListener = nullptr;
};

} // namespace rpcserver

#endif // RPC_EVENT_MANAGER_H_
