/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include <kaction.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <khtmlview.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <krun.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <kparts/browserextension.h>
#include <kparts/browserrun.h>

#include <QClipboard>
#include <QGridLayout>

#include <libkdepim/kfileio.h>

#include "akregatorconfig.h"
#include "openurlrequest.h"

// TODO: remove unneeded includes
#include "aboutdata.h"
#include "article.h"
#include "articleformatter.h"
#include "articleviewer.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "tagnode.h"
#include "utils.h"

namespace Akregator {

// from kmail::headerstyle.cpp
static inline QString directionOf(const QString &str)
{
    return str.isRightToLeft() ? "rtl" : "ltr" ;
}

class ArticleViewer::ShowSummaryVisitor : public TreeNodeVisitor
{
    public:

        ShowSummaryVisitor(ArticleViewer* view) : m_view(view) {}

        virtual bool visitFeed(Feed* node)
        {
            m_view->m_link = QString();
            QString text;
            text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");

            text += QString("<div class=\"headertitle\" dir=\"%1\">").arg(directionOf(Utils::stripTags(node->title())));
            text += node->title();
            if(node->unread() == 0)
                text += i18n(" (no unread articles)");
            else
                text += i18np(" (1 unread article)", " (%n unread articles)", node->unread());
            text += "</div>\n"; // headertitle
            text += "</div>\n"; // /headerbox

            if (!node->image().isNull()) // image
            {
                text += QString("<div class=\"body\">");
                QString file = Utils::fileNameForUrl(node->xmlUrl());
                KUrl u(m_view->m_imageDir);
                u.setFileName(file);
                text += QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(node->htmlUrl()).arg(u.url());
            }
            else text += "<div class=\"body\">";


            if( !node->description().isEmpty() )
            {
                text += QString("<div dir=\"%1\">").arg(Utils::stripTags(directionOf(node->description())));
                text += i18n("<b>Description:</b> %1<br><br>", node->description());
                text += "</div>\n"; // /description
            }

            if ( !node->htmlUrl().isEmpty() )
            {
                text += QString("<div dir=\"%1\">").arg(directionOf(node->htmlUrl()));
                text += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>", node->htmlUrl(), node->htmlUrl());
                text += "</div>\n"; // / link
            }

        //text += i18n("<b>Unread articles:</b> %1").arg(node->unread());
            text += "</div>"; // /body

            m_view->renderContent(text);
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            m_view->m_link = QString();

            QString text;
            text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
            text += QString("<div class=\"headertitle\" dir=\"%1\">%2").arg(directionOf(Utils::stripTags(node->title()))).arg(node->title());
            if(node->unread() == 0)
                text += i18n(" (no unread articles)");
            else
                text += i18np(" (1 unread article)", " (%n unread articles)", node->unread());
            text += QString("</div>\n");
            text += "</div>\n"; // /headerbox

            m_view->renderContent(text);
            return true;
        }

        virtual bool visitTagNode(TagNode* node)
        {
            m_view->m_link = QString();

            QString text;
            text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
            text += QString("<div class=\"headertitle\" dir=\"%1\">%2").arg(directionOf(Utils::stripTags(node->title()))).arg(node->title());
            if(node->unread() == 0)
                text += i18n(" (no unread articles)");
            else
                text += i18np(" (1 unread article)", " (%n unread articles)", node->unread());
            text += QString("</div>\n");
            text += "</div>\n"; // /headerbox

            m_view->renderContent(text);
            return true;
        }

    private:

        ArticleViewer* m_view;
};

ArticleViewer::ArticleViewer(QWidget *parent)
    : QWidget(parent), m_url(0), m_htmlFooter(), m_currentText(), m_node(0),
      m_viewMode(NormalView)
{
    m_normalViewFormatter = 0;
    m_combinedViewFormatter = 0;
    m_part = new ArticleViewerPart(this);
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_part->widget(), 0, 0);

    m_showSummaryVisitor = new ShowSummaryVisitor(this);
    m_part->setZoomFactor(100);
    m_part->setJScriptEnabled(true);
    m_part->setJavaEnabled(true);
    m_part->setMetaRefreshEnabled(true);
    m_part->setPluginsEnabled(true);
    m_part->setDNDEnabled(true);
    m_part->setAutoloadImages(true);
    m_part->setStatusMessagesEnabled(true);

