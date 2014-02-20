#ifndef _HTTPMMAP_H_
#define _HTTPMMAP_H_

#include "ch.h"
#include <stddef.h>

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

typedef struct HttpMmapDriver HttpMmapDriver;
typedef struct HttpMmapObject HttpMmapObject;

#include "httpmmap_lld.h"

struct HttpMmapDriver {
  HttpMmapObject* root;
  HttpMmapConfig* config;
  WORKING_AREA(working_area, 16 * 1024);
};

typedef void (*httpmmap_post_callback_t)(HttpMmapObject* object);

struct HttpMmapObject {
  char*           name;
  size_t          size;
  size_t          upload_size;
  char*           buffer;
  httpmmap_post_callback_t  callback;
  HttpMmapObject* next;
};

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void httpmmapInit(void);
  void httpmmapObjectInit(HttpMmapDriver *hmd);
  void httpmmapStart(HttpMmapDriver *hmd, HttpMmapConfig *config);
  void httpmmapAdd(HttpMmapDriver *hmd, HttpMmapObject *hmo);
#ifdef __cplusplus
}
#endif

#endif /* _HTTPMMAP_H_ */

/** @} */
