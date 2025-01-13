#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_camera.h>
#include "wifi_server.h"

static const char* TAG = __FILE__;

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";


void event_handler(void *asrg, esp_event_base_t event_base,
				   int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		ESP_ERROR_CHECK(esp_wifi_connect());
	} 
	
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		ESP_LOGI(TAG, "connect to the AP fail");
	} 
	
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
	}
}


void init_wifi(void)
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&event_handler,
														NULL,
														&instance_any_id));

	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&event_handler,
														NULL,
														&instance_got_ip));

	wifi_config_t wifi_config =
	{
		.sta = {
			.ssid = WIFI_SSID,
			.password = WIFI_PASS,
			.channel = WIFI_CHANNEL,
			.pmf_cfg = {
				.required = false,
			},
		},
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init finished. SSID:%s password:%s channel:%d",
					 WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
}


esp_err_t index_handler(httpd_req_t *req)
{
	char *html = (char *)req->user_ctx;
	esp_err_t res = httpd_resp_send(req, html, strlen(html));
	return res;
}


esp_err_t stream_handler(httpd_req_t *req)
{
	camera_fb_t *fb = NULL;
	esp_err_t res = ESP_OK;
	unsigned char *_jpg_buf = NULL;
	size_t _jpg_buf_len = 0;
	char *part_buf[64];

	res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
	if (res != ESP_OK)
	{
		return res; 
	}

	while (true)
	{
		fb = esp_camera_fb_get();
		if (!fb)
		{
			ESP_LOGE(TAG, "Camera capture failed");
			res = ESP_FAIL;
		} 

		if (fb->format != PIXFORMAT_JPEG) {
			if (!frame2jpg(fb, 40, &_jpg_buf, &_jpg_buf_len))
			{
				ESP_LOGE(TAG, "Converting to JPEG failed");
				res = ESP_FAIL;
			}
			esp_camera_fb_return(fb);
		} else
		{
			_jpg_buf = fb->buf;
			_jpg_buf_len = fb->len;
		}

		if (res == ESP_OK)
		{
			size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf);
			res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
		}

		if (res == ESP_OK)
		{
			res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
		}

		if (res == ESP_OK)
		{
			res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
		}

		if (fb)
		{
			esp_camera_fb_return(fb);
		}

		free(_jpg_buf);
	}
	
	return res;
}