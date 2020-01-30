#pragma once

#include "Plugin/PluginManager.h"
#include <QAbstractItemModel>

class PluginTableModel : public QAbstractItemModel
{
  Q_OBJECT
public:

  PluginTableModel(QObject *parent);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  Qt::ItemFlags flags(const QModelIndex & index) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  QModelIndex parent(const QModelIndex &index) const;

  void rediscover();

private:
  enum class Columns : int
  {
    NAME,
    VERSION,
    AUTHOR,
    IID,
    PRIORITY,
    PATH,
    TOPICS,

    COLUMN_COUNT
  };

  const QMap<Columns, QString> header_data
  {
    { Columns::NAME,     "Name" },
    { Columns::VERSION,  "Version" },
    { Columns::AUTHOR,   "Author" },
    { Columns::IID,      "IID" },
    { Columns::PRIORITY, "Priority" },
    { Columns::PATH,     "Path" },
    { Columns::TOPICS,   "Topics" }
  };

};
