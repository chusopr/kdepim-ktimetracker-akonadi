/*
  This file is part of CalendarSupport

  Copyright (c) 2010 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "history.h"
#include "history_p.h"
#include "utils.h"

#include <KLocale>

using namespace KCalCore;
using namespace CalendarSupport;

History::History( IncidenceChanger *changer, QWidget *parent ) : QObject( parent ),
                                                                d( new Private( this ) )
{
  Q_ASSERT( changer );

  d->mParent = parent;
  d->mChanger = changer;
  d->mOperationTypeInProgress = TypeNone;

  connect( d->mChanger, SIGNAL(incidenceAddFinished(Akonadi::Item,bool)),
           d, SLOT(addFinished(Akonadi::Item,bool)) );

  connect( d->mChanger, SIGNAL(incidenceDeleteFinished(Akonadi::Item,bool)),
           d, SLOT(deleteFinished(Akonadi::Item,bool)) );

  connect( d->mChanger,
  SIGNAL(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,CalendarSupport::IncidenceChanger::WhatChanged,bool)),
  d,SLOT(editFinished(Akonadi::Item,Akonadi::Item,CalendarSupport::IncidenceChanger::WhatChanged,bool)) );

}

History::~History()
{
  delete d;
}

void History::recordChange( const Akonadi::Item &oldItem,
                            const Akonadi::Item &newItem,
                            History::ChangeType changeType,
                            IncidenceChanger::WhatChanged whatChanged,
                            const uint atomicOperationId )
{
  Akonadi::Item::Id id = -1;

  // First, do some asserting.
  switch( changeType ) {
    case ChangeTypeAdd:
      Q_ASSERT_X( !oldItem.isValid() && newItem.isValid() && newItem.hasPayload<Incidence::Ptr>(),
                  "recordChange()", "oldItem must be invalid, and newItem valid" );
      id = newItem.id();
      d->mLatestRevisionByItemId.insert( id, newItem.revision() );
      break;
    case ChangeTypeDelete:
      Q_ASSERT_X( oldItem.isValid() && oldItem.hasPayload<Incidence::Ptr>() && !newItem.isValid(),
                  "recordChange()", "oldItem must be valid, and newItem invalid" );
      id = oldItem.id();
      d->mLatestRevisionByItemId.remove( id );
      break;
    case ChangeTypeEdit:
      Q_ASSERT_X( oldItem.isValid() && oldItem.hasPayload<Incidence::Ptr>() &&
                  newItem.isValid() && newItem.hasPayload<Incidence::Ptr>() &&
                  oldItem.id() == newItem.id(),
                  "recordChange()", "oldItem and newItem must be valid and have the same id" );
      id = oldItem.id();
      d->mLatestRevisionByItemId.insert( id, newItem.revision() );
      break;
    default:
      kDebug() << "changeType = " << changeType;
      Q_ASSERT_X( false, "recordChange()", "Invalid change type" );
  }

  Entry entry;
  entry.oldItem = oldItem;
  entry.newItem = newItem;
  entry.atomicOperationId = atomicOperationId;
  entry.itemId = id;
  entry.changeType = changeType;
  entry.whatChanged = whatChanged;

  d->mUndoStack.push( entry );
  d->mRedoStack.clear();
  //emit undoAvailable(); // nao gosto, se poder tirar.
  d->updateWidgets();
}

void History::registerRedoWidget( QWidget *w )
{
  if ( !d->mRedoWidgets.contains( w ) ) {
    d->mRedoWidgets.append( QPointer<QWidget>( w ) );
    w->setEnabled( d->isRedoAvailable() );
  }
}

void History::registerUndoWidget( QWidget *w )
{
  if ( !d->mUndoWidgets.contains( w ) ) {
    d->mUndoWidgets.append( QPointer<QWidget>( w ) );
    w->setEnabled( d->isUndoAvailable() );
  }
}

bool History::undo()
{
  // Don't call undo() without the previous one finishing
  Q_ASSERT( d->mOperationTypeInProgress == TypeNone );

  bool result;

  if ( !d->mUndoStack.isEmpty() ) {
    result = d->doIt( d->mUndoStack.pop(), TypeUndo );
  } else {
    kWarning() << "Don't call undo when the undo stack is empty." << endl;
    result = false;
  }

  return result;
}

bool History::redo()
{
  // Don't call redo() without the previous one finishing
  Q_ASSERT( d->mOperationTypeInProgress == TypeNone );

  bool result;

  if ( !d->mRedoStack.isEmpty() ) {
    result = d->doIt( d->mRedoStack.pop(), TypeRedo );
  } else {
    kWarning() << "Don't call redo() when the undo stack is empty." << endl;
    result = false;
  }

  return result;
}

bool History::clear()
{
  if ( d->mOperationTypeInProgress == TypeNone ) {
    d->mRedoStack.clear();
    d->mUndoStack.clear();
    d->updateWidgets();
    return true;
  } else {
    return false;
  }
}

QString History::lastErrorString() const
{
  return d->mLastErrorString;
}

bool History::Private::isUndoAvailable() const
{
  return !mUndoStack.isEmpty() && mOperationTypeInProgress == TypeNone;
}

bool History::Private::isRedoAvailable() const
{
  return !mRedoStack.isEmpty() && mOperationTypeInProgress == TypeNone;
}

void History::Private::updateWidgets()
{
  const bool undoAvailable = isUndoAvailable();
  const bool redoAvailable = isRedoAvailable();

  foreach( QPointer<QWidget> w, mUndoWidgets )
    if ( w )
      w->setEnabled( undoAvailable );

  foreach( QPointer<QWidget> w, mRedoWidgets )
    if ( w )
      w->setEnabled( redoAvailable );
}

void History::Private::updateIds( Item::Id oldId, Item::Id newId )
{
  for ( int i = 0; i < mUndoStack.count(); ++i ) {
    if ( mUndoStack[i].itemId == oldId ) {

      mUndoStack[i].itemId = newId;

      if ( mUndoStack[i].oldItem.isValid() )
        mUndoStack[i].oldItem.setId( newId );

      if ( mUndoStack[i].newItem.isValid() )
        mUndoStack[i].newItem.setId( newId );
    }
  }

  for ( int i = 0; i < mRedoStack.count(); ++i ) {
    if ( mRedoStack[i].itemId == oldId ) {

      mRedoStack[i].itemId = newId;

      if ( mRedoStack[i].oldItem.isValid() )
        mRedoStack[i].oldItem.setId( newId );

      if ( mRedoStack[i].newItem.isValid() )
        mRedoStack[i].newItem.setId( newId );
    }
  }

  if ( mEntryInProgress.oldItem.isValid() )
    mEntryInProgress.oldItem.setId( newId );

  if ( mEntryInProgress.newItem.isValid() )
    mEntryInProgress.newItem.setId( newId );

  mEntryInProgress.itemId = newId;
}

bool History::Private::doIt( const Entry &entry, OperationType type )
{
  mOperationTypeInProgress = type;
  mEntryInProgress = entry;
  updateWidgets();

  Entry e = entry;

  if ( type == TypeUndo ) {
    // Invert stuff, that's what undo means.
    switch( e.changeType ) {
      case ChangeTypeAdd:
        e.changeType = ChangeTypeDelete;
        break;
      case ChangeTypeDelete:
        e.changeType = ChangeTypeAdd;
        break;
      case ChangeTypeEdit:
        break;
      default:
        Q_ASSERT_X( false, "doIt()", "Invalid change type" );
    }

    // Swap old item with new item.
    Akonadi::Item oldItem2 = e.oldItem;
    e.oldItem = e.newItem;
    e.newItem = oldItem2;
  }

  Incidence::Ptr oldPayload = CalendarSupport::incidence( e.oldItem );
  Incidence::Ptr newPayload = CalendarSupport::incidence( e.newItem );

  bool result;
  if ( e.changeType == ChangeTypeAdd ) {
    Akonadi::Collection collection = e.newItem.parentCollection();
    result = mChanger->addIncidence( newPayload, collection, mParent, e.atomicOperationId );
    // now wait for mChanger to call our slot.
  } else if ( e.changeType == ChangeTypeDelete ) {
    Akonadi::Item item = e.oldItem;
    result = mChanger->deleteIncidence( item, e.atomicOperationId, mParent );
    // now wait for mChanger to call our slot.
  } else if ( e.changeType == ChangeTypeEdit ) {
    if ( mLatestRevisionByItemId.contains( e.itemId ) ) {
      e.newItem.setRevision( mLatestRevisionByItemId[e.itemId] );
    }

    result = mChanger->changeIncidence( oldPayload, e.newItem, e.whatChanged, mParent, e.atomicOperationId );
    // now wait for mChanger to call our slot.
  } else {
    result = false;
    Q_ASSERT_X( false, "History::Private::doIt()", "Must have at least one payload" );
  }

  if ( !result ) {
    // Don't i18n yet, only after refactoring IncidenceChanger.
    mLastErrorString = "Error in incidence changer, didn't even fire the job";
    mOperationTypeInProgress = TypeNone;
    stack().push( mEntryInProgress ); // Back to the original stack
    updateWidgets();
  }

  return result;
}

void History::Private::deleteFinished( const Akonadi::Item &item, bool success )
{
  History::ResultCode resultCode = success ? History::ResultCodeSuccess :
                                             History::ResultCodeError;
  // clean up hash
  if ( success )
    mLatestRevisionByItemId.remove( item.id() );

  finishOperation( resultCode );
}

void History::Private::addFinished( const Akonadi::Item &item, bool success )
{
  History::ResultCode resultCode;

  if ( success ) {
    resultCode = History::ResultCodeSuccess;
    // Por comentário.
    updateIds( mEntryInProgress.itemId /*old*/, item.id() /*new*/ );
    mLatestRevisionByItemId.insert( item.id(), item.revision() );
  } else {
    resultCode = History::ResultCodeError;
  }

  finishOperation( resultCode );
}

