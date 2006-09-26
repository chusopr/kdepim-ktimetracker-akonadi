/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Timo Hummel <timo.hummel@sap.com>
 *                    Tom Braun <braunt@fh-konstanz.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QtCore/QString>

#include "config.h"

#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>

#include "kcrash.h"

QString kBacktrace()
{
  QString s;

  void* trace[256];
  int n = backtrace(trace, 256);
  if (!n)
    return s;

  char** strings = backtrace_symbols (trace, n);

  s = QLatin1String("[\n");

  for (int i = 0; i < n; ++i)
    s += QString::number(i) +
         QLatin1String(": ") +
         QLatin1String(strings[i]) + QLatin1String("\n");
    s += QLatin1String("]\n");

  if (strings)
    free (strings);

  return s;
}
static KCrash::HandlerType s_emergencyMethod = 0;

void KCrash::setEmergencyMethod( HandlerType method )
{
  s_emergencyMethod = method;
}

static void defaultCrashHandler( int sig )
{
  fprintf( stderr, "%s", kBacktrace().toLatin1().data() );

  if ( s_emergencyMethod )
    s_emergencyMethod( sig );

  _exit(255);
}

void KCrash::init()
{
  HandlerType handler = defaultCrashHandler;

#ifdef Q_OS_UNIX
  if (!handler)
    handler = SIG_DFL;

  sigset_t mask;
  sigemptyset(&mask);

#ifdef SIGSEGV
  signal (SIGSEGV, handler);
  sigaddset(&mask, SIGSEGV);
  qDebug( "set SIGSEV handler" );
#endif
#ifdef SIGFPE
  signal (SIGFPE, handler);
  sigaddset(&mask, SIGFPE);
#endif
#ifdef SIGILL
  signal (SIGILL, handler);
  sigaddset(&mask, SIGILL);
#endif
#ifdef SIGABRT
  signal (SIGABRT, handler);
  sigaddset(&mask, SIGABRT);
#endif

  sigprocmask(SIG_UNBLOCK, &mask, 0);
#endif //Q_OS_UNIX

}

