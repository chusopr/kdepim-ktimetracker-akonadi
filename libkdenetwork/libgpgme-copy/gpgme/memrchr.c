/* memrchr.c - Replacement for memrchr.
 * Copyright (C) 2002 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

void *
memrchr (const void *block, int c, size_t size)
{
  unsigned char *p;
  unsigned char l;

  /* Deal with signed weird sizes and zero size */
  if (size < 1) return 0;

  l = c;

  /* If size is 1, you want to look at the single byte pointed to
     by *block ; if size is 2, you want to look at *block and *(block+1).
     Hence, add (size-1), not size. */
  p = ((unsigned char *)block) + (size - 1) ;

  /* Must examine *block as well, so instead loop on the size, not
     on a pointer comparison. */
  for ( ; size>0 ; size --, p --)
    if (*p == c)
      return p;
  return 0;
}
