/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef BROWSER_NETWORK_MANAGER_H
#define BROWSER_NETWORK_MANAGER_H

#include "browser-config.h"
#include "net_connection.h"

typedef enum
{
	BROWSER_CONNECTION_TYPE_DISCONNECTED = 0,                   /**< Disconnected */

	BROWSER_CONNECTION_NETWORK_STATE_WIFI = 10,                 /**< Wi-Fi is used for default connection */
	BROWSER_CONNECTION_WIFI_STATE_DEACTIVATED,                  /**< Deactivated state */
	BROWSER_CONNECTION_WIFI_STATE_DISCONNECTED,                 /**< disconnected state */
	BROWSER_CONNECTION_WIFI_STATE_CONNECTED,                    /**< Connected state */

	BROWSER_CONNECTION_NETWORK_STATE_CELLULAR = 20,             /**< Cellular is used for default connection */
	BROWSER_CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE,           /**< Out of service */
	BROWSER_CONNECTION_CELLULAR_STATE_FLIGHT_MODE,              /**< Flight mode */
	BROWSER_CONNECTION_CELLULAR_STATE_ROAMING_OFF,              /**< Roaming is turned off */
	BROWSER_CONNECTION_CELLULAR_STATE_CALL_ONLY_AVAILABLE,      /**< Call is only available. */
	BROWSER_CONNECTION_CELLULAR_STATE_AVAILABLE,                /**< Available */
	BROWSER_CONNECTION_CELLULAR_STATE_UNAVAILABLE,              /**< Unavailable */

} browser_connection_type_e;


class Browser_View;
class Browser_Network_Manager {
public:
	Browser_Network_Manager(void);
	~Browser_Network_Manager(void);

	Eina_Bool init(Browser_View *browser_view);
	void deinit(void);

private:
	void get_IP_address(void);
	void get_proxy_address(void);
	browser_connection_type_e get_network_connection_state(void);
	browser_connection_type_e get_network_cellular_state(void);
	browser_connection_type_e get_network_wifi_state(void);
	void network_state_changed(void);
	void network_configure_changed(void);

	static void __network_state_changed_ind_cb(connection_type_e system_network_state, void* user_data);
	static void __network_configure_changed_ind_cb(const char* ipv4_address, const char* ipv6_address, void* user_data);

	Browser_View *m_browser_view;
	connection_h m_network_handle;
	browser_connection_type_e m_network_state;

	std::string m_ip_address;
	std::string m_proxy_address;
};
#endif /* BROWSER_NETWORK_MANAGER_H */

