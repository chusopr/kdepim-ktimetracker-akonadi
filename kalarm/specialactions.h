/*
 *  specialactions.h  -  widget to specify special alarm actions
 *  Program:  kalarm
 *  (C) 2004 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of this program with any edition of the Qt library by
 *  Trolltech AS, Norway (or with modified versions of Qt that use the same
 *  license as Qt), and distribute linked combinations including the two.
 *  You must obey the GNU General Public License in all respects for all of
 *  the code used other than Qt.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not obligated to
 *  do so. If you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef SPECIALACTIONS_H
#define SPECIALACTIONS_H

#include <kdialogbase.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

class KLineEdit;


class SpecialActionsButton : public QPushButton
{
		Q_OBJECT
	public:
		SpecialActionsButton(const QString& caption, QWidget* parent = 0, const char* name = 0);
		void           setActions(const QString& pre, const QString& post);
		const QString& preAction() const      { return mPreAction; }
		const QString& postAction() const     { return mPostAction; }
		virtual void   setReadOnly(bool ro)   { mReadOnly = ro; }
		virtual bool   isReadOnly() const     { return mReadOnly; }

	signals:
		void           selected();

	protected slots:
		void           slotButtonPressed();

	private:
		QString  mPreAction;
		QString  mPostAction;
		bool     mReadOnly;
};


// Pre- and post-alarm actions widget
class SpecialActions : public QGroupBox
{
		Q_OBJECT
	public:
		SpecialActions(QWidget* parent = 0, const char* name = 0);
		SpecialActions(const QString& frameLabel, QWidget* parent = 0, const char* name = 0);
		void         setActions(const QString& pre, const QString& post);
		QString      preAction() const;
		QString      postAction() const;
		void         setReadOnly(bool);
		bool         isReadOnly() const    { return mReadOnly; }

	private:
		void         init(const QString& frameLabel);
		KLineEdit*   mPreAction;
		KLineEdit*   mPostAction;
		bool         mReadOnly;
};


// Pre- and post-alarm actions dialogue displayed by the push button
class SpecialActionsDlg : public KDialogBase
{
		Q_OBJECT
	public:
		SpecialActionsDlg(const QString& preAction, const QString& postAction,
		                  const QString& caption, QWidget* parent = 0, const char* name = 0);
		QString      preAction() const     { return mActions->preAction(); }
		QString      postAction() const    { return mActions->postAction(); }
		void         setReadOnly(bool ro)  { mActions->setReadOnly(ro); }
		bool         isReadOnly() const    { return mActions->isReadOnly(); }

	protected:
		virtual void resizeEvent(QResizeEvent*);

	protected slots:
		virtual void slotOk();

	private:
		SpecialActions* mActions;
};

#endif // SPECIALACTIONS_H
