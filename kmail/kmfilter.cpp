// kmfilter.cpp
// Author: Stefan Taferner <taferner@kde.org>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kmfilter.h"
#include "kmmessage.h"
#include "kmfilteraction.h"
#include "kmfolder.h"
#include "kmglobal.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdebug.h>
//#include <kapp.h>

#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#include <assert.h>
//#include <string.h>


KMFilter::KMFilter( KConfig* aConfig )
{
  mActions.setAutoDelete( TRUE );

  if ( aConfig )
    readConfig( aConfig );
}


KMFilter::KMFilter( KMFilter * aFilter )
{
  mActions.setAutoDelete( TRUE );

  if ( aFilter ) {
    mPattern = *aFilter->pattern();
    
    QListIterator<KMFilterAction> it( *aFilter->actions() );
    for ( it.toFirst() ; it.current() ; ++it ) {
      KMFilterActionDesc *desc = (*kernel->filterActionDict())[ (*it)->name() ];
      if ( desc ) {
	KMFilterAction *f = desc->create();
	if ( f ) {
	  f->argsFromString( (*it)->argsAsString() );
	  mActions.append( f );
	}
      }
    }
  }
}


KMFilter::ReturnCode KMFilter::execActions( KMMessage* msg, bool& stopIt ) const
{
  ReturnCode status = NoResult;
  stopIt = FALSE;

  QListIterator<KMFilterAction> it( mActions );
  for ( it.toFirst() ; !stopIt && it.current() ; ++it ) {

    kdDebug() << "####### KMFilter::process: going to apply action "
	      << (*it)->label() << " \"" << (*it)->argsAsString()
	      << "\"" << endl;

    KMFilterAction::ReturnCode result = (*it)->process( msg, stopIt );

    switch ( result ) {
    case KMFilterAction::CriticalError:
      return CriticalError;
    case KMFilterAction::ErrorButGoOn:
      // Small problem, keep a copy
      status = GoOn;
      break;
    case KMFilterAction::Finished:
      kdDebug() << "got result Finished" << endl;
      if ( status == NoResult )
	// Message saved in a folder
	status = MsgExpropriated;
    }
  }

  if ( status == NoResult ) // No filters matched, keep copy of message
    status = GoOn;

  return status;
}


bool KMFilter::folderRemoved( KMFolder* aFolder, KMFolder* aNewFolder )
{
  bool rem = FALSE;

  QListIterator<KMFilterAction> it( mActions );
  for ( it.toFirst() ; it.current() ; ++it )
    if ( (*it)->folderRemoved( aFolder, aNewFolder ) )
      rem = TRUE;

  return rem;
}


//-----------------------------------------------------------------------------
void KMFilter::readConfig(KConfig* config)
{
  // MKSearchPattern::readConfig ensures
  // it the pattern is purified.
  mPattern.readConfig(config);

  int i, numActions;
  QString actName, argsName;
  QStringList actNames, actArgs;

  mActions.clear();

  numActions = config->readNumEntry("actions",0);
  if (numActions > FILTER_MAX_ACTIONS) {
    numActions = FILTER_MAX_ACTIONS ;
    KMessageBox::information( 0, i18n("Too many filter actions in filter rule `%1'").arg( mPattern.name() ) );
  }

  // (mmutz) Some KMFilterAction* constructors want to read
  // from the config, too. So make sure we have read our stuff
  // before they can switch the group...
  for ( i=0 ; i < numActions ; i++ ) {
    actNames.append( config->readEntry( actName.sprintf("action-name-%d", i) ) );
    actArgs.append( config->readEntry( argsName.sprintf("action-args-%d", i) ) );
  }

  QStringList::Iterator nIt = actNames.begin(), aIt = actArgs.begin();
  for ( ; nIt != actNames.end() && aIt != actArgs.end(); ++nIt, ++aIt ) {
    KMFilterActionDesc *desc = (*kernel->filterActionDict())[ (*nIt) ];
    if ( desc ) {
      //...create an instance...
      KMFilterAction *fa = desc->create();
      if ( fa ) {
	//...load it with it's parameter...
	fa->argsFromString( *aIt );
	//...check if it's emoty and...
	if ( !fa->isEmpty() ) {
	  //...append it if it's not and...
	  mActions.append( fa );
	} else {
	  //...delete it else.
	  delete fa;
	}
      }
    } else {
      KMessageBox::information( 0 /* app-global modal dialog box */,
				i18n("Unknown filter action `%1'\n in filter rule `%2'."
				     "\nIgnoring it.").arg( *nIt ).arg( mPattern.name() ) );
    }
  }
}


void KMFilter::writeConfig(KConfig* config) const
{
  mPattern.writeConfig(config);

  QString key;
  int i;

  QListIterator<KMFilterAction> it( mActions );
  for ( i=0, it.toFirst() ; it.current() ; ++it, ++i ) {
    config->writeEntry( key.sprintf("action-name-%d", i),
			(*it)->name() );
    config->writeEntry( key.sprintf("action-args-%d", i),
			(*it)->argsAsString() );
  }
  config->writeEntry("actions", i );
}

void KMFilter::purify()
{
  mPattern.purify();
  
  QListIterator<KMFilterAction> it( mActions );
  it.toLast();
  while ( it.current() )
    if ( (*it)->isEmpty() )
      mActions.remove ( (*it) );
    else
      --it;

}

const QString KMFilter::asString() const
{
  QString result;

  result += mPattern.asString();

  QListIterator<KMFilterAction> it( mActions );
  for ( it.toFirst() ; it.current() ; ++it ) {
    result += "    action: ";
    result += (*it)->label();
    result += " ";
    result += (*it)->argsAsString();
    result += "\n";
  }
  result += "}";

  return result;
}

