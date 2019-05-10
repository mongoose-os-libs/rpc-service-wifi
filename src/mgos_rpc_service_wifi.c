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
    const struct mgos_wifi_scan_result *r = &res[i];
    if (i > 0) len += json_printf(out, ", ");

    len += json_printf(out,
                       "{ssid: %Q, bssid: \"%02x:%02x:%02x:%02x:%02x:%02x\", "
                       "auth: %d, channel: %d,"
                       " rssi: %d}",
                       r->ssid, r->bssid[0], r->bssid[1], r->bssid[2],
                       r->bssid[3], r->bssid[4], r->bssid[5], r->auth_mode,
                       r->channel, r->rssi);
  }

  return len;
}

static void wifi_scan_cb(int n, struct mgos_wifi_scan_result *res, void *arg) {
  struct mg_rpc_request_info *ri = (struct mg_rpc_request_info *) arg;

  if (n < 0) {
    mg_rpc_send_errorf(ri, n, "wifi scan failed");
    return;
  }
  mg_rpc_send_responsef(ri, "[%M]", wifi_scan_result_printer, n, res);
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

static void mgos_rpc_wifi_setup_sta_handler(struct mg_rpc_request_info *ri,
                                            void *cb_arg,
                                            struct mg_rpc_frame_info *fi,
                                            struct mg_str args) {
  bool enable = false;
  struct mgos_config_wifi_sta cfg = {0};
  json_scanf(args.p, args.len, ri->args_fmt, &enable, &cfg.ssid, &cfg.pass);
  cfg.enable = enable;

  if (mgos_wifi_setup_sta(&cfg)) {
    mg_rpc_send_responsef(ri, NULL);
  } else {
    mg_rpc_send_errorf(ri, -1, "%s failed", "mgos_wifi_setup_sta");
  }

  free(cfg.ssid);
  free(cfg.pass);
  (void) fi;
  (void) cb_arg;
}

static void mgos_rpc_wifi_setup_ap_handler(struct mg_rpc_request_info *ri,
                                           void *cb_arg,
                                           struct mg_rpc_frame_info *fi,
                                           struct mg_str args) {
  bool enable = false;
  struct mgos_config_wifi_ap cfg = {
      .ip = (char *) mgos_sys_config_get_wifi_ap_ip(),
      .netmask = (char *) mgos_sys_config_get_wifi_ap_netmask(),
      .dhcp_start = (char *) mgos_sys_config_get_wifi_ap_dhcp_start(),
      .dhcp_end = (char *) mgos_sys_config_get_wifi_ap_dhcp_end(),
      .channel = mgos_sys_config_get_wifi_ap_channel(),
      .max_connections = mgos_sys_config_get_wifi_ap_max_connections(),
  };
  json_scanf(args.p, args.len, ri->args_fmt, &enable, &cfg.ssid, &cfg.pass,
             &cfg.channel);
  cfg.enable = enable;

  if (mgos_wifi_setup_ap(&cfg)) {
    mg_rpc_send_responsef(ri, NULL);
  } else {
    mg_rpc_send_errorf(ri, -1, "%s failed", "mgos_wifi_setup_ap");
  }

  free(cfg.ssid);
  free(cfg.pass);
  (void) fi;
  (void) cb_arg;
}

bool mgos_rpc_service_wifi_init(void) {
  struct mg_rpc *c = mgos_rpc_get_global();
  mg_rpc_add_handler(c, "Wifi.Scan", "", mgos_rpc_wifi_scan_handler, NULL);
  mg_rpc_add_handler(c, "Wifi.SetupSTA", "{enable: %B, ssid: %Q, pass: %Q}",
                     mgos_rpc_wifi_setup_sta_handler, NULL);
  mg_rpc_add_handler(c, "Wifi.SetupAP",
                     "{enable: %B, ssid: %Q, pass: %Q, channel: %d}",
                     mgos_rpc_wifi_setup_ap_handler, NULL);

  return true;
}
