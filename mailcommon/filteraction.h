/*
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef MAILCOMMON_FILTERACTION_H
#define MAILCOMMON_FILTERACTION_H

#include <kmime/kmime_mdn.h>

#include <QList>
#include <QMultiHash>
#include <QStringList>
#include <akonadi/collection.h>

#include  <kmime/kmime_message.h>

#include "mailcommon_export.h"

namespace Akonadi {
  class Item;
}

class KTemporaryFile;
class QWidget;

namespace MailCommon {

//=========================================================
//
// class FilterAction
//
//=========================================================


/** Abstract base class for KMail's filter actions. All it can do is
    hold a name (ie. type-string). There are several sub-classes that
    inherit form this and are capable of providing parameter handling
    (import/export as string, a widget to allow editing, etc.)

    @short Abstract base class for KMail's filter actions.
    @author Marc Mutz <Marc@Mutz.com>, based on work by Stefan Taferner <taferner@kde.org>.
    @see Filter FilterMgr
*/
class MAILCOMMON_EXPORT FilterAction
{
public:
  /** Possible return codes of process:

      @li @p ErrorNeedComplete: Could not process because a
      complete message is needed.

      @li @p GoOn: Go on with applying filter actions.

      @li @p ErrorButGoOn: There was a non-critical error (e.g. an
      invalid address in the 'forward' action), but the processing
      should continue.

      @li @p CriticalError: A critical error has occurred during
      processing (e.g. "disk full").

  */
  enum ReturnCode { ErrorNeedComplete = 0x1, GoOn = 0x2, ErrorButGoOn = 0x4,
		    CriticalError = 0x8 };
  /** Initialize filter action with (english) name @p aName and
      (internationalized) label @p aLabel. */
  FilterAction(const char* aName, const QString &aLabel);
  virtual ~FilterAction();

  /** Returns nationalized label, ie. the one which is presented in
      the filter dialog. */
  const QString label() const { return mLabel; }

  /** Returns english name, ie. the one under which it is known in the
      config. */
  const QString name() const { return mName; }

  /** Execute action on given message. Returns @p CriticalError if a
      critical error has occurred (eg. disk full), @p ErrorButGoOn if
      there was a non-critical error (e.g. invalid address in
      'forward' action), @p ErrorNeedComplete if a complete message
      is required, @p GoOn if the message shall be processed by
      further filters and @p Ok otherwise.
  */
  virtual ReturnCode process( const Akonadi::Item& item) const = 0;

  /** Determines if the action depends on the body of the message
  */
  virtual bool requiresBody() const;

  /** Determines whether this action is valid. But this is just a
      quick test. Eg., actions that have a mail address as parameter
      shouldn't try real address validation, but only check if the
      string representation is empty. */
  virtual bool isEmpty() const { return false; }

  /** Creates a widget for setting the filter action parameter. Also
      sets the value of the widget. */
  virtual QWidget* createParamWidget(QWidget* parent) const;

  /** The filter action shall set it's parameter from the widget's
      contents. It is allowed that the value is read by the action
      before this function is called. */
  virtual void applyParamWidgetValue(QWidget* paramWidget);

  /** The filter action shall set it's widget's contents from it's
      parameter. */
  virtual void setParamWidgetValue(QWidget* paramWidget) const;

  /** The filter action shall clear it's parameter widget's
      contents. */
  virtual void clearParamWidget(QWidget* paramWidget) const;

  /** Read extra arguments from given string. */
  virtual void argsFromString(const QString &argsStr) = 0;

  /** Return extra arguments as string. Must not contain newlines. */
  virtual const QString argsAsString() const = 0;

  /** Returns a translated string describing this filter for visualization
      purposes, e.g. in the filter log. */
  virtual const QString displayString() const = 0;

  /** Called from the filter when a folder is removed.  Tests if the
      folder @p aFolder is used and changes to @p aNewFolder in this
      case. Returns true if a change was made.  */
  virtual bool folderRemoved(const Akonadi::Collection & aFolder, const Akonadi::Collection & aNewFolder);

  /** Static function that creates a filter action of this type. */
  static FilterAction* newAction();

  /** Automates the sending of MDNs from filter actions. */
  static void sendMDN( const Akonadi::Item& item, KMime::MDN::DispositionType d,
		       const QList<KMime::MDN::DispositionModifier> & m
		       = QList<KMime::MDN::DispositionModifier>() );

private:
  QString mName;
  QString mLabel;
};