void History::Private::editFinished( const Akonadi::Item &oldItem,
                                     const Akonadi::Item &newItem,
                                     CalendarSupport::IncidenceChanger::WhatChanged,
                                     bool success )
{
  // TODO: IncidenceChanger needs not to send these ones.
  Q_UNUSED( oldItem );
  Q_UNUSED( newItem );

  History::ResultCode resultCode = success ? History::ResultCodeSuccess :
                                             History::ResultCodeError;

  if ( success )
    mLatestRevisionByItemId[newItem.id()] = newItem.revision();

  finishOperation( resultCode );
}

// Just to share code between {add|change|delete}Finished
void History::Private::finishOperation( History::ResultCode resultCode )
{
  if ( resultCode == ResultCodeSuccess ) {
    mLastErrorString = QString();
    destinationStack().push( mEntryInProgress );
  } else {
    // Not very verbose. That's IncidenceChanger's fault. And that will change soon.
    mLastErrorString = i18n( "error" );
    stack().push( mEntryInProgress );
  }

  emitDone( mOperationTypeInProgress, resultCode );

  mOperationTypeInProgress = TypeNone;
  updateWidgets();
}

QStack<Entry>& History::Private::stack()
{
  // Entries from the undo stack go to the redo stack, and vice-versa
  if ( mOperationTypeInProgress == TypeUndo ) {
    return mUndoStack;
  } else {
    return mRedoStack;
  }
}

QStack<Entry>& History::Private::destinationStack()
{
  // Entries from the undo stack go to the redo stack, and vice-versa
  if ( mOperationTypeInProgress == TypeUndo ) {
    return mRedoStack;
  } else {
    return mUndoStack;
  }
}

void History::Private::emitDone( OperationType type, History::ResultCode resultCode )
{
  if ( type == TypeUndo ) {
    emit q->undone( resultCode );
  } else {
    emit q->redone( resultCode );
  }
}

#include "history.moc"
#include "history_p.moc"