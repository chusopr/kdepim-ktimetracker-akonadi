/*
 * Copyright (c) 2011 Montel Laurent <montel@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef KMSUBJECTLINEEDIT_H
#define KMSUBJECTLINEEDIT_H
#include "messagecomposer_export.h"

#include <KTextEdit>

class QContextMenuEvent;
class QMimeData;
namespace Message {

class MESSAGECOMPOSER_EXPORT KMSubjectLineEdit : public KTextEdit
{
  Q_OBJECT

public:
  /**
   * Constructs a KMSubjectLineEdit object.
   * @param parent of widget
   * @param configFile config file name for spell checking
   */
  explicit KMSubjectLineEdit(QWidget* parent, const QString& configFile);
  /**
   * Destructor
   */
  ~KMSubjectLineEdit();
  
protected:
  void createHighlighter();

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;
  virtual void keyPressEvent(QKeyEvent*);
  virtual void insertFromMimeData ( const QMimeData * source );

Q_SIGNALS:
  /**
   * Emitted when the user uses the up arrow in the first line. The application
   * should then put the focus on the widget above the text edit.
   */
  void focusUp();
  
  void focusDown();
private:
  class Private;
  Private *const d;

};
}
#endif /* KMSUBJECTLINEEDIT_H */

