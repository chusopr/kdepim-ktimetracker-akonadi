/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "threadmodel.h"

#include <QtGui/QItemSelectionRange>

#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>
#include <KMime/Message>
#include <map>

struct ItemLessThanComparator
{
  ThreadGrouperModelPrivate const * m_grouper;
  ItemLessThanComparator(const ThreadGrouperModelPrivate * grouper);
  ItemLessThanComparator(const ItemLessThanComparator &other);
  ItemLessThanComparator &operator=(const ItemLessThanComparator &other);

  bool operator()(const Akonadi::Item &left, const Akonadi::Item &right) const;
};

typedef std::map<Akonadi::Item, QByteArray, ItemLessThanComparator> MessageMap;

class ThreadGrouperModelPrivate
{
public:
  ThreadGrouperModelPrivate(ThreadGrouperModel *qq)
    : q_ptr(qq), m_messageMap(this)
  {
  }
  Q_DECLARE_PUBLIC(ThreadGrouperModel)
  ThreadGrouperModel * const q_ptr;

  Akonadi::Item getThreadItem(const Akonadi::Item &item) const;

  Akonadi::Item threadRoot(const QModelIndex &index) const;

  void populateThreadGrouperModel() const;

  mutable QHash<QByteArray, QSet<QByteArray> > m_threads;
  mutable MessageMap m_messageMap;
  mutable QHash<QByteArray, Akonadi::Item> m_threadItems;
};

static QHash<QByteArray, QSet<QByteArray> >::const_iterator findValue(const QHash<QByteArray, QSet<QByteArray> > &container, const QByteArray &target)
{
  QHash<QByteArray, QSet<QByteArray> >::const_iterator it = container.constBegin();
  const QHash<QByteArray, QSet<QByteArray> >::const_iterator end = container.constEnd();

  for ( ; it != end; ++it)
  {
    if (it.value().contains(target)) {
      return it;
    }
  }
  return end;
}

Akonadi::Item ThreadGrouperModelPrivate::getThreadItem(const Akonadi::Item& item) const
{
  Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
  const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
  const QByteArray identifier = message->messageID()->identifier();

  if (m_threadItems.contains(identifier))
    return m_threadItems.value(identifier);

  QHash<QByteArray, QSet<QByteArray> >::const_iterator it = findValue(m_threads, identifier);
  Q_ASSERT(it != m_threads.constEnd());
  Q_ASSERT(m_threadItems.value(it.key()).isValid());
  return m_threadItems.value(it.key());
}

ItemLessThanComparator::ItemLessThanComparator(const ThreadGrouperModelPrivate * grouper)
  : m_grouper(grouper)
{

}

ItemLessThanComparator::ItemLessThanComparator(const ItemLessThanComparator &other)
{
  m_grouper = other.m_grouper;
}

ItemLessThanComparator &ItemLessThanComparator::operator=(const ItemLessThanComparator &other)
{
  m_grouper = other.m_grouper;
  return *this;
}

