/*
 * The Minimal snprintf() implementation
 * Copyright (c) 2013 Michal Ludvig <michal@logix.cz>
 */


#ifndef __MINI_PRINTF__
#define __MINI_PRINTF__

#include <stdarg.h>
#include "ch.h"

int mini_format(
    void *(*cons) (void *, const char *, size_t), void *arg,
    const char *fmt, va_list va);

int snprintf( char *buf, size_t n, const char *fmt, ... );
void nchprintf(BaseSequentialStream *chp, const char *fmt, ...);

#endif