    // change the cursor when loading stuff...
    connect( this, SIGNAL(started(KIO::Job *)),
             this, SLOT(slotStarted(KIO::Job *)));
    connect( this, SIGNAL(completed()),
             this, SLOT(slotCompleted()));

    KParts::BrowserExtension* ext = m_part->browserExtension();
    connect(ext, SIGNAL(popupMenu (KXMLGUIClient*, const QPoint&, const KUrl&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)),
             this, SLOT(slotPopupMenu(KXMLGUIClient*, const QPoint&, const KUrl&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)));

    connect( ext, SIGNAL(openURLRequestDelayed(const KUrl&, const KParts::URLArgs&)), this, SLOT(slotOpenURLRequestDelayed(const KUrl&, const KParts::URLArgs& )) );

    connect(ext, SIGNAL(createNewWindow(const KUrl&, const KParts::URLArgs&)),
            parent, SLOT(slotCreateNewWindow(const KUrl&, const KParts::URLArgs&)));

    connect(ext, SIGNAL(createNewWindow(const KUrl&,
            const KParts::URLArgs&,
            const KParts::WindowArgs&,
            KParts::ReadOnlyPart*&)),
            parent, SLOT(slotCreateNewWindow(const KUrl&,
                         const KParts::URLArgs&,
                         const KParts::WindowArgs&,
                         KParts::ReadOnlyPart*&)));

    KStdAction::print(this, SLOT(slotPrint()), m_part->actionCollection(), "viewer_print");
    KStdAction::copy(this, SLOT(slotCopy()), m_part->actionCollection(), "viewer_copy");

    KAction *action = new KAction(KIcon("viewmag+"),  i18n("&Increase Font Sizes"),
                                  m_part->actionCollection(), "incFontSizes" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotZoomIn()));
    action->setShortcut(KShortcut( "Ctrl+Plus" ));
    action = new KAction(KIcon("viewmag-"),  i18n("&Decrease Font Sizes"), m_part->actionCollection(), "decFontSizes" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotZoomOut()));
    action->setShortcut(KShortcut( "Ctrl+Minus" ));

    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

    action = new KAction(i18n("Copy &Link Address"), m_part->actionCollection(), "copylinkaddress");
    connect(action, SIGNAL(triggered(bool) ), SLOT(slotCopyLinkAddress()));
    action = new KAction(i18n("&Save Link As..."), m_part->actionCollection(), "savelinkas");
    connect(action, SIGNAL(triggered(bool) ), SLOT(slotSaveLinkAs()));

    m_imageDir.setPath(KGlobal::dirs()->saveLocation("cache", "akregator/Media/"));

    setNormalViewFormatter(DefaultNormalViewFormatter(m_imageDir));
    setCombinedViewFormatter(DefaultCombinedViewFormatter(m_imageDir));

    updateCss();

    action = new KAction( i18n("&Scroll Up"), m_part->actionCollection(), "articleviewer_scroll_up" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotScrollUp()));
    action->setShortcut(KShortcut( "Up" ));
    action = new KAction( i18n("&Scroll Down"), m_part->actionCollection(), "articleviewer_scroll_down" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotScrollDown()));
    action->setShortcut(KShortcut( "Down" ));

    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

    connect(kapp, SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    connect(kapp, SIGNAL(kdisplayFontChanged()), this, SLOT(slotPaletteOrFontChanged()) );


    m_htmlFooter = "</body></html>";
}

ArticleViewer::~ArticleViewer()
{
    delete m_showSummaryVisitor;
}

KParts::ReadOnlyPart* ArticleViewer::part() const
{
    return m_part;
}

int ArticleViewer::pointsToPixel(int pointSize) const
{
    return ( pointSize * m_part->view()->logicalDpiY() + 36 ) / 72 ;
}