bool ItemLessThanComparator::operator()(const Akonadi::Item &leftItem, const Akonadi::Item &rightItem) const
{
  Q_ASSERT(leftItem.isValid());
  Q_ASSERT(rightItem.isValid());

  const Akonadi::Item leftThreadRootItem = m_grouper->getThreadItem(leftItem);
  const Akonadi::Item rightThreadRootItem = m_grouper->getThreadItem(rightItem);

  Q_ASSERT(rightThreadRootItem.isValid());
  Q_ASSERT(leftThreadRootItem.isValid());

  if (leftThreadRootItem != rightThreadRootItem) {
    Q_ASSERT(leftThreadRootItem.hasPayload<KMime::Message::Ptr>());
    Q_ASSERT(rightThreadRootItem.hasPayload<KMime::Message::Ptr>());

    const KMime::Message::Ptr leftThreadRootMessage = leftThreadRootItem.payload<KMime::Message::Ptr>();
    const KMime::Message::Ptr rightThreadRootMessage = rightThreadRootItem.payload<KMime::Message::Ptr>();

    // FIXME : Here we order by the date of the first message in the thread.
    // Needs to be configurable to order by the thread with the most recent reply.

    const KDateTime leftThreadRootDateTime = leftThreadRootMessage->date()->dateTime();
    const KDateTime rightThreadRootDateTime = rightThreadRootMessage->date()->dateTime();
    if (leftThreadRootDateTime != rightThreadRootDateTime) {
      return leftThreadRootDateTime > rightThreadRootDateTime;
    }
    return leftThreadRootItem.id() < rightThreadRootItem.id();
  }

  if (leftThreadRootItem == leftItem)
    return true;

  if (rightThreadRootItem == rightItem)
    return false;

  Q_ASSERT(leftItem.hasPayload<KMime::Message::Ptr>());
  Q_ASSERT(rightItem.hasPayload<KMime::Message::Ptr>());

  const KMime::Message::Ptr leftMessage = leftItem.payload<KMime::Message::Ptr>();
  const KMime::Message::Ptr rightMessage = rightItem.payload<KMime::Message::Ptr>();

  const KDateTime leftDateTime = leftMessage->date()->dateTime();
  const KDateTime rightDateTime = rightMessage->date()->dateTime();

  // Messages in the same thread are ordered most recent last.
  if (leftDateTime != rightDateTime) {
    return leftDateTime < rightDateTime;
  }

  return leftItem.id() < rightItem.id();
}

static QVector<QByteArray> getRemovableItems(const QHash<QByteArray, QSet<QByteArray> > &container, const QByteArray &value)
{
  QVector<QByteArray> items;

  foreach(const QByteArray &ba, container[value]) {
    items.push_back(ba);
    items += getRemovableItems(container, ba);
  }

  return items;
}

void ThreadGrouperModelPrivate::populateThreadGrouperModel() const
{
  Q_Q(const ThreadGrouperModel);
  m_threads.clear();
  m_messageMap.clear();

  QHash<QByteArray, QSet<QByteArray> > pendingThreads;

  if (!q->sourceModel())
    return;
  const int rowCount = q->sourceModel()->rowCount();

  for (int row = 0; row < rowCount; ++row) {
    const QModelIndex idx = q->sourceModel()->index(row, 0);
    Q_ASSERT(idx.isValid());
    const Akonadi::Item item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    Q_ASSERT(item.isValid());
    Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
    const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    const QByteArray identifier = message->messageID()->identifier();
    if (!message->inReplyTo()->isEmpty()) {
      QByteArray _inReplyTo = message->inReplyTo()->as7BitString(false);
      const QByteArray inReplyTo = _inReplyTo.mid(1, _inReplyTo.size() -2 );

      const QHash<QByteArray, QSet<QByteArray> >::const_iterator it = findValue(m_threads, inReplyTo);
      const QHash<QByteArray, QSet<QByteArray> >::const_iterator end = m_threads.constEnd();
      if (it == end) {
        pendingThreads[inReplyTo].insert(identifier);
        Q_ASSERT(item.isValid());
        m_threadItems[identifier] = item;
        m_messageMap[item] = identifier;
        continue;
      }
      m_threadItems.remove(identifier);
      m_threads[it.key()].insert(identifier);
    } else {
      foreach(const QByteArray &ba, getRemovableItems(pendingThreads, identifier)) {
        m_threadItems.remove(ba);
        pendingThreads.remove(ba);
        m_threads[identifier].insert(ba);
      }
      m_threadItems[identifier] = item;
      m_messageMap[item] = identifier;
    }
  }
}

ThreadGrouperModel::ThreadGrouperModel(QObject* parent)
  : QSortFilterProxyModel(parent), d_ptr(new ThreadGrouperModelPrivate(this))
{
  setDynamicSortFilter(true);
  sort(0, Qt::AscendingOrder);
}

ThreadGrouperModel::~ThreadGrouperModel()
{
  delete d_ptr;
}

Akonadi::Item ThreadGrouperModelPrivate::threadRoot(const QModelIndex &index) const
{
  const Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  Q_ASSERT(item.isValid());
  return getThreadItem(item);
}

