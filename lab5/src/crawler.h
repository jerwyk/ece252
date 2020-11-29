#pragma once

#include <stddef.h>
#include "util.h"

size_t crawler_cb(char *p_recv, size_t size, size_t nmemb, void *p_userdata);
int process_data(CURL *curl_handle, RECV_BUF *p_recv_buf);