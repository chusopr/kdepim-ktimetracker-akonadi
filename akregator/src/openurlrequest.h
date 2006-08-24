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

#ifndef AKREGATOR_OPENURLREQUEST_H
#define AKREGATOR_OPENURLREQUEST_H

#include <kparts/browserextension.h>

#include <kurl.h>

namespace KParts
{
    class ReadOnlyPart;
}

namespace Akregator
{

class Frame;
    
class OpenURLRequest
{
    public:
        
        /**
         * Akregator-specific options specifying how a link should be handled.
         * TODO: check what can be done by overriding KURLArgs flags.
         */
        enum Options
        {
            None=0, /**< no explicit options, use default */
            NewTab, /**< open in new tab */
            ExternalBrowser /**< open in external browser */
        };
        
        OpenURLRequest(const KUrl& url=KUrl());

        /**
         * the Id of the frame that sent the request */  
        int frameId() const;
        void setFrameId(int frameId);
        
        const KUrl& url() const;
        void setUrl(const KUrl& url);
         
        const KParts::URLArgs& args() const;
        void setArgs(const KParts::URLArgs& args);

        Options options() const;
        void setOptions(Options options);
        
        bool openInBackground() const;
        void setOpenInBackground(bool background);
        
        /**
         * The part that was created for a "NewTab" request.
         * 
         * It must be set after creating the tab, so that the initiating 
         * part can load the URL into the new part. This works only synchronously
         * and requires args().serviceType to be set.
         * 
         * @see KParts::BrowserExtension::createNewWindow()
         */
        KParts::ReadOnlyPart* part() const;
        void setPart(KParts::ReadOnlyPart* part);
     
        QString debugInfo() const;
        
    private:

        int m_frameId;
        KUrl m_url;
        KParts::URLArgs m_args;
        Options m_options;
        KParts::ReadOnlyPart* m_part;
        bool m_inBackground;
};

} // namespace Akregator

#endif // AKREGATOR_OPENURLREQUEST_H
