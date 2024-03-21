#include "esp_http_client.h"

#define MAX_HTTP_OUTPUT_BUFFER 8192

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
