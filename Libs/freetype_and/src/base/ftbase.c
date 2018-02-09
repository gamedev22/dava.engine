/***************************************************************************/
/*                                                                         */
/*  ftbase.c                                                               */
/*                                                                         */
/*    Single object library component (body only).                         */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009 by       */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <freetype/ft2build.h>

#define  FT_MAKE_OPTION_SINGLE_OBJECT

#include "ftpic.c"
#include "basepic.c"
#include "ftadvanc.h"
#include "ftcalc.h"
#include "ftdbgmem.c"
#include "ftgloadr.h"
#include "ftobjs.h"
#include "ftoutln.h"
#include "ftrfork.h"
#include "ftsnames.h"
#include "ftstream.h"
#include "fttrigon.h"
#include "ftutil.h"

#if defined( FT_MACINTOSH ) && !defined ( DARWIN_NO_CARBON )
#include "ftmac.c"
#endif

/* END */
