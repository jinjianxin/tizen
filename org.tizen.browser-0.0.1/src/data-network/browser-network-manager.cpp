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


/**
  *@file browser-network-manager.cpp
  *@brief 网络监控与管理
  */


#include "browser-view.h"
#include "browser-network-manager.h"

Browser_Network_Manager::Browser_Network_Manager(void)
:	m_browser_view(NULL)
	,m_network_handle(NULL)
	,m_network_state(BROWSER_CONNECTION_TYPE_DISCONNECTED)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Network_Manager::~Browser_Network_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);

	deinit();
}
/**
 * @brief Browser_Network_Manager::init 初始化函数
 * @param browser_view Browser_View 对象
 * @return
 */
Eina_Bool Browser_Network_Manager::init(Browser_View *browser_view)
{
	BROWSER_LOGD("[%s]", __func__);

	if (browser_view)
		m_browser_view = browser_view;

	if (connection_create(&m_network_handle) < 0) {
		BROWSER_LOGE("Failed to creat m_network_handle");
		return EINA_FALSE;
	}

	if (connection_set_type_changed_cb(m_network_handle, __network_state_changed_ind_cb, this) < 0) {
		BROWSER_LOGE("Fail to set __network_state_changed_ind_cb");
		return EINA_FALSE;
	}

	if (connection_set_ip_address_changed_cb(m_network_handle, __network_configure_changed_ind_cb, this) < 0) {
		BROWSER_LOGE("Fail to set __network_configure_changed_ind_cb");
		return EINA_FALSE;
	}

	get_IP_address();
    get_proxy_address(); //获取代理地址

    m_network_state = get_network_connection_state(); //获取网络连接状态
	BROWSER_LOGD("get_proxy_address[%s]", m_proxy_address.c_str());

    /**
     *根据获得的状态判断是否使用代理
     */
	if (!m_proxy_address.length()) {
		ewk_context_proxy_uri_set(ewk_context_default_get(), NULL);
	} else {
		ewk_context_proxy_uri_set(ewk_context_default_get(), m_proxy_address.c_str());
	}

	return EINA_TRUE;
}

//删除设置的状态回调函数
void Browser_Network_Manager::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (connection_unset_type_changed_cb(m_network_handle) < 0)
		BROWSER_LOGE("Fail to unregister network state changed cb");

	if (connection_unset_ip_address_changed_cb(m_network_handle) < 0)
		BROWSER_LOGE("Fail to unregister ip address changed cb");

	if (connection_destroy(m_network_handle) < 0)
		BROWSER_LOGE("Fail to destroy network handle");

	return;
}

/**
 * @brief Browser_Network_Manager::get_IP_address 获得ip
 */
void Browser_Network_Manager::get_IP_address(void)
{
	char *IP_Addr = NULL;
	connection_address_family_e family = CONNECTION_ADDRESS_FAMILY_IPV4;

	if (!m_network_handle) {
		BROWSER_LOGE("m_network_handle is NULL");
		return;
	}

	if (connection_get_ip_address(m_network_handle, family, &IP_Addr) < 0) {
		BROWSER_LOGE("Fail to get ip address");
		return;
	}

	if (!IP_Addr || !strlen(IP_Addr)) {
		BROWSER_LOGD("Acquired ip address is invalid. set new IP address[0.0.0.0] from previous[%s]", m_ip_address.c_str());
		m_ip_address = std::string("0.0.0.0");
	} else {
		BROWSER_LOGD("Previous IP address : [%s], new IP address : [%s]", m_ip_address.c_str(), IP_Addr);
		m_ip_address = std::string(IP_Addr);
	}

	if (IP_Addr) {
		free(IP_Addr);
		IP_Addr = NULL;
	}

	return;
}

/**
 * @brief Browser_Network_Manager::get_proxy_address 获得代理地址
 */
void Browser_Network_Manager::get_proxy_address(void)
{
	char *proxy_Address = NULL;
	connection_address_family_e family = CONNECTION_ADDRESS_FAMILY_IPV4;

	if (!m_network_handle) {
		BROWSER_LOGE("m_network_handle is NULL");
		return;
	}

	if (connection_get_proxy(m_network_handle, family, &proxy_Address) < 0) {
		BROWSER_LOGE("Fail to get proxy address");
		return;
	}

	if (!proxy_Address || !strlen(proxy_Address)) {
		BROWSER_LOGD("Acquired proxy address is invalid. m_proxy_address is set as NULL from previous[%s]", m_proxy_address.c_str());
		m_proxy_address.clear();
	} else {
		BROWSER_LOGD("Previous proxy address : [%s], new proxy address : [%s]", m_proxy_address.c_str(), proxy_Address);
		m_proxy_address = std::string(proxy_Address);
	}

	if (proxy_Address) {
		free(proxy_Address);
		proxy_Address = NULL;
	}

	return;
}

