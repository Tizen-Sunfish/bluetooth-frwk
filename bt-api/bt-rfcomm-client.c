/*
 * bluetooth-frwk
 *
 * Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <string.h>

#include "bluetooth-api.h"
#include "bt-internal-types.h"

#include "bt-common.h"
#include "bt-request-sender.h"
#include "bt-event-handler.h"

BT_EXPORT_API int bluetooth_rfcomm_connect(const bluetooth_device_address_t *remote_bt_address,
						const char *remote_uuid)
{
	int result;
	int connect_type;
	bt_user_info_t *user_info;
	char uuid[BLUETOOTH_UUID_STRING_MAX];

	BT_CHECK_PARAMETER(remote_bt_address, return);
	BT_CHECK_PARAMETER(remote_uuid, return);
	BT_CHECK_ENABLED(return);

	

	user_info = _bt_get_user_data(BT_COMMON);
	retv_if(user_info->cb == NULL, BLUETOOTH_ERROR_INTERNAL);

	/* connect_type:  BT_RFCOMM_UUID / BT_RFCOMM_CHANNEL*/
	/* In now, we only support to connecty using UUID */
	connect_type = BT_RFCOMM_UUID;

	BT_INIT_PARAMS();
	BT_ALLOC_PARAMS(in_param1, in_param2, in_param3, in_param4, out_param);

	g_array_append_vals(in_param1, remote_bt_address,
				sizeof(bluetooth_device_address_t));

	g_strlcpy(uuid, remote_uuid, sizeof(uuid));
	g_array_append_vals(in_param2, uuid, BLUETOOTH_UUID_STRING_MAX);

	g_array_append_vals(in_param3, &connect_type, sizeof(int));

	result = _bt_send_request_async(BT_BLUEZ_SERVICE,
				BT_RFCOMM_CLIENT_CONNECT,
				in_param1, in_param2,
				in_param3, in_param4,
				user_info->cb, user_info->user_data);

	BT_DBG("result: %x", result);

	BT_FREE_PARAMS(in_param1, in_param2, in_param3, in_param4, out_param);

	return result;
}

BT_EXPORT_API gboolean bluetooth_rfcomm_is_client_connected(void)
{
	int result;
	int connected = FALSE;

	BT_CHECK_ENABLED(return);

	BT_INIT_PARAMS();
	BT_ALLOC_PARAMS(in_param1, in_param2, in_param3, in_param4, out_param);

	result = _bt_send_request(BT_BLUEZ_SERVICE, BT_RFCOMM_CLIENT_IS_CONNECTED,
		in_param1, in_param2, in_param3, in_param4, &out_param);

	BT_DBG("result: %x", result);

	if (result == BLUETOOTH_ERROR_NONE) {
		connected = g_array_index(out_param,
				int, 0);
	} else {
		BT_ERR("Fail to send request");
	}

	BT_FREE_PARAMS(in_param1, in_param2, in_param3, in_param4, out_param);

	return connected;
}

BT_EXPORT_API int bluetooth_rfcomm_disconnect(int socket_fd)
{
	int result;
	int service_function;

	BT_CHECK_ENABLED(return);

	BT_INIT_PARAMS();
	BT_ALLOC_PARAMS(in_param1, in_param2, in_param3, in_param4, out_param);

	/* Support the OSP */
	if (socket_fd == -1) {
		/* Cancel connect */
		service_function = BT_RFCOMM_CLIENT_CANCEL_CONNECT;
	} else {
		g_array_append_vals(in_param1, &socket_fd, sizeof(int));
		service_function = BT_RFCOMM_SOCKET_DISCONNECT;
	}

	result = _bt_send_request(BT_BLUEZ_SERVICE, service_function,
		in_param1, in_param2, in_param3, in_param4, &out_param);

	BT_DBG("result: %x", result);

	BT_FREE_PARAMS(in_param1, in_param2, in_param3, in_param4, out_param);

	return result;
}

BT_EXPORT_API int bluetooth_rfcomm_write(int fd, const char *buf, int length)
{
	int result;
	char *buffer;

	BT_CHECK_PARAMETER(buf, return);
	BT_CHECK_ENABLED(return);
	retv_if(length <= 0, BLUETOOTH_ERROR_INVALID_PARAM);

	BT_INIT_PARAMS();
	BT_ALLOC_PARAMS(in_param1, in_param2, in_param3, in_param4, out_param);

	buffer = g_malloc0(length + 1);

	memcpy(buffer, buf, length + 1);
	//g_strlcpy(buffer, buf, length + 1);

	g_array_append_vals(in_param1, &fd, sizeof(int));
	g_array_append_vals(in_param2, &length, sizeof(int));
	g_array_append_vals(in_param3, buffer, length);

	result = _bt_send_request(BT_BLUEZ_SERVICE, BT_RFCOMM_SOCKET_WRITE,
		in_param1, in_param2, in_param3, in_param4, &out_param);

	BT_DBG("result: %x", result);

	BT_FREE_PARAMS(in_param1, in_param2, in_param3, in_param4, out_param);

	g_free(buffer);

	return result;
}

