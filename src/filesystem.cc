#include <esp_log.h>
#include <esp_littlefs.h>
#include "filesystem.h"

static const char *TAG = __FILE__;


void init_littlefs()
{
	esp_vfs_littlefs_conf_t conf = {
		.base_path = LITTLEFS_MOUNT_POINT,
		.partition_label = "littlefs",
		.format_if_mount_failed = true,
	};

	esp_err_t ret = esp_vfs_littlefs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG, "Failed to mount or format littlefs partition");
		} else if (ret == ESP_ERR_NOT_FOUND)
		{
			ESP_LOGE(TAG, "Failed to find littlefs partition");
		} else
		{
			ESP_LOGE(TAG, "littlefs initialization error (%s)", esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_littlefs_info(conf.partition_label, &total, &used);
	if (ret == ESP_OK)
	{
		ESP_LOGI(TAG, "littlefs mounted: total: %d, used: %d", total, used);
	}
	else
	{
		ESP_LOGE(TAG, "littlefs info error (%s)", esp_err_to_name(ret));
	}
}


char *read_file_to_buffer(const char *path)
{
	FILE *file = fopen(path, "r");
	if (file == NULL)
	{
		ESP_LOGE(TAG, "Couldn't open file: %s", path);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	char *buf = (char *)malloc(size + 1);
	if (buf == NULL)
	{
		ESP_LOGE(TAG, "Couldn't allocate memory for file: %s", path);
		fclose(file);
		return NULL;
	}

	size_t read_size = fread(buf, 1, size, file);
	if (read_size != size)
	{
		ESP_LOGE(TAG, "Error while reading file: %s", path);
		free(buf);
		fclose(file);
		return NULL;
	}

	buf[size] = '\0';
	fclose(file);

	return buf;
}