/**
 * @brief Browser_Network_Manager::get_network_connection_state 获得当前网络状态
 * @return 网络连接标识
 */

browser_connection_type_e Browser_Network_Manager::get_network_connection_state(void)
{
	connection_type_e system_network_state = CONNECTION_TYPE_DISCONNECTED;
	browser_connection_type_e browser_network_state = BROWSER_CONNECTION_TYPE_DISCONNECTED;

	if (!m_network_handle) {
		BROWSER_LOGE("m_network_handle is NULL");
		return BROWSER_CONNECTION_TYPE_DISCONNECTED;
	}
	if (connection_get_type(m_network_handle, &system_network_state) < 0) {
		BROWSER_LOGD("Fail to get network status");
		return BROWSER_CONNECTION_TYPE_DISCONNECTED;
	}

	switch (system_network_state) {

		case CONNECTION_TYPE_DISCONNECTED:
			BROWSER_LOGD("Network state : CONNECTION_TYPE_DISCONNECTED");
			browser_network_state = BROWSER_CONNECTION_TYPE_DISCONNECTED;
			break;

		case CONNECTION_TYPE_WIFI:
			BROWSER_LOGD("Network state : CONNECTION_TYPE_WIFI");
			browser_network_state = get_network_wifi_state();
			break;

		case CONNECTION_TYPE_CELLULAR:
			BROWSER_LOGD("Network state : CONNECTION_TYPE_CELLULAR");
			browser_network_state = get_network_cellular_state();
			break;

		default:
			BROWSER_LOGD("Network state : Unable to get network status");
			browser_network_state = BROWSER_CONNECTION_TYPE_DISCONNECTED;
	}

	return browser_network_state;
}

/**
 * @brief Browser_Network_Manager::get_network_cellular_state 获得3g连接状态
 * @return
 */
browser_connection_type_e Browser_Network_Manager::get_network_cellular_state(void)
{
	connection_cellular_state_e system_network_cellular_state = CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
	browser_connection_type_e browser_network_cellular_state = BROWSER_CONNECTION_TYPE_DISCONNECTED;

	if (!m_network_handle) {
		BROWSER_LOGE("m_network_handle is NULL");
		return BROWSER_CONNECTION_TYPE_DISCONNECTED;
	}

	if (connection_get_cellular_state(m_network_handle, &system_network_cellular_state) < 0) {
		BROWSER_LOGD(" Fail to get cellular state");
		return BROWSER_CONNECTION_CELLULAR_STATE_UNAVAILABLE;
	}

	switch(system_network_cellular_state) {

		case CONNECTION_CELLULAR_STATE_AVAILABLE:
			BROWSER_LOGD("Cellular state : CONNECTION_CELLULAR_STATE_AVAILABLE");
			browser_network_cellular_state = BROWSER_CONNECTION_CELLULAR_STATE_AVAILABLE;
			break;

		case CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE:
			BROWSER_LOGD("Cellular state : CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE");
			browser_network_cellular_state = BROWSER_CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
			break;

		case CONNECTION_CELLULAR_STATE_FLIGHT_MODE:
			BROWSER_LOGD("Cellular state : CONNECTION_CELLULAR_STATE_FLIGHT_MODE");
			browser_network_cellular_state = BROWSER_CONNECTION_CELLULAR_STATE_FLIGHT_MODE;
			break;

		case CONNECTION_CELLULAR_STATE_ROAMING_OFF:
			BROWSER_LOGD("Cellular state : CONNECTION_CELLULAR_STATE_ROAMING_OFF");
			browser_network_cellular_state = BROWSER_CONNECTION_CELLULAR_STATE_ROAMING_OFF;
			break;

		case CONNECTION_CELLULAR_STATE_CALL_ONLY_AVAILABLE:
			BROWSER_LOGD("Cellular state : CONNECTION_CELLULAR_STATE_CALL_ONLY_AVAILABLE");
			browser_network_cellular_state = BROWSER_CONNECTION_CELLULAR_STATE_CALL_ONLY_AVAILABLE;
			break;

		default:
			BROWSER_LOGD("Cellular state : Index of network-cellular state is out of support");
			browser_network_cellular_state = BROWSER_CONNECTION_CELLULAR_STATE_UNAVAILABLE;
	}

	return browser_network_cellular_state;
}

