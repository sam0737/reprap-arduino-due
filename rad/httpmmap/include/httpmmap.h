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

struct HttpMmapObject {
  char*           name;
  size_t          size;
  char*           buffer;
  char            modified;
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