void ArticleViewer::slotOpenURLRequestDelayed(const KUrl& url, const KParts::URLArgs& args)
{
    OpenURLRequest req(url);
    req.setArgs(args);
    req.setOptions(OpenURLRequest::NewTab);

    if (m_part->button() == Qt::LeftButton)
    {
        switch (Settings::lMBBehaviour())
        {
            case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
                req.setOptions(OpenURLRequest::ExternalBrowser);
                break;
            case Settings::EnumLMBBehaviour::OpenInBackground:
                req.setOpenInBackground(true);
                break;
            default:
                break;
        }
    }
    else if (m_part->button() == Qt::MidButton)
    {
        switch (Settings::mMBBehaviour())
        {
            case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
                req.setOptions(OpenURLRequest::ExternalBrowser);
                break;
            case Settings::EnumMMBBehaviour::OpenInBackground:
                req.setOpenInBackground(true);
                break;
            default:
                break;
        }
    }

    emit signalOpenURLRequest(req);
}

void ArticleViewer::slotCreateNewWindow(const KUrl& url, const KParts::URLArgs& args)
{
    OpenURLRequest req(url);
    req.setArgs(args);
    req.setOptions(OpenURLRequest::NewTab);

    emit signalOpenURLRequest(req);
}

void ArticleViewer::slotCreateNewWindow(const KUrl& url,
                                       const KParts::URLArgs& args,
                                       const KParts::WindowArgs& /*windowArgs*/,
                                       KParts::ReadOnlyPart*& part)
{
    OpenURLRequest req;
    req.setUrl(url);
    req.setArgs(args);
    req.setOptions(OpenURLRequest::NewTab);

    emit signalOpenURLRequest(req);
    part = req.part();
}

void ArticleViewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KUrl& kurl, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags kpf, mode_t)
{
    const bool isLink = (kpf & KParts::BrowserExtension::ShowNavigationItems) == 0;
    const bool isSelection = (kpf & KParts::BrowserExtension::ShowTextSelectionItems) != 0;

    QString url = kurl.url();

    m_url = url;
    KMenu popup;

    if (isLink && !isSelection)
    {
        popup.insertItem(SmallIcon("tab_new"), i18n("Open Link in New &Tab"), this, SLOT(slotOpenLinkInForegroundTab()));
        popup.insertItem(SmallIcon("window_new"), i18n("Open Link in External &Browser"), this, SLOT(slotOpenLinkInBrowser()));
        popup.addSeparator();
        popup.addAction( m_part->action("savelinkas") );
        popup.addAction( m_part->action("copylinkaddress") );
    }
    else
    {
        if (isSelection)
        {
            popup.addAction( m_part->action("viewer_copy") );
            popup.addSeparator();
        }
        popup.addAction( m_part->action("viewer_print") );
       //KAction *ac = action("setEncoding");
       //if (ac)
       //     ac->plug(&popup);
    }
    popup.exec(p);
}

// taken from KDevelop
void ArticleViewer::slotCopy()
{
    QString text = m_part->selectedText();
    text.replace( QChar( 0xa0 ), ' ' );
    QClipboard *cb = QApplication::clipboard();
    disconnect( cb, SIGNAL( selectionChanged() ), this, SLOT( slotClearSelection() ) );
    cb->setText(text);
    connect( cb, SIGNAL( selectionChanged() ), this, SLOT( slotClearSelection() ) );
}

void ArticleViewer::slotCopyLinkAddress()
{
    if(m_url.isEmpty()) return;
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_url.prettyUrl(), QClipboard::Clipboard);
    cb->setText(m_url.prettyUrl(), QClipboard::Selection);
}

void ArticleViewer::slotSelectionChanged()
{
    m_part->action("viewer_copy")->setEnabled(!m_part->selectedText().isEmpty());
}

void ArticleViewer::slotOpenLinkInternal()
{
    openUrl(m_url);
}

void ArticleViewer::slotOpenLinkInForegroundTab()
{
    OpenURLRequest req(m_url);
    req.setOptions(OpenURLRequest::NewTab);
    emit signalOpenURLRequest(req);
}

void ArticleViewer::slotOpenLinkInBackgroundTab()
{
    OpenURLRequest req(m_url);
    req.setOptions(OpenURLRequest::NewTab);
    req.setOpenInBackground(true);
    emit signalOpenURLRequest(req);
}

void ArticleViewer::slotOpenLinkInBrowser()
{
    OpenURLRequest req(m_url);
    req.setOptions(OpenURLRequest::ExternalBrowser);
    emit signalOpenURLRequest(req);
}

