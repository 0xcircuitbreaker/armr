// Copyright (c) 2011-2015 The Bitcoin Core developers
// Copyright (c) 2018 The ARMR Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bantablemodel.h"

#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"

#include "sync.h"
#include "net.h"
#include <QDateTime>
#include <QDebug>
#include <QList>

bool BannedNodeLessThan::operator()(const CCombinedBan &left, const CCombinedBan &right) const
{
  const CCombinedBan *pLeft = &left;
  const CCombinedBan *pRight = &right;

  if (order == Qt::DescendingOrder)
    std::swap(pLeft, pRight);

  switch (column)
  {
  case BanTableModel::Address:
    return pLeft->subnet.ToString().compare(pRight->subnet.ToString()) < 0;
  case BanTableModel::Bantime:
    return pLeft->banEntry < pRight->banEntry;
  }

  return false;
}

class BanTablePriv
{
public:
  /** Local cache of peer information */
  QList<CCombinedBan> cachedBanlist;
  /** Column to sort nodes by */
  int sortColumn;
  /** Order (ascending or descending) to sort nodes by */
  Qt::SortOrder sortOrder;
  /** Pull a full list of banned nodes from CNode into our cache */
  void refreshBanlist()
  {
    std::map<CNetAddr, int64_t> banMap = CNode::GetBanned();

    cachedBanlist.clear();
#if QT_VERSION >= 0x040700
    cachedBanlist.reserve(banMap.size());
#endif
    for (std::map<CNetAddr, int64_t>::iterator it = banMap.begin(); it != banMap.end(); it++)
    {
      CCombinedBan banEntry;
      banEntry.subnet = (*it).first;
      banEntry.banEntry = (*it).second;
      cachedBanlist.append(banEntry);
    }

    if (sortColumn >= 0)
      // sort cachedBanlist (use stable sort to prevent rows jumping around unneceesarily)
      qStableSort(cachedBanlist.begin(), cachedBanlist.end(), BannedNodeLessThan(sortColumn, sortOrder));
  }

  int size() const
  {
    return cachedBanlist.size();
  }

  CCombinedBan *index(int idx)
  {
    if (idx >= 0 && idx < cachedBanlist.size())
      return &cachedBanlist[idx];

    return 0;
  }
};

BanTableModel::BanTableModel(ClientModel *parent) : QAbstractTableModel(parent),
                                                    clientModel(parent)
{
  columns << tr("IP/Netmask") << tr("Banned Until");
  priv = new BanTablePriv();
  // don't sort
  priv->sortColumn = -1;

  refresh();
}

int BanTableModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return priv->size();
}

int BanTableModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return columns.length();
}

QVariant BanTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  CCombinedBan *rec = static_cast<CCombinedBan *>(index.internalPointer());

  if (role == Qt::DisplayRole)
  {
    switch (index.column())
    {
    case Address:
      return QString::fromStdString(rec->subnet.ToString());
    case Bantime:
      QDateTime date = QDateTime::fromMSecsSinceEpoch(0);
      date = date.addSecs((qint64)rec->banEntry);
      return date.toString(Qt::SystemLocaleLongDate);
    }
  }

  return QVariant();
}

QVariant BanTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal)
  {
    if (role == Qt::DisplayRole && section < columns.size())
    {
      return columns[section];
    }
  }
  return QVariant();
}

Qt::ItemFlags BanTableModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  Qt::ItemFlags retval = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return retval;
}

QModelIndex BanTableModel::index(int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  CCombinedBan *data = priv->index(row);

  if (data)
    return createIndex(row, column, data);
  return QModelIndex();
}

void BanTableModel::refresh()
{
  Q_EMIT layoutAboutToBeChanged();
  priv->refreshBanlist();
  Q_EMIT layoutChanged();
}

void BanTableModel::sort(int column, Qt::SortOrder order)
{
  priv->sortColumn = column;
  priv->sortOrder = order;
  refresh();
}

bool BanTableModel::shouldShow()
{
  if (priv->size() > 0)
    return true;
  return false;
}