//=========================================================
//
// class FilterActionWithNone
//
//=========================================================


/** Abstract base class for KMail's filter actions that need no
    parameter, e.g. "Confirm Delivery". Creates an (empty) QWidget as
    parameter widget. A subclass of this must provide at least
    implementations for the following methods:

    @li virtual FilterAction::ReturnCodes FilterAction::process
    @li static FilterAction::newAction

    @short Abstract base class for filter actions with no parameter.
    @author Marc Mutz <Marc@Mutz.com>, based upon work by Stefan Taferner <taferner@kde.org>
    @see FilterAction Filter

*/
class FilterActionWithNone : public FilterAction
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
  FilterActionWithNone(const char* aName, const QString &aLabel);

  /** Read extra arguments from given string. This type of filter
      action has no parameters, so this is a no-op. */
  virtual void argsFromString(const QString &/*argStr*/) {}

  /** Return extra arguments as string. Must not contain newlines. We
      return QString(), because we have no parameter. */
  virtual const QString argsAsString() const { return QString(); }

  /** Returns a translated string describing this filter for visualization
      purposes, e.g. in the filter log. */
  virtual const QString displayString() const;
};


//=========================================================
//
// class FilterActionWithString
//
//=========================================================


/** Abstract base class for KMail's filter actions that need a
    free-form parameter, e.g. 'set transport' or 'set reply to'.  Can
    create a QLineEdit as parameter widget. A subclass of this
    must provide at least implementations for the following methods:

    @li virtual FilterAction::ReturnCodes FilterAction::process
    @li static FilterAction::newAction

    @short Abstract base class for filter actions with a free-form string as parameter.
    @author Marc Mutz <Marc@Mutz.com>, based upon work by Stefan Taferner <taferner@kde.org>
    @see FilterAction Filter

*/
class FilterActionWithString : public FilterAction
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
  FilterActionWithString(const char* aName, const QString &aLabel);

  /** Determines whether this action is valid. But this is just a
      quick test. Eg., actions that have a mail address as parameter
      shouldn't try real address validation, but only check if the
      string representation is empty. */
  virtual bool isEmpty() const { return mParameter.trimmed().isEmpty(); }

  /** Creates a widget for setting the filter action parameter. Also
      sets the value of the widget. */
  virtual QWidget* createParamWidget(QWidget* parent) const;

  /** The filter action shall set it's parameter from the widget's
      contents. It is allowed that the value is read by the action
      before this function is called. */
  virtual void applyParamWidgetValue(QWidget* paramWidget);

  /** The filter action shall set it's widget's contents from it's
      parameter. */
  virtual void setParamWidgetValue(QWidget* paramWidget) const;

  /** The filter action shall clear it's parameter widget's
      contents. */
  virtual void clearParamWidget(QWidget* paramWidget) const;

  /** Read extra arguments from given string. */
  virtual void argsFromString(const QString &argsStr);

  /** Return extra arguments as string. Must not contain newlines. */
  virtual const QString argsAsString() const;

  /** Returns a translated string describing this filter for visualization
      purposes, e.g. in the filter log. */
  virtual const QString displayString() const;

protected:
  QString mParameter;
};

//=========================================================
//
// class FilterActionWithUOID
//
//=========================================================


/** Abstract base class for KMail's filter actions that need a
    parameter that has a UOID, e.g. "set identity". A subclass of this
    must provide at least implementations for the following methods:

    @li virtual FilterAction::ReturnCodes FilterAction::process
    @li static FilterAction::newAction
    @li the *ParamWidget* methods.

    @short Abstract base class for filter actions with a free-form string as parameter.
    @author Marc Mutz <Marc@Mutz.com>, based upon work by Stefan Taferner <taferner@kde.org>
    @see FilterAction Filter

*/
class FilterActionWithUOID : public FilterAction
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
  FilterActionWithUOID(const char* aName, const QString &aLabel);

  /** Determines whether this action is valid. But this is just a
      quick test. Eg., actions that have a mail address as parameter
      shouldn't try real address validation, but only check if the
      string representation is empty. */
  virtual bool isEmpty() const { return mParameter == 0; }

  /** Read extra arguments from given string. */
  virtual void argsFromString(const QString &argsStr);

  /** Return extra arguments as string. Must not contain newlines. */
  virtual const QString argsAsString() const;

  /** Returns a translated string describing this filter for visualization
      purposes, e.g. in the filter log. */
  virtual const QString displayString() const;

