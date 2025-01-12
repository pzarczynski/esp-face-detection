#ifndef _WIFI_SERVER_H
#define _WIFI_SERVER_H

#include <esp_http_server.h>

#ifndef WIFI_SSID
#error "define WIFI_SSID"
#endif

#ifndef WIFI_PASS
#error "define WIFI_PASS"
#endif

#define WIFI_CHANNEL   1
#define STA_CONN       4

#define PART_BOUNDARY "123456789000000000000987654321"

void event_handler(void *, esp_event_base_t, int32_t, void*);
void init_wifi(void);
esp_err_t index_handler(httpd_req_t *);
esp_err_t stream_handler(httpd_req_t *);

#endif