void ArticleViewer::slotSaveLinkAs()
{
    KUrl tmp( m_url );

    if ( tmp.fileName(false).isEmpty() )
        tmp.setFileName( "index.html" );
    KParts::BrowserRun::simpleSave(tmp, tmp.fileName());
}

void ArticleViewer::slotStarted(KIO::Job *)
{
    m_part->widget()->setCursor( Qt::WaitCursor );
}

void ArticleViewer::slotCompleted()
{
    m_part->widget()->unsetCursor();
}

void ArticleViewer::slotScrollUp()
{
    m_part->view()->scrollBy(0,-10);
}

void ArticleViewer::slotScrollDown()
{
    m_part->view()->scrollBy(0,10);
}

void ArticleViewer::slotZoomIn()
{
    int zf = m_part->zoomFactor();
    if (zf < 100)
    {
        zf = zf - (zf % 20) + 20;
        m_part->setZoomFactor(zf);
    }
    else
    {
        zf = zf - (zf % 50) + 50;
        m_part->setZoomFactor(zf < 300 ? zf : 300);
    }
}

void ArticleViewer::slotZoomOut()
{
    int zf = m_part->zoomFactor();
    if (zf <= 100)
    {
        zf = zf - (zf % 20) - 20;
        m_part->setZoomFactor(zf > 20 ? zf : 20);
    }
    else
    {
        zf = zf - (zf % 50) - 50;
        m_part->setZoomFactor(zf);
    }
}

void ArticleViewer::slotSetZoomFactor(int percent)
{
    m_part->setZoomFactor(percent);
}

// some code taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void ArticleViewer::slotPrint( )
{
    m_part->view()->print();
}


void ArticleViewer::setSafeMode()
{
    m_part->setJScriptEnabled(false);
    m_part->setJavaEnabled(false);
    m_part->setMetaRefreshEnabled(false);
    m_part->setPluginsEnabled(false);
    m_part->setDNDEnabled(true);
    m_part->setAutoloadImages(true);
    m_part->setStatusMessagesEnabled(false);
}

void ArticleViewer::connectToNode(TreeNode* node)
{
    if (node)
    {
        if (m_viewMode == CombinedView)
        {
//            connect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
            connect( node, SIGNAL(signalArticlesAdded(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesAdded(TreeNode*, const QList<Article>&)));
            connect( node, SIGNAL(signalArticlesRemoved(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesRemoved(TreeNode*, const QList<Article>&)));
            connect( node, SIGNAL(signalArticlesUpdated(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesUpdated(TreeNode*, const QList<Article>&)));
        }
        if (m_viewMode == SummaryView)
            connect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotShowSummary(TreeNode*) ) );

        connect( node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear() ) );
    }
}

void ArticleViewer::disconnectFromNode(TreeNode* node)
{
    if (node)
    {
//        disconnect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdateCombinedView() ) );
        disconnect( node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear() ) );
        disconnect( node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotShowSummary(TreeNode*) ) );
        disconnect( node, SIGNAL(signalArticlesAdded(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesAdded(TreeNode*, const QList<Article>&)));
        disconnect( node, SIGNAL(signalArticlesRemoved(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesRemoved(TreeNode*, const QList<Article>&)));
        disconnect( node, SIGNAL(signalArticlesUpdated(TreeNode*, const QList<Article>&)), this, SLOT(slotArticlesUpdated(TreeNode*, const QList<Article>&)));

    }
}

void ArticleViewer::renderContent(const QString& text)
{
    m_part->closeURL();
    m_currentText = text;
    beginWriting();
    //kDebug() << text << endl;
    m_part->write(text);
    endWriting();
}

void ArticleViewer::beginWriting()
{
    QString head = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n <html><head><title>.</title>");

    if (m_viewMode == CombinedView)
        head += m_combinedModeCSS;
    else
        head += m_normalModeCSS;

    head += "</style></head><body>";
    m_part->view()->setContentsPos(0,0);
    m_part->begin(m_link);
    m_part->write(head);
}

void ArticleViewer::endWriting()
{
    m_part->write(m_htmlFooter);
    //kDebug() << m_htmlFooter << endl;
    m_part->end();
}


