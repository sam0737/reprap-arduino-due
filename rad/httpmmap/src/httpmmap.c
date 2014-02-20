#include "httpmmap.h"
#include <stdio.h>

void httpmmapInit(void)
{
  httpmmap_lld_init();
}

void httpmmapObjectInit(HttpMmapDriver *hmd)
{
  hmd->root = 0;
}

void httpmmapStart(HttpMmapDriver *hmd, HttpMmapConfig *config)
{
  httpmmap_lld_start(hmd, config);
}

void httpmmapAdd(HttpMmapDriver *hmd, HttpMmapObject *hmo)
{
  HttpMmapObject* search = hmd->root;
  while (search) {
    if (search == hmo)
      return;
    search = search->next;
  }

  hmo->next = hmd->root;
  hmd->root = hmo;
}
