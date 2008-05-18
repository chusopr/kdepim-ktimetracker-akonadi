#include "opml.h"

#include <KConfigGroup>

#include <kdebug.h>

#include <QTimer>
#include <QDomDocument>

namespace feedsync
{

Opml::Opml(const KConfigGroup& configgroup, QObject* parent)
    : Aggregator( parent ),
    _xmlDoc( 0 ),
    _xmlFile( configgroup.readEntry("Filename") ),
    _loaded( false )
{
    kDebug();
}

Opml::~Opml() 
{
    kDebug();
    delete _xmlDoc;
}

SubscriptionList Opml::getSubscriptionList() const 
{
    kDebug();
    return _subscriptionList;
}

void Opml::load() 
{
    kDebug();

    // If not already done load the file
    if ( !_loaded ) {

    }

    // Read the XML
    _xmlDoc = new QDomDocument("opml");
    if (!_xmlFile.open(QIODevice::ReadOnly)) {
        kDebug() << "File Error";
        return;
    }
    if (!_xmlDoc->setContent(&_xmlFile)) {
        kDebug() << "File Error";
        _xmlFile.close();
        return;
    }
    _xmlFile.close();
    QDomNodeList nodeList = _xmlDoc->elementsByTagName("outline");
    bool firstCat = true;
    QString cat;
    for (int i=0;i<nodeList.count();i++) {
        QDomNode node = nodeList.at(i);
        if (!node.attributes().namedItem("xmlUrl").isNull()) {
            _subscriptionList.add(node.attributes().namedItem("xmlUrl").nodeValue(),
                                  node.attributes().namedItem("title").nodeValue(),
                                  cat);
            firstCat = true;
        } else {
            if (firstCat) {
              cat = "";
              firstCat = false;
            } else {
              cat += '/';
            }
            cat += node.attributes().namedItem("text").nodeValue();
        }
    }

    // Send the signal
    QTimer::singleShot( 0, this, SLOT(sendSignalLoadDone()) );
}

void Opml::sendSignalLoadDone() 
{
    emit loadDone();
}

void Opml::add(const SubscriptionList & list) 
{
    kDebug();
    QDomNode nodeList = _xmlDoc->documentElement().firstChild().nextSibling();

    QString m_rss;
    for (int i=0;i<list.count();i++) {
        m_rss = list.getRss(i);

        // Create element
        QDomElement m_element = _xmlDoc->createElement("outline");
        m_element.setAttribute(QString("title"),list.getName(i));
        m_element.setAttribute(QString("type"),QString("rss"));
        m_element.setAttribute(QString("text"),list.getName(i));
        m_element.setAttribute(QString("xmlUrl"),m_rss);

        // append
        nodeList.appendChild(m_element);
    }

    // TODO Test save
    QFile file;
    QTextStream out;
    file.setFileName("~/out.xml");
    if (!file.open(QIODevice::WriteOnly))
        return;
    out.setDevice(&file);
    _xmlDoc->save(out,2);
    file.close();

    // Send signal
    emit addDone();
}

void Opml::update(const SubscriptionList & list) 
{
    kDebug();

    // Send signal
    emit updateDone();
}

void Opml::remove(const SubscriptionList & list) 
{
    kDebug();

    // Send signal
    emit removeDone();
}

}