void ArticleViewer::slotShowSummary(TreeNode* node)
{
    m_viewMode = SummaryView;

    if (!node)
    {
        slotClear();
        return;
    }

    if (node != m_node)
    {
        disconnectFromNode(m_node);
        connectToNode(node);
        m_node = node;
    }
    m_showSummaryVisitor->visit(node);
}


void ArticleViewer::slotShowArticle(const Article& article)
{
    m_viewMode = NormalView;
    disconnectFromNode(m_node);
    m_article = article;
    m_node = 0;
    m_link = article.link();
    if (article.feed()->loadLinkedWebsite())
        openUrl(article.link());
    else
        renderContent( m_normalViewFormatter->formatArticle(article, ArticleFormatter::ShowIcon) );
}

bool ArticleViewer::openUrl(const KUrl& url)
{
    if (!m_article.isNull() && m_article.feed()->loadLinkedWebsite())
    {
        return m_part->openUrl(url);
    }
    else
    {
        reload();
        return true;
    }
}

void ArticleViewer::slotSetFilter(const Akregator::Filters::ArticleMatcher& textFilter, const Akregator::Filters::ArticleMatcher& statusFilter)
{
    if (m_statusFilter == statusFilter && m_textFilter == textFilter)
        return;

    m_textFilter = textFilter;
    m_statusFilter = statusFilter;

    slotUpdateCombinedView();
}

void ArticleViewer::slotUpdateCombinedView()
{
    if (m_viewMode != CombinedView)
        return;

    if (!m_node)
        return slotClear();

    QList<Article> articles = m_node->articles();
    qSort(articles);
    QList<Article>::ConstIterator end = articles.end();
    QList<Article>::ConstIterator it = articles.begin();

    QString text;

    int num = 0;
    QTime spent;
    spent.start();

    for ( ; it != end; ++it)
    {
        if ( !(*it).isDeleted() && m_textFilter.matches(*it) && m_statusFilter.matches(*it) )
        {
            text += "<p><div class=\"article\">"+m_combinedViewFormatter->formatArticle(*it, ArticleFormatter::NoIcon)+"</div><p>";
            ++num;
        }
    }
    kDebug() << "Combined view rendering: (" << num << " articles):\n" << "generating HTML: " << spent.elapsed() << "ms " << endl;
    renderContent(text);
    kDebug() << "HTML rendering: " << spent.elapsed() << "ms" << endl;


}

void ArticleViewer::slotArticlesUpdated(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
    if (m_viewMode == CombinedView)
        slotUpdateCombinedView();
}

void ArticleViewer::slotArticlesAdded(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
}

void ArticleViewer::slotArticlesRemoved(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
}

/* testingtesting :)
void ArticleViewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KUrl& kurl, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)
{
    kDebug() << m_link << endl;
    kDebug() << kurl.url() << endl;
}*/


void ArticleViewer::slotClear()
{
    disconnectFromNode(m_node);
    m_node = 0;
    m_article = Article();

    renderContent(QString());
}

void ArticleViewer::slotShowNode(TreeNode* node)
{
    m_viewMode = CombinedView;

    if (node != m_node)
        disconnectFromNode(m_node);

    connectToNode(node);

    m_article = Article();
    m_node = node;

    if (node && !node->articles().isEmpty())
        m_link = node->articles().first().link();
    else
        m_link = KUrl();

    slotUpdateCombinedView();
}

void ArticleViewer::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

void ArticleViewer::slotPaletteOrFontChanged()
{
    updateCss();
    reload();
}

void ArticleViewer::reload()
{
    beginWriting();
    m_part->write(m_currentText);
    endWriting();
}

