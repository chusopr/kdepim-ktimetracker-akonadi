/* wait-global.c 
   Copyright (C) 2000 Werner Koch (dd9jn)
   Copyright (C) 2001, 2002, 2003 g10 Code GmbH
 
   This file is part of GPGME.
 
   GPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   GPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with GPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "gpgme.h"
#include "sema.h"
#include "util.h"
#include "context.h"
#include "wait.h"
#include "io.h"

/* The global event loop is used for all asynchronous operations
   (except key listing) for which no user I/O callbacks are specified.

   A context sets up its initial I/O callbacks and then sends the
   GPGME_EVENT_START event.  After that, it is added to the global
   list of active contexts.

   The gpgme_wait function contains a select() loop over all file
   descriptors in all active contexts.  If an error occurs, it closes
   all fds in that context and moves the context to the global done
   list.  Likewise, if a context has removed all I/O callbacks, it is
   moved to the global done list.

   All contexts in the global done list are eligible for being
   returned by gpgme_wait if requested by the caller.  */

/* The ctx_list_lock protects the list of active and done contexts.
   Insertion into any of these lists is only allowed when the lock is
   held.  This allows a muli-threaded program to loop over gpgme_wait
   and in parallel start asynchronous gpgme operations.

   However, the fd tables in the contexts are not protected by this
   lock.  They are only allowed to change either before the context is
   added to the active list (ie, before the start event is signalled)
   or in a callback handler.  */
DEFINE_STATIC_LOCK (ctx_list_lock);

/* A ctx_list_item is an item in the global list of active or done
   contexts.  */
struct ctx_list_item
{
  /* Every ctx_list_item is an element in a doubly linked list.  The
     list pointers are protected by the ctx_list_lock.  */
  struct ctx_list_item *next;
  struct ctx_list_item *prev;

  gpgme_ctx_t ctx;
  /* The status is set when the ctx is moved to the done list.  */
  gpgme_error_t status;
};

/* The active list contains all contexts that are in the global event
   loop, have active I/O callbacks, and have already seen the start
   event.  */
static struct ctx_list_item *ctx_active_list;

/* The done list contains all contexts that have previously been
   active but now are not active any longer, either because they
   finished successfully or an I/O callback returned an error.  The
   status field in the list item contains the error value (or 0 if
   successful).  */
static struct ctx_list_item *ctx_done_list;


/* Enter the context CTX into the active list.  */
static gpgme_error_t
ctx_active (gpgme_ctx_t ctx)
{
  struct ctx_list_item *li = malloc (sizeof (struct ctx_list_item));
  if (!li)
    return gpg_error_from_errno (errno);
  li->ctx = ctx;

  LOCK (ctx_list_lock);
  /* Add LI to active list.  */
  li->next = ctx_active_list;
  li->prev = NULL;
  if (ctx_active_list)
    ctx_active_list->prev = li;
  ctx_active_list = li;
  UNLOCK (ctx_list_lock);
  return 0;
}


/* Enter the context CTX into the done list with status STATUS.  */
static void
ctx_done (gpgme_ctx_t ctx, gpgme_error_t status)
{
  struct ctx_list_item *li;

  LOCK (ctx_list_lock);
  li = ctx_active_list;
  while (li && li->ctx != ctx)
    li = li->next;
  assert (li);

  /* Remove LI from active list.  */
  if (li->next)
    li->next->prev = li->prev;
  if (li->prev)
    li->prev->next = li->next;
  else
    ctx_active_list = li->next;

  li->status = status;

  /* Add LI to done list.  */
  li->next = ctx_done_list;
  li->prev = NULL;
  if (ctx_done_list)
    ctx_done_list->prev = li;
  ctx_done_list = li;
  UNLOCK (ctx_list_lock);
}


/* Find finished context CTX (or any context if CTX is NULL) and
   return its status in STATUS after removing it from the done list.
   If a matching context could be found, return it.  Return NULL if no
   context could be found.  */
static gpgme_ctx_t
ctx_wait (gpgme_ctx_t ctx, gpgme_error_t *status)
{
  struct ctx_list_item *li;

  LOCK (ctx_list_lock);
  li = ctx_done_list;
  if (ctx)
    {
      /* A specific context is requested.  */
      while (li && li->ctx != ctx)
	li = li->next;
    }
  if (li)
    {
      ctx = li->ctx;
      if (status)
	*status = li->status;

      /* Remove LI from done list.  */
      if (li->next)
	li->next->prev = li->prev;
      if (li->prev)
	li->prev->next = li->next;
      else
	ctx_done_list = li->next;
      free (li);
    }
  else
    ctx = NULL;
  UNLOCK (ctx_list_lock);
  return ctx;
}


/* Internal I/O callback functions.  */

/* The add_io_cb and remove_io_cb handlers are shared with the private
   event loops.  */

