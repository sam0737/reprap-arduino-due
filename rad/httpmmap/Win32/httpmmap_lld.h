#ifndef _HTTPMMAP_LLD_H_
#define _HTTPMMAP_LLD_H_

#include <stdint.h>

typedef struct {
  uint16_t    port;
} HttpMmapConfig;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void httpmmap_lld_init(void);
  void httpmmap_lld_start(HttpMmapDriver *hmd, HttpMmapConfig *config);
#ifdef __cplusplus
}
#endif

#endif /* _HTTPMMAP_LLD_H_ */
