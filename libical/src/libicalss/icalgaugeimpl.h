/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalgaugeimpl.h
 CREATOR: eric 09 Aug 2000


 $Id$
 $Locker:  $

 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The Original Code is eric. The Initial Developer of the Original
 Code is Eric Busboom


======================================================================*/

#include "ical.h"

struct icalgauge_impl
{
	icalcomponent* select;
	icalcomponent* from;
	icalcomponent* where;

};


