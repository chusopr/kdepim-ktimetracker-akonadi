/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Qt includes
#include <qstrlist.h>

// KDE includes
#include <klocale.h>
#include <krun.h>

// Local includes
#include "EmpathMessageStructureWidget.h"
#include "EmpathMessageHTMLView.h"
#include "EmpathHeaderViewWidget.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathComposeWindow.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
#include <RMM_Message.h>
#include <RMM_BodyPart.h>
#include <RMM_ContentType.h>
    
EmpathMessageViewWidget::EmpathMessageViewWidget(
        const EmpathURL & url,
        QWidget *parent,
        const char *name)
    :    QWidget(parent, name),
        url_(url),
        viewingSource_(false)
{
    empathDebug("ctor");

    structureWidget_ =
        new EmpathMessageStructureWidget(0, "structureWidget");
    CHECK_PTR(structureWidget_);
    
    QVBoxLayout * layout = new QVBoxLayout(this);
    CHECK_PTR(layout);
    
    layout->setAutoAdd(true);
    
    headerViewWidget_ =
        new EmpathHeaderViewWidget(this, "headerViewWidget");
    CHECK_PTR(headerViewWidget_);
    
    messageWidget_ = new EmpathMessageHTMLWidget(this, "messageWidget");
    CHECK_PTR(messageWidget_);
    
    QObject::connect(
        headerViewWidget_,  SIGNAL(clipClicked()),
        this,               SLOT(s_clipClicked()));
    
    QObject::connect(messageWidget_, SIGNAL(URLSelected(QString, int)),
            SLOT(s_URLSelected(QString, int)));

    QObject::connect(
        empath, SIGNAL(retrieveComplete(bool, const EmpathURL &, QString)),
        this,   SLOT(s_retrieveComplete(bool, const EmpathURL &, QString)));
    
    QObject::connect(
        structureWidget_,    SIGNAL(partChanged(RMM::RBodyPart *)),
        this,                SLOT(s_partChanged(RMM::RBodyPart *)));
   
    layout->activate();
    show();    
}

    void
EmpathMessageViewWidget::s_retrieveComplete(
    bool b, const EmpathURL & url, QString xinfo)
{
    if ((b == false) || (xinfo != "view"))
        return;

    url_ = url;

    empathDebug("Got operation complete signal");
    empathDebug("b is " + QString(b ? "true" : "false")); 

    RMM::RMessage * m(empath->message(url_));
    
    if (m == 0) {
        empathDebug("Couldn't get supposedly retrieved message from \"" +
            url_.asString() + "\"");
        return;
    }
    
    RMM::RBodyPart message(m->decode());
    structureWidget_->setMessage(message);
    
    // Ok I'm going to try and get the viewable body parts now.
    // To start with, I'll see if there's only one part. If so, I'll show it,
    // if possible. Otherwise, I'll have to have a harder look and see what
    // we're supposed to be showing. If we're looking at a
    // multipart/alternative, I'll pick the 'best' of the possibilities.
    
    headerViewWidget_->useEnvelope(message.envelope());
    
    QCString s;
    
    if (message.body().count() == 0)
        s = message.decode().data();
    
    else if (message.body().count() == 1)
        s = message.body().at(0)->decode().data();
    
    else {
        
        // Multipart message
        // 
        // Options
        // 
        // ALTERNATIVE
        // 
        // Choose the 'nicest' version that we are capable of displaying.
        // 
        // I hate HTML mail, but that's just me.
        // 
        // Order of preference:
        // HTML, RichText, Plain
        // 
        // MIXED
        // 
        // With mixed parts, we don't have a clue which part is supposed
        // to be shown. We assume all, really, but we're not going to do
        // that.
        // 
        // What we want to do is to find a part that looks like it's
        // equivalent to what you expect, i.e. some text.
        // 
        // So, look for text/plain.
        // 
        // We won't use the priority order of ALTERNATIVE as we haven't
        // been asked to, and we might end up showing some HTML that just
        // confuses the user, when they should be seeing 'hello here's that
        // web page I told you about'
        
        empathDebug("===================== MULTIPART ====================");
        
        QListIterator<RMM::RBodyPart> it(message.body());
        
        int i = 0;
        for (; it.current(); ++it) {
        
            ++i;
        
            empathDebug(
                " ===================== PART # "
                + QString().setNum(i) +
                " =====================");

            if (it.current()->envelope().has(RMM::HeaderContentType)) {
                
                empathDebug("Ok this part has a Content-Type");                            
                    
                RMM::RContentType t = it.current()->envelope().contentType();
                
                empathDebug("   Type of this part is \"" + t.type() + "\"");
                empathDebug("SubType of this part is \"" + t.subType() + "\"");

                if (!stricmp(t.type(), "text")) {
                    
                    if (!stricmp(t.subType(), "html")) {

                        empathDebug("Using this part as body");

                        RMM::RBodyPart p(*it.current());
                    
                        s = p.decode().data();
                        showText(s, true);
                        return;
    
                    } else if (!stricmp(t.subType(), "plain")) {
                    
                        empathDebug("Using this part as body");

                        RMM::RBodyPart p(*it.current());
                    
                        s = p.decode().data();
                        showText(s);
                        return;
                    }
                    
                } else {

                    empathDebug("Haven't decided what to do with this part yet");
                    RMM::RBodyPart p(*it.current());
                    s = p.decode().data();
                    showText(s);
                }
            }
        }
        
        empathDebug("=================== END MULTIPART =====================");
    }

    showText(s);
}

    void
EmpathMessageViewWidget::s_print()
{
    empathDebug("print() called");
//    messageWidget_->print();
}

EmpathMessageViewWidget::~EmpathMessageViewWidget()
{
    empathDebug("dtor");
    delete structureWidget_;
}

    void
EmpathMessageViewWidget::s_setMessage(const EmpathURL & url)
{
    empathDebug("setMessage() called with \"" + url.asString() + "\"");
    url_ = url;
    empath->retrieve(url_, "view");
}

    void
EmpathMessageViewWidget::s_URLSelected(QString fixedURL, int button)
{
    fixedURL = fixedURL.stripWhiteSpace();

    if (fixedURL.left(7) == "mailto:") {
        
        fixedURL = fixedURL.right(fixedURL.length() - 7);

        if (button == 1) {
            empath->s_compose();
        }
    
    } else {

        // It's an URL we don't handle. Pass to KRun.
        
        new KRun(fixedURL);
    }
}

    void
EmpathMessageViewWidget::s_clipClicked()
{
    if (structureWidget_->isVisible())
        structureWidget_->hide();
    else
        structureWidget_->show();
}

    void
EmpathMessageViewWidget::s_partChanged(RMM::RBodyPart * part)
{
    empathDebug("s_partChanged() called");
    RMM::RBodyPart p(*part);
    QCString s(p.data());
    showText(s, true);
}

    void
EmpathMessageViewWidget::s_switchView()
{
    empathDebug("s_switchView() called");
    if (viewingSource_) {
        
        empathDebug("Doing normal view");
        viewingSource_ = false;
        //go();

    } else {
        
        empathDebug("Doing source view");
        viewingSource_ = true;
    
        RMM::RMessage * m(empath->message(url_));
    
        if (m == 0) {
            empathDebug("Can't load message from \"" + url_.asString() + "\"");
            return;
        }
        
        QCString s(m->asString());
        showText(s, false);
    }
}

    void
EmpathMessageViewWidget::showText(QCString & s, bool markup)
{
    messageWidget_->showText(s, markup);
}

