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

#ifndef COMPOSERIMAGEDIALOG_H
#define COMPOSERIMAGEDIALOG_H

#include <KDialog>
#include <QWebElement>

namespace ComposerEditorNG
{
class ComposerImageDialogPrivate;
class ComposerImageDialog : public KDialog
{
    Q_OBJECT
public:
    explicit ComposerImageDialog(QWidget *parent);
    explicit ComposerImageDialog(const QWebElement& element, QWidget *parent);
    ~ComposerImageDialog();

    /**
     * @brief html
     * @return string which represents html code
     */
    QString html() const;

private Q_SLOTS:
    void slotOkClicked();

private:
    friend class ComposerImageDialogPrivate;
    ComposerImageDialogPrivate * const d;
};
}

#endif // COMPOSERIMAGEDIALOG_H
