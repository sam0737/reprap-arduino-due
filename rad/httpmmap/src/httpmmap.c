#include "httpmmap.h"

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
  hmo->next = hmd->root;
  hmo->modified = 0;
  hmd->root = hmo;
}