protected:
  uint mParameter;
};

//=========================================================
//
// class FilterActionWithStringList
//
//=========================================================


/** Abstract base class for KMail's filter actions that need a
    parameter which can be chosen from a fixed set, e.g. 'set
    identity'.  Can create a KComboBox as parameter widget. A
    subclass of this must provide at least implementations for the
    following methods:

    @li virtual FilterAction::ReturnCodes FilterAction::process
    @li static  FilterAction::newAction

    Additionally, it's constructor should populate the
    QStringList @p mParameterList with the valid parameter
    strings. The combobox will then contain be populated automatically
    with those strings. The default string will be the first one.

    @short Abstract base class for filter actions with a fixed set of string parameters.
    @author Marc Mutz <Marc@Mutz.com>, based upon work by Stefan Taferner <taferner@kde.org>
    @see FilterActionWithString FilterActionWithFolder FilterAction Filter

*/
class FilterActionWithStringList : public FilterActionWithString
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
  FilterActionWithStringList(const char* aName, const QString &aLabel);

  /** Creates a widget for setting the filter action parameter. Also
      sets the value of the widget. */
  virtual QWidget* createParamWidget(QWidget* parent) const;

  /** The filter action shall set it's parameter from the widget's
      contents. It is allowed that the value is read by the action
      before this function is called. */
  virtual void applyParamWidgetValue(QWidget* paramWidget);

  /** The filter action shall set it's widget's contents from it's
      parameter. */
  virtual void setParamWidgetValue(QWidget* paramWidget) const;

  /** The filter action shall clear it's parameter widget's
      contents. */
  virtual void clearParamWidget(QWidget* paramWidget) const;

  /** Read extra arguments from given string. */
  virtual void argsFromString(const QString &argsStr);

protected:
  QStringList mParameterList;
};


//=========================================================
//
// class FilterActionWithFolder
//
//=========================================================


/** Abstract base class for KMail's filter actions that need a
    mail folder as parameter, e.g. 'move into folder'. Can
    create a KComboBox as parameter widget. A subclass of this
    must provide at least implementations for the following methods:

    @li virtual FilterAction::ReturnCodes FilterAction::process
    @li static FilterAction::newAction

    @short Abstract base class for filter actions with a mail folder as parameter.
    @author Marc Mutz <Marc@Mutz.com>, based upon work by Stefan Taferner <taferner@kde.org>
    @see FilterActionWithStringList FilterAction Filter

*/

class FilterActionWithFolder : public FilterAction
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
  FilterActionWithFolder(const char* aName, const QString &aLabel);

  /** Determines whether this action is valid. But this is just a
      quick test. Eg., actions that have a mail address as parameter
      shouldn't try real address validation, but only check if the
      string representation is empty. */
  virtual bool isEmpty() const { return (!mFolder.isValid() && mFolderName.isEmpty()); }

  /** Creates a widget for setting the filter action parameter. Also
      sets the value of the widget. */
  virtual QWidget* createParamWidget(QWidget* parent) const;

  /** The filter action shall set it's parameter from the widget's
      contents. It is allowed that the value is read by the action
      before this function is called. */
  virtual void applyParamWidgetValue(QWidget* paramWidget);

  /** The filter action shall set it's widget's contents from it's
      parameter. */
  virtual void setParamWidgetValue(QWidget* paramWidget) const;

  /** The filter action shall clear it's parameter widget's
      contents. */
  virtual void clearParamWidget(QWidget* paramWidget) const;

  /** Read extra arguments from given string. */
  virtual void argsFromString(const QString &argsStr);

  /** Return extra arguments as string. Must not contain newlines. */
  virtual const QString argsAsString() const;

  /** Returns a translated string describing this filter for visualization
      purposes, e.g. in the filter log. */
  virtual const QString displayString() const;

  /** Called from the filter when a folder is removed.  Tests if the
      folder @p aFolder is used and changes to @p aNewFolder in this
      case. Returns true if a change was made.  */
  virtual bool folderRemoved(const Akonadi::Collection& aFolder, const Akonadi::Collection& aNewFolder);

