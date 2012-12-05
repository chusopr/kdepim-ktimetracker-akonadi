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

#ifndef MANAGELINK_H
#define MANAGELINK_H
#include <KDialog>

#include <QWebElement>

class KLineEdit;

namespace ComposerEditorNG
{
class ManageLink : public KDialog
{
    Q_OBJECT
public:
    explicit ManageLink(QWidget *parent);
    explicit ManageLink(const QWebElement& element, QWidget *parent);
    ~ManageLink();

    void setLinkText(const QString& link);
    QString linkText() const;

    void setLinkLocation(const QString& location);
    QString linkLocation() const;

public Q_SLOTS:
    void slotOkClicked();

private:
    void initialize();
    QWebElement mWebElement;
    KLineEdit *mLinkText;
    KLineEdit *mLinkLocation;
};
}

#endif // MANAGELINK_H
