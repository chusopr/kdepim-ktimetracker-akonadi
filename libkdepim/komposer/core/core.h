// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/**
 * core.h
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#ifndef KOMPOSER_CORE_H
#define KOMPOSER_CORE_H

#include <kmainwindow.h>
#include <qptrlist.h>

namespace KSettings {
  class Dialog;
}
class QWidgetStack;

namespace Komposer
{

  class Editor;
  class Plugin;
  class PluginManager;

  /**
   * This class provides the interface to the Komposer core for the editor.
   */
  class Core : public KMainWindow
  {
    Q_OBJECT
  public:
    Core( QWidget *parentWidget = 0, const char *name = 0 );
    virtual ~Core();

  protected slots:
    //void slotActivePartChanged( KParts::Part *part );
    void slotPluginLoaded( Plugin* );
    void slotAllPluginsLoaded();
    void slotPreferences();
    void slotQuit();
    void slotClose();

    void slotSendNow();
    void slotSendLater();
    void slotSaveDraft();
    void slotInsertFile();
    void slotAddrBook();
    void slotNewComposer();
    void slotAttachFile();

  protected:
    virtual void initWidgets();
    void initCore();
    void initConnections();
    void loadSettings();
    void saveSettings();
    void createActions();

    void addEditor( Komposer::Editor *editor );
    void addPlugin( Komposer::Plugin *plugin );

  private:
    QWidgetStack *m_stack;
    Editor *m_currentEditor;
    PluginManager *m_pluginManager;

    KSettings::Dialog *m_dlg;

    class Private;
    Private *d;
};

}

#endif
