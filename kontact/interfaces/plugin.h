/*
   This file is part of KDE Kontact.

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KONTACT_PLUGIN_H
#define KONTACT_PLUGIN_H

#include <qobject.h>
#include <kxmlguiclient.h>
#include <qptrlist.h>

class QStringList;
class DCOPClient;
class DCOPObject;
class KAboutData;
class KAction;
class QWidget;

namespace Kontact
{

class Core;
class Summary;

/**
  Base class for all Plugins in Kontact. Inherit from it
  to get a plugin. It can insert an icon into the sidepane,
  add widgets to the widgetstack and add menu items via XMLGUI.
 */
class Plugin : public QObject, virtual public KXMLGUIClient
{
  Q_OBJECT

  public:
    /**
      Creates a new Plugin, note that @param name is required if
      you want your plugin to do dcop via it's own instance of
      @ref DCOPClient by calling @ref dcopClient.
      @note @ref name MUST be the name of the application that
      provides the part!
    */
    Plugin( Core *core, QObject *parent, const char *name );

    ~Plugin();

    /**
      Sets the identifier.
    */
    void setIdentifier( const QString &identifier );

    /**
      Returns the identifier. It is used as argument for several
      methods of Kontacts core.
    */
    QString identifier() const;

    /**
      Sets the localized title.
     */
    void setTitle( const QString &title );

    /**
      Returns the localized title.
    */
    QString title() const;

    /**
      Sets the icon name.
    */
    void setIcon( const QString &icon );

    /**
      Returns the icon name.
    */
    QString icon() const;

    /**
      Sets the name of executable (if existant).
    */
    void setExecutableName( const QString &bin );

    /**
      Returns the name of the binary (if existant).
    */
    QString executableName() const;

    /**
      Set name of library which contains the KPart used by this plugin.
    */
    void setPartLibraryName( const QCString & );

    /**
      Create the DCOP interface for the given @p serviceType, if this
      plugin provides it. Return false otherwise.
    */
    virtual bool createDCOPInterface( const QString& /*serviceType*/ ) { return false; }

    /**
      Reimplement this method and return wether a standalone application is still running
      This is only required if your part is also available as standalone application.
    */
    virtual bool isRunningStandalone() { return false; }

    /**
      Reimplement this method if your application needs a different approach to be brought
      in the foreground. The default behaviour is calling the binary.
      This is only required if your part is also available as standalone application.
    */
    virtual void bringToForeground();

    /**
      Reimplement this method if you want to add your credits to the Kontact
      about dialog.
    */
    virtual const KAboutData *aboutData();

    /**
      You can use this method if you need to access the current part. You can be
      sure that you always get the same pointer as long as the part has not been
      deleted.
    */
    KParts::Part *part();

     /**
       Reimplement this method and return the a path relative to "data" to the tips file.
     */
    virtual QString tipFile() const;

    /**
      This function is called when the plugin is selected by the user before the
      widget of the KPart belonging to the plugin is raised.
    */
    virtual void select();

    /**
      Reimplement this method if you want to add a widget for your application
      to Kontact's summary page.

      @param parent parent widget.
    */
    virtual Summary *createSummaryWidget( QWidget * /*parent*/ ) { return 0; }

    /**
      Reimplement this method if you don't want to have a plugin shown in the sidebar.
    */
    virtual bool showInSideBar() const { return true; }

    /**
      Retrieve the current DCOP Client for the plugin.

      The clients name is taken from the name argument in the constructor.
      @note: The DCOPClient object will only be created when this method is
      called for the first time. Make sure that the part has been loaded
      before calling this method, if it's the one that contains the DCOP
      interface that other parts might use.
    */
    DCOPClient *dcopClient() const;

    /**
      Return the weight of the plugin. The higher the weight the lower it will
      be displayed in the sidebar. The default implementation returns 0.
    */
    virtual int weight() const { return 0; }

    /**
      Insert "New" action.
    */
    void insertNewAction( KAction *action );

    /**
      FIXME: write API doc for Kontact::Plugin::newActions().
    */
    QPtrList<KAction>* newActions() const;

    /**
      Return, if the plugin can handle the drag object of the given mime type.
    */
    virtual bool canDecodeDrag( QMimeSource * ) { return false; }

    /**
      Process drop event.
    */
    virtual void processDropEvent( QDropEvent * ) {}

    Core *core() const;

  protected:
    /**
      Reimplement and retun the part here. Reimpleneting createPart() is
      mandatory!
    */
    virtual KParts::Part *createPart() = 0;

    KParts::Part *loadPart();

  private slots:
    void partDestroyed();

  private:
    class Private;
    Private *d;
};

}

#endif

// vim: sw=2 et sts=2 tw=80
