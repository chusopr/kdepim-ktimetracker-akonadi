/*
       This file is part of Kaplan
       Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

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

#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>

#include "kpcore.h"


#include "knotes_plugin.h"
#include "knotes_part.h"

typedef KGenericFactory< KNotesPlugin, Kaplan::Core > KNotesPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpknotesplugin, KNotesPluginFactory( "kpknotesplugin" ) );

KNotesPlugin::KNotesPlugin(Kaplan::Core *_core, const char *name, const QStringList &)
  : Kaplan::Plugin(_core, _core, name), m_part(0)
{
  setInstance(KNotesPluginFactory::instance());

//  (void) new KAction(i18n("New note"),0, this, SLOT(slotNewNote()), actionCollection(), "edit_newnote");
  
  setXMLFile("kpknotesplugin.rc");

  core()->addMainEntry(i18n("Notes"), "knotes", this, SLOT(slotShowNotes()));
}


KNotesPlugin::~KNotesPlugin()
{
}


void KNotesPlugin::slotKNotesMenu()
{
  KMessageBox::information(0, "KNotes menu activated");
}


void KNotesPlugin::slotShowNotes()
{
  if (!m_part)
  {
    m_part = new KNotesPart(this, "notes");
    core()->addPart(m_part);
  }

  core()->showView(m_part->widget()); 
}

#include "knotes_plugin.moc"