void ArticleViewer::displayAboutPage()
{
    QString location = KStandardDirs::locate("data", "akregator/about/main.html");

    m_part->begin(KUrl::fromPath( location ));
    QString info =
            i18nc("%1: Akregator version; %2: help:// URL; %3: homepage URL; "
            "--- end of comment ---",
    "<h2 style='margin-top: 0px;'>Welcome to Akregator %1</h2>"
            "<p>Akregator is an RSS feed aggregator for the K Desktop Environment. "
            "Feed aggregators provide a convenient way to browse different kinds of "
            "content, including news, blogs, and other content from online sites. "
            "Instead of checking all your favorite web sites manually for updates, "
            "Akregator collects the content for you.</p>"
            "<p>For more information about using Akregator, check the "
            "<a href=\"%3\">Akregator website</a>. If you do not want to see this page anymore, <a href=\"config:/disable_introduction\">click here</a>.</p>"
            "<p>We hope that you will enjoy Akregator.</p>\n"
            "<p>Thank you,</p>\n"
            "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The Akregator Team</p>\n",
    AKREGATOR_VERSION, // Akregator version
    "http://akregator.sourceforge.net/"); // Akregator homepage URL

    QString fontSize = QString::number( pointsToPixel( Settings::mediumFontSize() ));
    QString appTitle = i18n("Akregator");
    QString catchPhrase = ""; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
    QString quickDescription = i18n("An RSS feed reader for the K Desktop Environment.");

    QString content = KPIM::kFileToByteArray(location);

    QString infocss = KStandardDirs::locate( "data", "libkdepim/about/kde_infopage.css" );
    QString rtl = kapp->isRightToLeft() ? QString("@import \"%1\";" ).arg( KStandardDirs::locate( "data", "libkdepim/about/kde_infopage_rtl.css" )) : QString();

    m_part->write(content.arg(infocss).arg(rtl).arg(fontSize).arg(appTitle).arg(catchPhrase).arg(quickDescription).arg(info));
    m_part->end();
}

ArticleViewerPart::ArticleViewerPart(QWidget* parent) : KHTMLPart(parent),
     m_button(-1)
{
    setXMLFile(KStandardDirs::locate("data", "akregator/articleviewer.rc"), true);
}

int ArticleViewerPart::button() const
{
    return m_button;
}

bool ArticleViewerPart::closeURL()
{
    emit browserExtension()->loadingProgress(-1);
    emit canceled(QString::null);
    return KHTMLPart::closeUrl();
}

void ArticleViewerPart::urlSelected(const QString &url, int button, int state, const QString &_target, KParts::URLArgs args)
{
    m_button = button;
    KHTMLPart::urlSelected(url,button,state,_target,args);
    /*
    if (url == "config:/disable_introduction")
    {
        if(KMessageBox::questionYesNo( widget(), i18n("Are you sure you want to disable this introduction page?"), i18n("Disable Introduction Page"), i18n("Disable"), i18n("Keep Enabled") ) == KMessageBox::Yes)
        {
            KConfig* conf = Settings::self()->config();
            conf->setGroup("General");
            conf->writeEntry("Disable Introduction", "true");
        }
    }
    else
    {
        m_url = completeURL(url);
        browserExtension()->setURLArgs(args);
        if (button == Qt::LeftButton)
        {
            switch (Settings::lMBBehaviour())
            {
                case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
                    slotOpenLinkInBrowser();
                    break;
                case Settings::EnumLMBBehaviour::OpenInBackground:
                    slotOpenLinkInBackgroundTab();
                    break;
                default:
                    slotOpenLinkInForegroundTab();
                    break;
            }
            return;
        }
        else if (button == Qt::MidButton)
        {
            switch (Settings::mMBBehaviour())
            {
                case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
                    slotOpenLinkInBrowser();
                    break;
                case Settings::EnumMMBBehaviour::OpenInBackground:
                    slotOpenLinkInBackgroundTab();
                    break;
                default:
                    slotOpenLinkInForegroundTab();
                    break;
            }
            return;
        }
        KHTMLPart::urlSelected(url,button,state,_target,args);
    }
    */
}

void ArticleViewer::updateCss()
{
    m_normalModeCSS =  m_normalViewFormatter->getCss();
    m_combinedModeCSS = m_combinedViewFormatter->getCss();
}

void ArticleViewer::setNormalViewFormatter(const ArticleFormatter& formatter)
{
    delete m_normalViewFormatter;
    m_normalViewFormatter = formatter.clone();
    m_normalViewFormatter->setPaintDevice(m_part->view());
}

void ArticleViewer::setCombinedViewFormatter(const ArticleFormatter& formatter)
{
    delete m_combinedViewFormatter;
    m_combinedViewFormatter = formatter.clone();
    m_combinedViewFormatter->setPaintDevice(m_part->view());
}

} // namespace Akregator

#include "articleviewer.moc"

