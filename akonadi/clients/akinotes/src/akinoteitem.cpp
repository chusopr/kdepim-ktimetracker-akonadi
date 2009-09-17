/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@kdab.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy 
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#include "akinoteitem.h"

#include <QByteArray>

#include <Akonadi/Item>
#include <KCal/Incidence>


#include <KDebug>

using namespace StickyNotes;

/* AkiNoteItem */

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

AkiNoteItem::AkiNoteItem(const Akonadi::Item &_item)
: BaseNoteItem(0), m_valid(false)
{
	if (_item.hasPayload<IncidencePtr>()) {
		m_item = _item.payload<IncidencePtr>();
		m_valid = true;
	}
}

AkiNoteItem::~AkiNoteItem(void)
{
}

bool
AkiNoteItem::isValid(void) const
{
	return (m_valid);
}

QVariant
AkiNoteItem::attribute(const QString &_name) const
{
	return (m_item->nonKDECustomProperty(_name.toAscii()));
}

QList<QString>
AkiNoteItem::attributeNames(void) const
{
	QList<QString> keys;

	foreach(QByteArray key, m_item->customProperties().keys())
		keys << key;

	return (keys);
}

QString
AkiNoteItem::content(void) const
{
	return (m_item->description());
}

QString
AkiNoteItem::subject(void) const
{
	return (m_item->summary());
}

bool
AkiNoteItem::applyAttribute(BaseNoteItem * const _sender,
    const QString &_name, const QVariant &_value)
{
	if (attribute(_name) == _value)
		return false;

	m_item->setNonKDECustomProperty(_name.toAscii(), _value.toString());

	return (true);
}

bool
AkiNoteItem::applyContent(BaseNoteItem * const _sender,
    const QString &_content)
{
	if (0 == content().compare(_content))
		return (false);

	m_item->setDescription(_content);

	return (true);
}

bool
AkiNoteItem::applySubject(BaseNoteItem * const _sender,
    const QString &_subject)
{
	if (0 == subject().compare(_subject))
		return (false);

	m_item->setSummary(_subject);

	return (true);
}