/**
 * @brief Browser_Network_Manager::get_network_wifi_state 获得wifi状态
 * @return 返回网络连接标识
 */
browser_connection_type_e Browser_Network_Manager::get_network_wifi_state(void)
{
	connection_wifi_state_e system_network_wifi_state = CONNECTION_WIFI_STATE_DEACTIVATED;
	browser_connection_type_e browser_network_wifi_state = BROWSER_CONNECTION_WIFI_STATE_DISCONNECTED;

	if (!m_network_handle) {
		BROWSER_LOGE("m_network_handle is NULL");
		return BROWSER_CONNECTION_TYPE_DISCONNECTED;
	}

	if (connection_get_wifi_state(m_network_handle, &system_network_wifi_state) < 0) {
		BROWSER_LOGD(" Fail to get Wifi state");
		return BROWSER_CONNECTION_WIFI_STATE_DISCONNECTED;
	}

	switch(system_network_wifi_state) {

		case CONNECTION_WIFI_STATE_CONNECTED:
			BROWSER_LOGD("Wifi state : CONNECTION_WIFI_STATE_CONNECTED");
			browser_network_wifi_state = BROWSER_CONNECTION_WIFI_STATE_CONNECTED;
			break;

		case CONNECTION_WIFI_STATE_DEACTIVATED:
			BROWSER_LOGD("Wifi state : CONNECTION_WIFI_STATE_DEACTIVATED");
			browser_network_wifi_state = BROWSER_CONNECTION_WIFI_STATE_DEACTIVATED;
			break;

		case CONNECTION_WIFI_STATE_DISCONNECTED:
			BROWSER_LOGD("Wifi state : CONNECTION_WIFI_STATE_DISCONNECTED");
			browser_network_wifi_state = BROWSER_CONNECTION_WIFI_STATE_DISCONNECTED;
			break;

		default:
			BROWSER_LOGD("Wifi state : Index of network-Wifi state is out of support");
			browser_network_wifi_state = BROWSER_CONNECTION_WIFI_STATE_DISCONNECTED;
	}

	return browser_network_wifi_state;
}

/**
 * @brief Browser_Network_Manager::network_state_changed  网络状态改变时的回调函数
 */
void Browser_Network_Manager::network_state_changed(void)
{
	BROWSER_LOGD("[%s]", __func__);

	browser_connection_type_e browser_network_state = BROWSER_CONNECTION_TYPE_DISCONNECTED;
	browser_network_state = get_network_connection_state();

	if (m_network_state != browser_network_state) {
		BROWSER_LOGD("Previous network state[%d], Changed network state", m_network_state, browser_network_state);
		m_network_state = browser_network_state;
	} else {
		BROWSER_LOGD("Network state is not changed, Current[%d]", browser_network_state);
	}

	return;
}

/**
 * @brief Browser_Network_Manager::network_configure_changed 网络配置状态变化
 */

void Browser_Network_Manager::network_configure_changed(void)
{
	BROWSER_LOGD("[%s]", __func__);

	get_IP_address();
	get_proxy_address();

	BROWSER_LOGD("get_proxy_address[%s]", m_proxy_address.c_str());
	if (!m_proxy_address.length())
		ewk_context_proxy_uri_set(ewk_context_default_get(), NULL);
	else
		ewk_context_proxy_uri_set(ewk_context_default_get(), m_proxy_address.c_str());

	return;
}

void Browser_Network_Manager::__network_state_changed_ind_cb(connection_type_e system_network_state, void* user_data)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!user_data)
		return;

	Browser_Network_Manager *network_manager = (Browser_Network_Manager *)user_data;
	network_manager->network_state_changed();

	return;
}

void Browser_Network_Manager::__network_configure_changed_ind_cb(const char* ipv4_address, const char* ipv6_address, void* user_data)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!user_data)
		return;

	Browser_Network_Manager *network_manager = (Browser_Network_Manager *)user_data;
	network_manager->network_configure_changed();

	return;
}

