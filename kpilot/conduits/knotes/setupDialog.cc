// setupDialog.cc
//
// Copyright (C) 2000 Dan Pilone, Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
// This is setupDialog.cc for KDE 2 / KPilot 4


// This is the setup dialog for null-conduit.
// Because null-conduit does nothing, the
// setup is fairly simple.
//
//

#include "options.h"

#include <stream.h>
#include <kdebug.h>
#include "kpilotlink.h"
#include "setupDialog.moc"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *id="$Id$";



/* static */ const QString KNotesOptions::KNotesGroup("conduitKNote");

KNotesOptions::KNotesOptions(QWidget *parent) :
	setupDialog(parent, "conduitKNotes",0L)
{
	FUNCTIONSETUP;
	addPage(new setupInfoPage(this));
	adjustSize();
}

  
// $Log$