void
_gpgme_wait_global_event_cb (void *data, gpgme_event_io_t type,
			     void *type_data)
{
  gpgme_ctx_t ctx = (gpgme_ctx_t) data;

  assert (ctx);

  switch (type)
    {
    case GPGME_EVENT_START:
      {
	gpgme_error_t err = ctx_active (ctx);

	if (err)
	  {
	    /* An error occured.  Close all fds in this context, and
	       send the error in a done event.  */
	    unsigned int idx;
	    
	    for (idx = 0; idx <= ctx->fdt.size; idx++)
	      if (ctx->fdt.fds[idx].fd != -1)
		_gpgme_io_close (ctx->fdt.fds[idx].fd);
	    _gpgme_engine_io_event (ctx->engine, GPGME_EVENT_DONE, &err);
	  }
      }
      break;

    case GPGME_EVENT_DONE:
      {
	gpgme_error_t *errp = (gpgme_error_t *) type_data;
	assert (errp);
	ctx_done (ctx, *errp);
      }
      break;

    case GPGME_EVENT_NEXT_KEY:
      assert (!"Unexpected event GPGME_EVENT_NEXT_KEY");
      break;

    case GPGME_EVENT_NEXT_TRUSTITEM:
      assert (!"Unexpected event GPGME_EVENT_NEXT_TRUSTITEM");
      break;

    default:
      assert (!"Unexpected event");
      break;
    }
}



/* Perform asynchronous operations in the global event loop (ie, any
   asynchronous operation except key listing and trustitem listing
   operations).  If CTX is not a null pointer, the function will
   return if the asynchronous operation in the context CTX finished.
   Otherwise the function will return if any asynchronous operation
   finished.  If HANG is zero, the function will not block for a long
   time.  Otherwise the function does not return until an operation
   matching CTX finished.

   If a matching context finished, it is returned, and *STATUS is set
   to the error value of the operation in that context.  Otherwise, if
   the timeout expires, NULL is returned and *STATUS is 0.  If an
   error occurs, NULL is returned and *STATUS is set to the error
   value.  */
gpgme_ctx_t
gpgme_wait (gpgme_ctx_t ctx, gpgme_error_t *status, int hang)
{
  do
    {
      unsigned int i = 0;
      struct ctx_list_item *li;
      struct fd_table fdt;
      int nr;

      /* Collect the active file descriptors.  */
      LOCK (ctx_list_lock);
      for (li = ctx_active_list; li; li = li->next)
	i += li->ctx->fdt.size;
      fdt.fds = malloc (i * sizeof (struct io_select_fd_s));
      if (!fdt.fds)
	{
	  int saved_errno = errno;
	  UNLOCK (ctx_list_lock);
	  if (status)
	    *status = gpg_error_from_errno (saved_errno);
	  return NULL;
	}
      fdt.size = i;
      i = 0;
      for (li = ctx_active_list; li; li = li->next)
	{
	  memcpy (&fdt.fds[i], li->ctx->fdt.fds,
		  li->ctx->fdt.size * sizeof (struct io_select_fd_s));
	  i += li->ctx->fdt.size;
	}
      UNLOCK (ctx_list_lock);

      nr = _gpgme_io_select (fdt.fds, fdt.size, 0);
      if (nr < 0)
	{
	  int saved_errno = errno;
	  free (fdt.fds);
	  if (status)
	    *status = gpg_error_from_errno (saved_errno);
	  return NULL;
	}

      for (i = 0; i < fdt.size && nr; i++)
	{
	  if (fdt.fds[i].fd != -1 && fdt.fds[i].signaled)
	    {
	      gpgme_ctx_t ictx;
	      gpgme_error_t err;
	      struct wait_item_s *item;
	      
	      assert (nr);
	      nr--;
	      
	      item = (struct wait_item_s *) fdt.fds[i].opaque;
	      assert (item);
	      ictx = item->ctx;
	      assert (ictx);

	      err = item->handler (item->handler_value, fdt.fds[i].fd);
	      if (err)
		{
		  /* An error occured.  Close all fds in this context,
		     and signal it.  */
		  unsigned int idx;
	    
		  for (idx = 0; idx < ictx->fdt.size; idx++)
		    if (ictx->fdt.fds[idx].fd != -1)
		      _gpgme_io_close (ictx->fdt.fds[idx].fd);
		  _gpgme_engine_io_event (ictx->engine, GPGME_EVENT_DONE,
					  &err);
		}
	    }
	}
      free (fdt.fds);

      /* Now some contexts might have finished successfully.  */
      LOCK (ctx_list_lock);
      for (li = ctx_active_list; li; li = li->next)
	{
	  for (i = 0; i < ctx->fdt.size; i++)
	    if (ctx->fdt.fds[i].fd != -1)
	      break;
	  if (i == ctx->fdt.size)
	    {
	      gpgme_error_t err = 0;
	      _gpgme_engine_io_event (ctx->engine, GPGME_EVENT_DONE, &err);
	    }
	}
      UNLOCK (ctx_list_lock);

      {
	gpgme_ctx_t dctx = ctx_wait (ctx, status);

	if (dctx)
	  {
	    ctx = dctx;
	    hang = 0;
	  }
      }
    }
  while (hang);

  return ctx;
}
