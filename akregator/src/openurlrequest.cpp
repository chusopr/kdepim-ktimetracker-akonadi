/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "openurlrequest.h"


namespace Akregator {

OpenURLRequest::OpenURLRequest() : m_frameId(-1), m_options(None), m_part(0L)
{
}

int OpenURLRequest::frameId() const
{
    return m_frameId;
}

void OpenURLRequest::setFrameId(int frameId)
{
    m_frameId = frameId;
}

const KUrl& OpenURLRequest::url() const
{
    return m_url;
}

void OpenURLRequest::setUrl(const KUrl& url)
{
    m_url = url;
}
    
const KParts::URLArgs& OpenURLRequest::args() const
{
    return m_args;
}

void OpenURLRequest::setArgs(const KParts::URLArgs& args)
{
    //m_hasArgs = true;
    m_args = args;
}

OpenURLRequest::Options OpenURLRequest::options() const
{
    return m_options;
}

void OpenURLRequest::setOptions(OpenURLRequest::Options options)
{
    m_options = options;
}

void OpenURLRequest::setPart(KParts::ReadOnlyPart* part)
{
    m_part = part;
}

KParts::ReadOnlyPart* OpenURLRequest::part() const
{
    return m_part;
}

QString OpenURLRequest::debugInfo() const
{
    return  "url=" + m_url.url() 
            + " serviceType=" + m_args.serviceType  
            + " newTab=" + m_args.newTab() 
            + " forcesNewWindow=" + m_args.forcesNewWindow()
            + " options="+ m_options; 
}

} // namespace Akregator
