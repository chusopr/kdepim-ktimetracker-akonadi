/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "articleviewer.h"
#include "viewer.h"
#include "feed.h"
#include "akregatorconfig.h"
#include "akregator_run.h"

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kmimetype.h>
#include <khtmlview.h>
#include <krun.h>
#include <kprocess.h>
#include <kshell.h>

#include <qdatetime.h>
#include <qvaluelist.h>
#include <qscrollview.h>
#include <qevent.h>


using namespace Akregator;

// from kmail::headerstyle.cpp
static inline QString directionOf(const QString &str)
{
    return str.isRightToLeft() ? "rtl" : "ltr" ;
}

int pointsToPixel(const QPaintDeviceMetrics &metrics, int pointSize)
{
    return ( pointSize * metrics.logicalDpiY() + 36 ) / 72 ;
}

ArticleViewer::ArticleViewer(QWidget *parent, const char *name)
   : Viewer(parent, name), m_htmlHead(), m_metrics(widget()), m_currentText() 
{
    generateCSS();
}

void ArticleViewer::openDefault()
{
    QString text= QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
                        "<html><head><title></title></head><body>\n");

    text += QString("<h1>%1</h1>\n"
                    "<p>%2</p><ul>\n"
                    "<li>%3</li><ul>\n"
                    "<li><b>%4</b>%5</li>\n"
                    "<li><b>%6</b>%7</li>\n"
                    "<li><b>%8</b>%9</li>")
                    .arg(i18n("Welcome to aKregator"))
                    .arg(i18n("Use the tree to manage your feeds."))
                    .arg(i18n("Right click a folder, such as \"All Feeds\", and choose:"))
                    .arg(i18n("Add..."))
                    .arg(i18n(" to add a new feed to your feed list."))
                    .arg(i18n("New Folder..."))
                    .arg(i18n(" to add a new folder to your list."))
                    .arg(i18n("Edit"))
                    .arg(i18n(" to edit an existing feed or folder."));
   
    text += QString("<li><b>%1</b>%2</li>\n"
                    "<li><b>%3</b>%4</li></ul></ul>\n"
                    "<p>%5</p></body></html>")
                    .arg(i18n("Delete"))
                    .arg(i18n(" to remove an existing feed or folder."))
                    .arg(i18n("Fetch"))
                    .arg(i18n(" to update a feed or folder."))
                    .arg(i18n("Click \"Fetch All\" to update all feeds."));

    begin();
    write(text);
    end();

}

void ArticleViewer::generateCSS()
{
    const QColorGroup & cg = QApplication::palette().active();
    m_htmlHead=QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
                        "<html><head><title></title></head><body>");
    m_htmlHead += QString (
    "<style type=\"text/css\">\n"
    "body {\n"
    "  font-family: \"%1\" ! important;\n"
// from kmail::headerstyle.cpp
    "  font-size: %2 ! important;\n"
    "  color: %3 ! important;\n"
    "  background: %4 ! important;\n"
    "}\n\n"
    "a {\n"
    "  color: %5 ! important;\n"
    "  text-decoration: none ! important;\n"
    "}\n\n"
    "#headerbox {\n"
    "  background: %6 ! important;\n"
    "  color: %7 ! important;\n"
    "  border:1px solid #000;\n"
    "  margin-bottom: 10pt;\n"
    "  width: 100%;\n"
    "}\n\n"
    "#headertitle a:link { color: %9  ! important; }\n"
    "#headertitle a:visited { color: %9 ! important; }\n"
    "#headertitle a:hover{ color: %9 ! important; }\n"
    "#headertitle a:active { color: %9 ! important; }\n"
    "#headertitle {\n"
    "  background: %8 ! important;\n"
    "  padding:2px;\n"
    "  color: %9 ! important;\n"
    "  font-weight: bold;\n"
    "}\n\n"
    "#header {\n"
    "  font-weight: bold;\n"
    "  padding:2px;\n"
    "  margin-right: 5px;\n"
    "}\n\n"
    "#headertext {\n"
    "}\n\n"
    "#headimage {\n"
    "  float: right;\n"
    "}\n\n"
    "#content {\n"
    "  clear: none;\n"
    "  overflow: auto;\n"
    "}\n\n")
    .arg(KGlobalSettings::generalFont().family())
    .arg(QString::number( pointsToPixel( m_metrics, KGlobalSettings::generalFont().pointSize()))+"px")
    .arg(cg.text().name())
    .arg(cg.base().name())
    .arg(cg.link().name())
    .arg(cg.background().name())
    .arg(cg.text().name())
    .arg(cg.highlight().name())
    .arg(cg.highlightedText().name());

    m_htmlHead += QString (
    "#article {\n"
    "  overflow: hidden;\n"
    "  border:1px solid #000;\n"
    "  background: %1;\n"
    "  padding: 3px;\n"
    "  padding-right: 6px;}\n\n"
    "#titleanchor {\n"
    "  color: %2 !important;}\n\n"
    "</style>\n")
    .arg(cg.background().light(108).name())
    .arg(cg.text().name());

}

