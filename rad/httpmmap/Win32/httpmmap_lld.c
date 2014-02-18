#include "httpmmap.h"

#include <microhttpd.h>
#include <stdio.h>

void httpmmap_lld_init(void)
{

}

static char *string_to_base64 (const char *message, size_t length)
{
  const char *lookup =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned long l;
  int i;
  char *tmp;

  tmp = malloc (length * 2);
  if (NULL == tmp)
    return tmp;

  tmp[0] = 0;

  for (i = 0; i < length; i += 3)
    {
      l = (((unsigned long) message[i]) << 16)
        | (((i + 1) < length) ? (((unsigned long) message[i + 1]) << 8) : 0)
        | (((i + 2) < length) ? ((unsigned long) message[i + 2]) : 0);


      strncat (tmp, &lookup[(l >> 18) & 0x3F], 1);
      strncat (tmp, &lookup[(l >> 12) & 0x3F], 1);

      if (i + 1 < length)
        strncat (tmp, &lookup[(l >> 6) & 0x3F], 1);
      if (i + 2 < length)
        strncat (tmp, &lookup[l & 0x3F], 1);
    }

  if (length % 3)
    strncat (tmp, "===", 3 - length % 3);

  return tmp;
}

int answer_to_connection (void *cls, struct MHD_Connection *connection,
    const char *url,
    const char *method, const char *version,
    const char *upload_data,
    size_t *upload_data_size, void **con_cls)
{
  HttpMmapDriver* hmd = (HttpMmapDriver*) cls;

  const char *page_name = url;
  if (page_name[0] == '/') page_name++;

  HttpMmapObject* obj = hmd->root;
  while (obj)
  {
    if (strcmp(obj->name, page_name) == 0)
      break;
    obj = obj->next;
  }

  struct MHD_Response *response;
  if (obj == NULL)
  {
    const char *page = "<html><body>Object not found</body></html>";
    response = MHD_create_response_from_buffer(strlen (page),
        (void*) page, MHD_RESPMEM_PERSISTENT);

  } else {
    chSysLock();
    char* result = string_to_base64(obj->buffer, obj->size);
    chSysUnlock();
    response = MHD_create_response_from_buffer(strlen(result),
        (void*) result, MHD_RESPMEM_MUST_FREE);
  }
  MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
  int ret = MHD_queue_response(connection, obj == NULL ? MHD_HTTP_NOT_FOUND : MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

static msg_t threadMhd(void *arg)
{
  HttpMmapDriver* hmd = (HttpMmapDriver*)arg;
  chRegSetThreadName("httpmmap");

  struct MHD_Daemon *daemon;
  daemon = MHD_start_daemon(0, hmd->config->port, NULL, NULL,
      &answer_to_connection, hmd, MHD_OPTION_END);
  fd_set rs;
  fd_set ws;
  fd_set es;
  int max;
  struct timeval tv;
  while (1)
  {
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    max = 0;
    FD_ZERO(&rs);
    FD_ZERO(&ws);
    FD_ZERO(&es);

    while (1) {
      MHD_get_fdset(daemon, &rs, &ws, &es, &max);
      chThdSleepMilliseconds(5);
      if (select(max + 1, &rs, &ws, &es, &tv))
        break;
    }
    MHD_run(daemon);
  }
}

void httpmmap_lld_start(HttpMmapDriver *hmd, HttpMmapConfig *config)
{
  hmd->config = config;
  chThdCreateStatic(hmd->working_area, sizeof(hmd->working_area), LOWPRIO + 5, threadMhd, hmd);
  printf("HttpMmap listening at %d\n", config->port);
}