protected:
  Akonadi::Collection mFolder;
  QString mFolderName;
};

//=========================================================
//
// class FilterActionWithAddress
//
//=========================================================


/** Abstract base class for KMail's filter actions that need a mail
    address as parameter, e.g. 'forward to'. Can create a
    KComboBox (capable of completion from the address book) as
    parameter widget. A subclass of this must provide at least
    implementations for the following methods:

    @li virtual FilterAction::ReturnCodes FilterAction::process
    @li static FilterAction::newAction

    @short Abstract base class for filter actions with a mail address as parameter.
    @author Marc Mutz <Marc@Mutz.com>, based upon work by Stefan Taferner <taferner@kde.org>
    @see FilterActionWithString FilterAction Filter

*/
class FilterActionWithAddress : public FilterActionWithString
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
  FilterActionWithAddress(const char* aName, const QString &aLabel);

  /** Creates a widget for setting the filter action parameter. Also
      sets the value of the widget. */
  virtual QWidget* createParamWidget(QWidget* parent) const;

  /** The filter action shall set it's parameter from the widget's
      contents. It is allowed that the value is read by the action
      before this function is called. */
  virtual void applyParamWidgetValue(QWidget* paramWidget);

  /** The filter action shall set it's widget's contents from it's
      parameter. */
  virtual void setParamWidgetValue(QWidget* paramWidget) const;

  /** The filter action shall clear it's parameter widget's
      contents. */
  virtual void clearParamWidget(QWidget* paramWidget) const;
};

//=========================================================
//
// class FilterActionWithCommand
//
//=========================================================


/** Abstract base class for KMail's filter actions that need a command
    line as parameter, e.g. 'forward to'. Can create a QLineEdit
    (are there better widgets in the depths of the kdelibs?) as
    parameter widget. A subclass of this must provide at least
    implementations for the following methods:

    @li virtual FilterAction::ReturnCodes FilterAction::process
    @li static FilterAction::newAction

    The implementation of FilterAction::process should take the
    command line specified in mParameter, make all required
    modifications and stream the resulting command line into @p
    mProcess. Then you can start the command with @p mProcess.start().

    @short Abstract base class for filter actions with a command line as parameter.
    @author Marc Mutz <Marc@Mutz.com>, based upon work by Stefan Taferner <taferner@kde.org>
    @see FilterActionWithString FilterAction Filter KProcess

*/
class FilterActionWithUrl : public FilterAction
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
    FilterActionWithUrl(const char* aName, const QString &aLabel);
    ~FilterActionWithUrl();
  /** Determines whether this action is valid. But this is just a
      quick test. Eg., actions that have a mail address as parameter
      shouldn't try real address validation, but only check if the
      string representation is empty. */
  virtual bool isEmpty() const { return mParameter.trimmed().isEmpty(); }

  /** Creates a widget for setting the filter action parameter. Also
      sets the value of the widget. */
  virtual QWidget* createParamWidget(QWidget* parent) const;

  /** The filter action shall set it's parameter from the widget's
      contents. It is allowed that the value is read by the action
      before this function is called. */
  virtual void applyParamWidgetValue(QWidget* paramWidget);

  /** The filter action shall set it's widget's contents from it's
      parameter. */
  virtual void setParamWidgetValue(QWidget* paramWidget) const;

  /** The filter action shall clear it's parameter widget's
      contents. */
  virtual void clearParamWidget(QWidget* paramWidget) const;

  /** Read extra arguments from given string. */
  virtual void argsFromString(const QString &argsStr);

  /** Return extra arguments as string. Must not contain newlines. */
  virtual const QString argsAsString() const;

  /** Returns a translated string describing this filter for visualization
      purposes, e.g. in the filter log. */
  virtual const QString displayString() const;

protected:
  QString mParameter;
};


