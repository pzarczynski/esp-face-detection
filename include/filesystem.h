#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#define LITTLEFS_MOUNT_POINT "/littlefs"

void init_littlefs(void);
char* read_file_to_buffer(const char *);

#endif