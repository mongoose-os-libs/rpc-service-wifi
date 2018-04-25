/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>

#include "common/json_utils.h"
#include "mgos.h"
#include "mgos_rpc.h"

static int wifi_scan_result_printer(struct json_out *out, va_list *ap) {
  int len = 0;
  int num_res = va_arg(*ap, int);

  const struct mgos_wifi_scan_result *res =
      va_arg(*ap, const struct mgos_wifi_scan_result *);

  for (int i = 0; i < num_res; i++) {
    if (i > 0) len += json_printf(out, ", ");

    char bssid[18];
    sprintf(bssid, "%02x:%02x:%02x:%02x:%02x:%02x", res[i].bssid[0],
            res[i].bssid[1], res[i].bssid[2], res[i].bssid[3],
            res[i].bssid[4], res[i].bssid[5]);

    len +=
        json_printf(out, "{ssid: %Q, bssid: %Q, auth: %d, channel: %d,"
                    " rssi: %d}", res[i].ssid, bssid, res[i].auth_mode,
                    res[i].channel, res[i].rssi);
  }

  return len;
}

static void wifi_scan_cb(int n, struct mgos_wifi_scan_result *res, void *arg) {
  struct mg_rpc_request_info *ri = (struct mg_rpc_request_info *) arg;

  if (n < 0) {
    mg_rpc_send_errorf(ri, n, "wifi scan failed");
    return;
  }
  mg_rpc_send_responsef(ri, "[%M]", wifi_scan_result_printer, n,
                        res);
}

static void mgos_rpc_wifi_scan_handler(struct mg_rpc_request_info *ri,
                                       void *cb_arg,
                                       struct mg_rpc_frame_info *fi,
                                       struct mg_str args) {
  mgos_wifi_scan(wifi_scan_cb, ri);

  (void) args;
  (void) cb_arg;
  (void) fi;
}

bool mgos_rpc_service_wifi_init(void) {
  struct mg_rpc *c = mgos_rpc_get_global();
  mg_rpc_add_handler(c, "Wifi.Scan", "", mgos_rpc_wifi_scan_handler, NULL);

  return true;
}
