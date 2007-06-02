/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.


   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.


   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef SCANPROGRESSPAGE_H
#define SCANPROGRESSPAGE_H

#include <QtGui/QWizardPage>
#include "engineui_export.h"

class QProgressBar;
class ScanProgressPagePrivate;
class ENGINEUI_EXPORT ScanProgressPage : public QWizardPage
{
Q_OBJECT
public:
    ScanProgressPage( QWidget * parent = 0 );
    QString statusString() const;
    void setStatusString(const QString &);
    QProgressBar *progressBar();
    void setProgress(int progress);
    int progress();
    virtual void cleanupPage();
private:
    ScanProgressPagePrivate *const d;
};
#endif