QVariant ThreadGrouperModel::data(const QModelIndex& index, int role) const
{
  Q_D(const ThreadGrouperModel);
  if (role == ThreadIdRole) {
    return d->threadRoot(index).id();
  }
  if (role == Qt::DisplayRole) {
    Akonadi::Item item = QSortFilterProxyModel::data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    return  QSortFilterProxyModel::data(index, role).toString() + message->subject()->asUnicodeString() + " - " + message->date()->asUnicodeString();
  }

  return QSortFilterProxyModel::data(index, role);
}

void ThreadGrouperModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  Q_D(ThreadGrouperModel);
  d->populateThreadGrouperModel();
  QSortFilterProxyModel::setSourceModel(sourceModel);

  disconnect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
      this, SIGNAL(_q_sourceRowsAboutToBeInserted(QModelIndex,int,int)));

  disconnect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
      this, SLOT(_q_sourceRowsInserted(QModelIndex,int,int)));

  disconnect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
      this, SLOT(_q_sourceRowsAboutToBeRemoved(QModelIndex,int,int)));

  disconnect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
      this, SLOT(_q_sourceRowsRemoved(QModelIndex,int,int)));

  disconnect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
      this, SLOT(_q_sourceLayoutAboutToBeChanged()));

  disconnect(sourceModel, SIGNAL(layoutChanged()),
      this, SLOT(_q_sourceLayoutChanged()));

  connect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
      this, SLOT(_q_sourceAboutToBeReset()));

  connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
      this, SLOT(populateThreadGrouperModel()));

  connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
      this, SLOT(_q_sourceReset()));

  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
      this, SLOT(_q_sourceAboutToBeReset()));

  connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
      this, SLOT(populateThreadGrouperModel()));

  connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
      this, SLOT(_q_sourceReset()));

  connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
      this, SLOT(_q_sourceAboutToBeReset()));

  connect(sourceModel, SIGNAL(layoutChanged()),
      this, SLOT(populateThreadGrouperModel()));

  connect(sourceModel, SIGNAL(layoutChanged()),
      this, SLOT(_q_sourceReset()));

}

bool ThreadGrouperModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  Q_D(const ThreadGrouperModel);
  static ItemLessThanComparator lt(d);
  const Akonadi::Item leftItem = left.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  return lt(leftItem, rightItem);
}

struct ThreadModelNode {
  ThreadModelNode(const QModelIndex &top, const QModelIndex &bottom)
    : range(top, bottom)
  {

  }
  QItemSelectionRange range;
};

class ThreadModelPrivate
{
  ThreadModelPrivate(QAbstractItemModel *emailModel, ThreadModel *qq)
    : q_ptr(qq), m_emailModel(emailModel)
  {
    populateThreadModel();
  }
  Q_DECLARE_PUBLIC(ThreadModel)
  ThreadModel * const q_ptr;

  void populateThreadModel();

  QAbstractItemModel *m_emailModel;

  QVector<ThreadModelNode*> m_threads;

};

void ThreadModelPrivate::populateThreadModel()
{
  Q_Q(ThreadModel);
  q->beginResetModel();
  m_threads.clear();
  const int rowCount = m_emailModel->rowCount();
  const QModelIndex firstIdx = m_emailModel->index(0, 0);
  Akonadi::Item::Id currentThreadId = firstIdx.data(ThreadGrouperModel::ThreadIdRole).toLongLong();
  int startRow = 0;
  int row = 1;
  static const int column = 0;
  for ( ; row < rowCount; ++row) {
    const QModelIndex idx = m_emailModel->index(row, column);
    Q_ASSERT(idx.isValid());
    const Akonadi::Item::Id threadRoot = idx.data(ThreadGrouperModel::ThreadIdRole).toLongLong();
    if (threadRoot != currentThreadId)
    {
      const QModelIndex top = m_emailModel->index(startRow, column);
      const QModelIndex bottom = m_emailModel->index(row - 1, column);
      Q_ASSERT(top.isValid());
      Q_ASSERT(bottom.isValid());
      m_threads.push_back(new ThreadModelNode(top, bottom));
      startRow = row;
    }
    currentThreadId = threadRoot;
  }
  const QModelIndex idx = m_emailModel->index(row, column);
  const Akonadi::Item::Id threadRoot = idx.data(ThreadGrouperModel::ThreadIdRole).toLongLong();
  if (threadRoot != currentThreadId)
  {
    const QModelIndex top = m_emailModel->index(startRow, column);
    const QModelIndex bottom = m_emailModel->index(row - 1, column);
    Q_ASSERT(top.isValid());
    Q_ASSERT(bottom.isValid());
    m_threads.push_back(new ThreadModelNode(top, bottom));
    startRow = row;
  }
  q->endResetModel();
}