class FilterActionWithCommand : public FilterActionWithUrl
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
  FilterActionWithCommand(const char* aName, const QString &aLabel);

  /** Creates a widget for setting the filter action parameter. Also
      sets the value of the widget. */
  virtual QWidget* createParamWidget(QWidget* parent) const;

  /** The filter action shall set it's parameter from the widget's
      contents. It is allowed that the value is read by the action
      before this function is called. */
  virtual void applyParamWidgetValue(QWidget* paramWidget);

  /** The filter action shall set it's widget's contents from it's
      parameter. */
  virtual void setParamWidgetValue(QWidget* paramWidget) const;

  /** The filter action shall clear it's parameter widget's
      contents. */
  virtual void clearParamWidget(QWidget* paramWidget) const;

  /** Substitutes various placeholders for data from the message
      resp. for filenames containing that data. Currently, only %n is
      supported, where n in an integer >= 0. %n gets substituted for
      the name of a tempfile holding the n'th message part, with n=0
      meaning the body of the message. */
  virtual QString substituteCommandLineArgsFor( const KMime::Message::Ptr &aMsg, QList<KTemporaryFile*> & aTempFileList  ) const;

  virtual ReturnCode genericProcess( const Akonadi::Item &item, bool filtering ) const;
};



class FilterActionWithTest : public FilterAction
{
public:
  /** Initialize filter action with (english) name @p aName. This is
      the name under which this action is known in the config file. */
  FilterActionWithTest(const char* aName, const QString &aLabel);
    ~FilterActionWithTest();
  /** Determines whether this action is valid. But this is just a
      quick test. Eg., actions that have a mail address as parameter
      shouldn't try real address validation, but only check if the
      string representation is empty. */
  virtual bool isEmpty() const { return mParameter.trimmed().isEmpty(); }

  /** Creates a widget for setting the filter action parameter. Also
      sets the value of the widget. */
  virtual QWidget* createParamWidget(QWidget* parent) const;

  /** The filter action shall set it's parameter from the widget's
      contents. It is allowed that the value is read by the action
      before this function is called. */
  virtual void applyParamWidgetValue(QWidget* paramWidget);

  /** The filter action shall set it's widget's contents from it's
      parameter. */
  virtual void setParamWidgetValue(QWidget* paramWidget) const;

  /** The filter action shall clear it's parameter widget's
      contents. */
  virtual void clearParamWidget(QWidget* paramWidget) const;

  /** Read extra arguments from given string. */
  virtual void argsFromString(const QString &argsStr);

  /** Return extra arguments as string. Must not contain newlines. */
  virtual const QString argsAsString() const;

  /** Returns a translated string describing this filter for visualization
      purposes, e.g. in the filter log. */
  virtual const QString displayString() const;

protected:
  QString mParameter;
};


typedef FilterAction* (*FilterActionNewFunc)(void);


//-----------------------------------------------------------------------------
/** Auxiliary struct to FilterActionDict.  */
struct FilterActionDesc
{
  QString label, name;
  FilterActionNewFunc create;
};

/** Dictionary that contains a list of all registered filter actions
    with their creation functions. They are hard-coded into the
    constructor. If you want to add a new FilterAction, make
    sure you add the details of it in init, too.

    You will be able to find a description of a FilterAction by
    looking up either it's (english) name or it's (i18n) label:
    <pre>
    FilterActionDict dict;
    // get name of the action with label "move into folder":
    dict[i18n("move into folder")]->name; // == "transfer"
    // create one such action:
    FilterAction *action = dict["transfer"]->create();
    </pre>

    You can iterate over all known filter actions by using list.

    @short List of known FilterAction-types.
    @author Marc Mutz <Marc@Mutz.com>, based on work by Stefan Taferner <taferner@kde.org>
    @see FilterAction FilterActionDesc Filter

*/
class MAILCOMMON_EXPORT FilterActionDict : public QMultiHash<QString, FilterActionDesc*>
{
public:
  FilterActionDict();
  virtual ~FilterActionDict();

  /** Overloaded member function, provided for convenience. Thin
      wrapper around QDict::insert and QPtrList::insert.
      Inserts the resulting FilterActionDesc
      thrice: First with the name, then with the label as key into the
      QDict, then into the QPtrList. For that, it creates an
      instance of the action internally and deletes it again after
      querying it for name and label. */
  void insert(FilterActionNewFunc aNewFunc);

  /** Provides read-only access to a list of all known filter
      actions. */
  const QList<FilterActionDesc*>& list() const { return mList; }

protected:
  /** Populate the dictionary with all known  FilterAction
     types. Called automatically from the constructor. */
  virtual void init(void);

private:
  QList<FilterActionDesc*> mList;
};

}

#endif /*MAILCOMMON_FILTERACTION_H*/