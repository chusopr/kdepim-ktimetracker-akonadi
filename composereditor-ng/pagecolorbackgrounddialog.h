/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#ifndef PAGECOLORBACKGROUNDDIALOG_H
#define PAGECOLORBACKGROUNDDIALOG_H

#include <KDialog>
class QWebFrame;

namespace ComposerEditorNG
{
class PageColorBackgroundWidget;

class PageColorBackgroundDialog : public KDialog
{
    Q_OBJECT
public:
    explicit PageColorBackgroundDialog(QWebFrame *frame, QWidget *parent = 0);
    ~PageColorBackgroundDialog();

private Q_SLOTS:
    void slotOkClicked();

private:
    void initSettings();

private:
    PageColorBackgroundWidget *mPageColorWidget;
    QWebFrame *mFrame;
};
}

#endif // PAGECOLORBACKGROUNDDIALOG_H
