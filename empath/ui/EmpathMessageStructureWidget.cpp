/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathMessageStructureWidget.h"
#endif

// Qt includes
#include <qfile.h>
#include <qdatastream.h>
#include <qmessagebox.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kfiledialog.h>

// Local includes
#include "EmpathMessageStructureWidget.h"
#include "EmpathMessageStructureItem.h"
#include "EmpathUIUtils.h"

EmpathMessageStructureWidget::EmpathMessageStructureWidget
    (QWidget * parent, const char * name)
    :    QListView(parent, name)
{
    setCaption(i18n("Message Structure"));
    
    addColumn(i18n("Type"));
    addColumn(i18n("Subtype"));
    addColumn(i18n("Size"));
    
    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    setSorting(-1); // Don't sort this.

    QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
            this, SLOT(s_currentChanged(QListViewItem *)));
    
    QObject::connect(
        this,
        SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
        this,
        SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int)));
    
    popup_.insertItem(empathIcon("menu-save"),
            i18n("Save &As"),
            this, SLOT(s_saveAs()));
    
    popup_.insertItem(
            i18n("Open with..."),
            this, SLOT(s_openWith()));
    
}

EmpathMessageStructureWidget::~EmpathMessageStructureWidget()
{
    // Empty.
}

    void
EmpathMessageStructureWidget::setMessage(RMM::RBodyPart p)
{
    clear();
    
    EmpathMessageStructureItem * i = new EmpathMessageStructureItem(this, p);
    
    QList<RMM::RBodyPart> body(p.body());
    QListIterator<RMM::RBodyPart> it(body);
    
    for (; it.current(); ++it) {
        
        EmpathMessageStructureItem * j =
            new EmpathMessageStructureItem(i, *(it.current()));

        _addChildren(*(it.current()), j);
    }
}

    void
EmpathMessageStructureWidget::_addChildren(RMM::RBodyPart p, QListViewItem *i)
{
    QList<RMM::RBodyPart> body(p.body());
    QListIterator<RMM::RBodyPart> it(body);
    
    for (; it.current(); ++it) {
    
        EmpathMessageStructureItem * j =
            new EmpathMessageStructureItem((EmpathMessageStructureItem *)i, p);

        _addChildren(*(it.current()), j);
    }
}

    void
EmpathMessageStructureWidget::s_currentChanged(QListViewItem * item)
{
    empathDebug("s_currentChanged() called");
    if (item->depth() == 0) return;
    EmpathMessageStructureItem * i = (EmpathMessageStructureItem *)item;
    emit(partChanged(i->part()));
}

    void
EmpathMessageStructureWidget::s_rightButtonPressed(
    QListViewItem * item, const QPoint &, int)
{
    setSelected(item, true);
    popup_.exec(QCursor::pos());
}

    void
EmpathMessageStructureWidget::s_saveAs()
{
    QString saveFilePath =
        KFileDialog::getSaveFileName(
            QString::null, QString::null,
            this, i18n("Empath: Save Message").latin1());
    
    if (saveFilePath.isEmpty()) {
        empathDebug("No filename given");
        return;
    }
    
    QFile f(saveFilePath);
    if (!f.open(IO_WriteOnly)) {
        // Warn user file cannot be opened.
        empathDebug("Couldn't open file for writing");
        QMessageBox::information(this, "Empath",
            i18n("Sorry I can't write to that file. "
                "Please try another filename."), i18n("OK"));
        return;
    }
    empathDebug("Opened " + saveFilePath + " OK");
    
    QDataStream d(&f);
    
    QListViewItem * i(currentItem());
    
    if (!i) return;
    
    QCString s = ((EmpathMessageStructureItem *)i)->part().asString();
    
    d.writeRawBytes(s, s.length());

    f.close();
    
}

    void
EmpathMessageStructureWidget::s_openWith()
{
}

// vim:ts=4:sw=4:tw=78