ThreadModel::ThreadModel(QAbstractItemModel *emailModel, QObject* parent)
  : QAbstractListModel(parent), d_ptr(new ThreadModelPrivate(emailModel, this))
{
  connect(emailModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
      this, SLOT(populateThreadModel()));

  connect(emailModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
      this, SLOT(populateThreadModel()));

  connect(emailModel, SIGNAL(layoutChanged()),
      this, SLOT(populateThreadModel()));

  connect(emailModel, SIGNAL(modelReset()),
      this, SLOT(populateThreadModel()));

  setRoleNames(emailModel->roleNames());

}

ThreadModel::~ThreadModel()
{
  qDeleteAll(d_func()->m_threads);
  delete d_ptr;
}

QVariant ThreadModel::data(const QModelIndex& index, int role) const
{
  Q_D(const ThreadModel);
  const int idx = index.row();
  if (idx < d->m_threads.size()) {
    ThreadModelNode *node = d->m_threads.at(idx);
    const QModelIndex firstMailIndex = node->range.topLeft();
    Q_ASSERT(firstMailIndex.isValid());

    if (role == ThreadRangeStartRole)
      return node->range.top();
    if (role == ThreadRangeEndRole)
      return node->range.bottom();

    if (role == Qt::DisplayRole) {
      QString displ = firstMailIndex.data(role).toString();
      return displ + "(" + QString::number( node->range.height() ) + ")";
    }
    return firstMailIndex.data(role);
  }

  return QVariant();
}

int ThreadModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  Q_D(const ThreadModel);
  return d->m_threads.size();
}

class ThreadSelectionModelPrivate
{
  ThreadSelectionModelPrivate(ThreadSelectionModel *qq, QItemSelectionModel *selectionModel)
    : q_ptr(qq), m_selectionModel(selectionModel)
  {

  }
  Q_DECLARE_PUBLIC(ThreadSelectionModel)
  ThreadSelectionModel * const q_ptr;

  QItemSelectionModel * const m_selectionModel;

};

ThreadSelectionModel::ThreadSelectionModel(QAbstractItemModel* model, QItemSelectionModel* selectionModel, QObject *parent)
  : QItemSelectionModel(model, parent),
    d_ptr(new ThreadSelectionModelPrivate(this, selectionModel))
{

}

void ThreadSelectionModel::select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command)
{
  select(QItemSelection(index, index), command);
}

void ThreadSelectionModel::select(const QItemSelection& selection, QItemSelectionModel::SelectionFlags command)
{
  Q_D(ThreadSelectionModel);
  QItemSelectionModel::select(selection, command);
  QItemSelection thread;
  foreach(const QItemSelectionRange &range, selection) {
    for (int row = range.top(); row <= range.bottom(); ++row) {
      static const int column = 0;
      const QModelIndex idx = model()->index(row, column);
      const int threadStartRow = idx.data(ThreadModel::ThreadRangeStartRole).toInt();
      const int threadEndRow = idx.data(ThreadModel::ThreadRangeEndRole).toInt();
      const QModelIndex threadStart = d->m_selectionModel->model()->index(threadStartRow, column);
      const QModelIndex threadEnd = d->m_selectionModel->model()->index(threadEndRow, column);
      Q_ASSERT(threadStart.isValid());
      Q_ASSERT(threadEnd.isValid());
      thread.select(threadStart, threadEnd);
    }
  }
  kDebug() << thread;
  d->m_selectionModel->select(thread, ClearAndSelect);
}


#include "threadmodel.moc"
