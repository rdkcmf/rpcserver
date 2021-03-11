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
#ifndef ABSTRACT_PEER_SERVER_CONNECTOR_H_
#define ABSTRACT_PEER_SERVER_CONNECTOR_H_

#include <string>

#include <json/json.h>

#include "../interface/IPeerServerConnector.h"
#include "../interface/IPeerServerHandler.h"

namespace rpcserver {

/***************************************************************
 *
 * *************************************************************/
template<typename H>
class AbstractPeerServerConnector : public IPeerServerConnector<H>
{
public:
  /***************************************************************
   *
   * *************************************************************/
  AbstractPeerServerConnector() {}

  /***************************************************************
   *
   * *************************************************************/
  virtual ~AbstractPeerServerConnector() = default;

  /***************************************************************
   *
   * *************************************************************/
  virtual void setPeerHandler(IPeerServerHandler<H>* peerServerHandler) override
  {
    m_peerServerHandler = peerServerHandler;
  }

protected:
  IPeerServerHandler<H>* m_peerServerHandler = nullptr;
};

} // namespace rpcserver

#endif // ABSTRACT_PEER_SERVER_CONNECTOR_H_
