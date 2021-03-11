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

#ifndef I_CLIENT_PEER_CONNECTION_HANDLER_H_
#define I_CLIENT_PEER_CONNECTION_HANDLER_H_

#include <string>

namespace rpcserver {

/***************************************************************
 *
 * *************************************************************/
template<typename H>
class IClientPeerConnectionHandler
{
public:
  /***************************************************************
   *
   * *************************************************************/
  virtual ~IClientPeerConnectionHandler() = default;

  /***************************************************************
   *
   * *************************************************************/
  virtual void HandlePeerRequest(const H& clientCtx,
                                 const std::string& request,
                                 std::string& retValue) = 0;

  /***************************************************************
   *
   * *************************************************************/
  virtual void onPeerConnected(const H& clientCtx) = 0;

  /***************************************************************
   *
   * *************************************************************/
  virtual void onPeerDisconnected(const H& clientCtx) = 0;
};

} // namespace rpcserver

#endif // I_CLIENT_PEER_CONNECTION_HANDLER_H_
