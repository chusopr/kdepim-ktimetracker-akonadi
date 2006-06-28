/*
 * This file was generated by dbusidl2cpp version 0.6
 * Command line was: dbusidl2cpp -m -a knoteadaptor -- org.kde.KNotes.xml
 *
 * dbusidl2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef KNOTEADAPTOR_H_321131151501067
#define KNOTEADAPTOR_H_321131151501067

#include <QtCore/QObject>
#include <dbus/qdbus.h>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface org.kde.KNotes
 */
class KNotesAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KNotes")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.kde.KNotes\" >\n"
"    <method name=\"newNote\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"text\" />\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"newNoteFromClipboard\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\" />\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"killNote\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\" />\n"
"    </method>\n"
"    <method name=\"killNote\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\" />\n"
"      <arg direction=\"in\" type=\"b\" name=\"force\" />\n"
"    </method>\n"
"    <method name=\"setName\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"noteId\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"newText\" />\n"
"    </method>\n"
"    <method name=\"setName\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"noteId\" />\n"
"    </method>\n"
"    <method name=\"showNote\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"noteId\" />\n"
"    </method>\n"
"    <method name=\"hideNote\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"noteId\" />\n"
"    </method>\n"
"    <method name=\"showAllNotes\" />\n"
"    <method name=\"hideAllNotes\" />\n"
"  </interface>\n"
        "")
public:
    KNotesAdaptor(QObject *parent);
    virtual ~KNotesAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void hideAllNotes();
    void hideNote(const QString &noteId);
    Q_ASYNC void killNote(const QString &name, bool force);
    Q_ASYNC void killNote(const QString &name);
    QString newNote(const QString &name, const QString &text);
    QString newNoteFromClipboard(const QString &name);
    void setName(const QString &noteId);
    Q_ASYNC void setName(const QString &noteId, const QString &newText);
    void showAllNotes();
    void showNote(const QString &noteId);
Q_SIGNALS: // SIGNALS
};

#endif
