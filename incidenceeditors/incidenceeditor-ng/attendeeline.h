/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef ATTENDEELINE_H
#define ATTENDEELINE_H

#include "attendeedata.h"

#include <libkdepim/multiplyingline.h>
#include <libkdepim/addresseelineedit.h>

#include <KComboBox>
#include <KGlobalSettings>

#include <QCheckBox>
#include <QKeyEvent>

namespace IncidenceEditorsNG {

class Attendee;

class AttendeeComboBox : public KComboBox
{
    Q_OBJECT
  public:
    explicit AttendeeComboBox( QWidget *parent );

  signals:
    void rightPressed();
    void leftPressed();

  protected:
    void keyPressEvent( QKeyEvent *ev );
};

class AttendeeCheckBox : public QCheckBox
{
    Q_OBJECT
  public:
    explicit AttendeeCheckBox( QWidget *parent );

  signals:
    void leftPressed();

  protected:
    void keyPressEvent( QKeyEvent *ev );
};

class AttendeeLineEdit : public KPIM::AddresseeLineEdit
{
    Q_OBJECT
  public:
    explicit AttendeeLineEdit( QWidget * parent );

  signals:
    void deleteMe();
    void leftPressed();
    void rightPressed();
    void upPressed();
    void downPressed();

  protected:
    void keyPressEvent( QKeyEvent *ev );
};

class AttendeeLine : public KPIM::MultiplyingLine
{
    Q_OBJECT
  public:
    AttendeeLine( QWidget* parent );
    virtual ~AttendeeLine(){}

    virtual void activate();
    virtual bool isActive() const;

    virtual bool isEmpty() const;
    virtual void clear();

    virtual bool isModified() const;
    virtual void clearModified();

    virtual KPIM::MultiplyingLineData::Ptr data() const;
    virtual void setData( const KPIM::MultiplyingLineData::Ptr &data );

    virtual void fixTabOrder( QWidget* previous );
    virtual QWidget* tabOut() const;

    virtual void moveCompletionPopup();
    virtual void setCompletionMode( KGlobalSettings::Completion );

    virtual int setColumnWidth( int w );

  signals:
    void changed();

  private slots:
    void slotTextChanged( const QString & );
    void slotEditingFinished();
  private:
    void dataFromFields();
    void fieldsFromData();

    AttendeeComboBox *mRoleCombo;
    AttendeeComboBox *mStateCombo;
    AttendeeCheckBox *mResponseCheck;
    AttendeeLineEdit *mEdit;
    QSharedPointer<AttendeeData> mData;
    QString mUid;
    bool mModified;
    

};
  
}


#endif // ATTENDEELINE_H