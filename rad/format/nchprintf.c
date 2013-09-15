/* ****************************************************************************
 * Format - lightweight string formatting library.
 * Copyright (C) 2011, Neil Johnson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ************************************************************************* */

/*****************************************************************************/
/* System Includes                                                           */
/*****************************************************************************/

#include <stdarg.h>
#include <stddef.h>

/*****************************************************************************/
/* Project Includes                                                          */
/*****************************************************************************/

#include "mini_printf.h"

/*****************************************************************************/
/* Private function prototypes.  Declare as static.                          */
/*****************************************************************************/

/*****************************************************************************/
/**
    Copy characters from buffer into memory.

    This function implements a bounded buffer write.  It only copies up to the
    number of characters as specified in the call.  After that characters are
    silently dropped.

    Do not use memcpy as it is not available in a freestanding implementation.

    @param memptr  Pointer to output buffer.
    @param buf     Pointer to input buffer.
    @param n       Number of characters from buffer to copy to output buffer.

    @return Address of next empty cell in output buffer.
**/
static void * bufnwrite( void * p, const char * buf, size_t n )
{
    chSequentialStreamWrite((BaseSequentialStream*)p, (uint8_t*) buf, n);
    return p;
}

/*****************************************************************************/
/* Public functions.  Declared as per header file.                           */
/*****************************************************************************/

/*****************************************************************************/
/**
    Produce output according to a format string, with optional arguments.

    @param buf      Output buffer.
    @param fmt      Format specifier.
    @param ap       Argument pointer.

    @return Number of characters written into the output buffer, or -1.
**/
void vnchprintf(BaseSequentialStream *chp, const char *fmt, va_list ap)
{
    mini_format(bufnwrite, (void *)chp, fmt, ap);
}

/*****************************************************************************/
/**
    Produce output according to a format string, with optional arguments.

    @param buf      Output buffer.
    @param fmt      Format specifier.

    @return Number of characters written into the output buffer, or -1.
**/
void nchprintf(BaseSequentialStream *chp, const char *fmt, ...)
{
    va_list arg;
    va_start ( arg, fmt );
    vnchprintf( chp, fmt, arg );
    va_end ( arg );
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
