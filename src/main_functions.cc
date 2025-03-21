#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <esp_camera.h>

#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

#include "camera_pins.h"
#include "filesystem.h"
#include "wifi_server.h"
#include "model.h"
#include "main_functions.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = __FILE__;

static camera_config_t camera_config =
{
	.pin_pwdn = PWDN_GPIO_NUM,
	.pin_reset = RESET_GPIO_NUM,
	.pin_xclk = XCLK_GPIO_NUM,
	.pin_sccb_sda = SIOD_GPIO_NUM,
	.pin_sccb_scl = SIOC_GPIO_NUM,

	.pin_d7 = Y9_GPIO_NUM,
	.pin_d6 = Y8_GPIO_NUM,
	.pin_d5 = Y7_GPIO_NUM,
	.pin_d4 = Y6_GPIO_NUM,
	.pin_d3 = Y5_GPIO_NUM,
	.pin_d2 = Y4_GPIO_NUM,
	.pin_d1 = Y3_GPIO_NUM,
	.pin_d0 = Y2_GPIO_NUM,

	.pin_vsync = VSYNC_GPIO_NUM,
	.pin_href = HREF_GPIO_NUM,
	.pin_pclk = PCLK_GPIO_NUM,

	.xclk_freq_hz = 20000000,
	.ledc_timer = LEDC_TIMER_0,
	.ledc_channel = LEDC_CHANNEL_0,

	.pixel_format = PIXFORMAT_GRAYSCALE,
	.frame_size = FRAMESIZE_128X128,

	.fb_count = 2,
	.grab_mode = CAMERA_GRAB_WHEN_EMPTY
};

static httpd_handle_t stream_httpd = NULL;

const tflite::Model* model = nullptr;


void start_camera_server()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  char *index_html = read_file_to_buffer("/littlefs/index.html");

  httpd_uri_t index_uri = {
		.uri = "/",
		.method = HTTP_GET,
		.handler = index_handler,
		.user_ctx = index_html};

  httpd_uri_t stream_uri = {
		.uri = "/stream",
		.method = HTTP_GET,
		.handler = stream_handler,
		.user_ctx = NULL};

	ESP_LOGI(TAG, "Starting web server on port: '%d'\n", config.server_port);

	if (httpd_start(&stream_httpd, &config) == ESP_OK)
	{
		httpd_register_uri_handler(stream_httpd, &index_uri);
		httpd_register_uri_handler(stream_httpd, &stream_uri);
	}
}


void setup()
{
  esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ESP_ERROR_CHECK(nvs_flash_init());
	}

	init_littlefs();
	init_wifi();

	ESP_ERROR_CHECK(esp_camera_init(&camera_config));

	start_camera_server();

	model = tflite::GetModel(face_detector_data);
}


void loop() {
	vTaskDelay(1000);
};