void ArticleViewer::reload()
{
    generateCSS();
    begin( KURL( "file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/") ) );
    write(m_htmlHead + m_currentText);
    end();
}

QString ArticleViewer::formatArticle(Feed *f, MyArticle a)
{
    QString text;
    text = QString("<div id=\"headerbox\" dir=\"%1\">\n").arg(QApplication::reverseLayout() ? "rtl" : "ltr");

    if (!a.title().isEmpty())
    {
        text += QString("<div id=\"headertitle\" dir=\"%1\">\n").arg(directionOf(a.title()));
        if (a.link().isValid())
            text += "<a id=\"titleanchor\" href=\""+a.link().url()+"\">";
        text += a.title();
        if (a.link().isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (a.pubDate().isValid())
    {
        text += QString("<span id=\"header\" dir=\"%1\">").arg(directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span id=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(a.pubDate(), false, false)+"</span>\n"; // TODO: might need RTL?
    }
    text += "</div>\n"; // end headerbox

    if (f && !f->image.isNull())
    {
        QString url=f->xmlUrl;
        text += QString("<a href=\""+f->htmlUrl+"\"><img id=\"headimage\" src=\""+url.replace("/", "_").replace(":", "_")+".png\"></a>\n");
    }

    text += "<div id=\"content\">"+a.description();
    if (a.link().isValid() || (a.guidIsPermaLink() && KURL(a.guid()).isValid()))
    {
        if (!a.description().isNull())
            text += "<p>\n";
        text += "<a href=\"";
        // in case link isn't valid, fall back to the guid permaLink.
        if (a.link().isValid())
        {
            text += a.link().url();
        }
        else
        {
            text += a.guid();
        }
        text += "\">" + i18n( "Complete Story" ) + "</a>";
        if (!a.description().isNull())
            text += "<p>\n";
    }
    text += "</div>";
    return text;

}

void ArticleViewer::beginWriting()
{
    view()->setContentsPos(0,0);
    begin( KURL( "file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/") ) );
    write(m_htmlHead);
}

void ArticleViewer::endWriting()
{
    m_currentText = m_currentText + "</body></html>";
    write("</body></html>");
    end();
}

void ArticleViewer::show(Feed *f, bool writeHeaders)
{
    QString art, text;

    if (writeHeaders)
    {
        beginWriting();
    }

    ArticleSequence::iterator it;
    ArticleSequence::iterator en(f->articles.end());
    for ( it = f->articles.begin(); it != en; ++it )
    {
        // we set f to 0 to not show feed image
        art="<p><div id=\"article\">"+formatArticle(0, *it)+"</div><p>";
        text += art;
        write(art);
    }

    if (writeHeaders)
    {
    	endWriting();
    }
    else
    {
        m_currentText = m_currentText+text;
    }
}

void ArticleViewer::show(Feed *f, MyArticle a)
{
    view()->setContentsPos(0,0);
    begin( KURL( "file:"+KGlobal::dirs()->saveLocation("cache", "akregator/Media/") ) );

    QString text=formatArticle(f, a) +"</body></html>";
    m_currentText=text;

    write(m_htmlHead + text);
    end();
}

bool ArticleViewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
    // special case the fast case
    QString type=url.url();
    kdError() << "type==="<<type<<endl;
    if (type.right(5)==".html" ||type.right(4)==".php" || type.right(4)==".htm"
            || type.right(4)==".xml")
    {
        openPage(url, args, type);    
        return true;
    }

    // else check mimetype
    new aKregatorRun(this, (QWidget*)parent(), this, url, args, true);
    return true;
}


void ArticleViewer::openPage(const KURL&url, const KParts::URLArgs& args, const QString &)
{
   kdDebug() << "ArticleViewer: Open url request: " << url << endl;
   if(Viewer::slotOpenURLRequest(url, args)) return;
   emit urlClicked(url);
   return;
}

void ArticleViewer::slotOpenLinkInternal()
{
    if(m_url.isEmpty()) return;
    emit urlClicked(m_url);
}

#include "articleviewer.moc"
// vim: set et ts=4 sts=4 sw=4:
