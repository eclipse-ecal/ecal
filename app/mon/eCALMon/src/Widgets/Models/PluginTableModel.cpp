#include "PluginTableModel.h"

#include <QMessageBox>
#include <QDebug>

PluginTableModel::PluginTableModel(QObject *parent) :
  QAbstractItemModel(parent)
{}

int PluginTableModel::rowCount(const QModelIndex &parent) const// override
{
  if (parent.isValid())
  {
    return 0;
  }

  return PluginLoader::getInstance()->getAvailableIIDs().count();
}

int PluginTableModel::columnCount(const QModelIndex &parent) const// override
{
  if (parent.isValid())
  {
    return 0;
  }
  return static_cast<int>(Columns::COLUMN_COUNT);
}

static QString GetTopicDisplayString(const PluginWrapper::MetaData::Topic& topic)
{
  auto name = (!topic.name.isEmpty()) ? topic.name : QString("*");
  auto type = (!topic.type.isEmpty()) ? topic.type : QString("*");
  auto format = (!topic.format.isEmpty()) ? topic.format : QString("*");

  return name + "(" + format + ":" + type + ")";
}

QVariant PluginTableModel::data(const QModelIndex &index, int role) const// override
{
  if (index.isValid())
  {
    auto iid = PluginLoader::getInstance()->getAvailableIIDs().at(index.row());
    PluginWrapper& plugin = PluginLoader::getInstance()->getPluginByIID(iid);

    Columns column = static_cast<Columns>(index.column());

    if (role == Qt::ItemDataRole::DisplayRole)
    {
      switch (column)
      {
      case Columns::NAME:
        return plugin.getMetaData().name;
      case Columns::VERSION:
        return plugin.getMetaData().version;
      case Columns::AUTHOR:
        return plugin.getMetaData().author;
      case Columns::IID:
        return plugin.getIID();
      case Columns::PRIORITY:
        return plugin.getMetaData().priority;
      case Columns::PATH:
        return plugin.getPath();
      case Columns::TOPICS:
      {
        QString topics_str;
        for (const auto& topic : plugin.getMetaData().topics)
          topics_str += "\"" + GetTopicDisplayString(topic) + "\"  ";
        return topics_str;
      }
      default:
        return QVariant();
      }
    }
    if (role == Qt::ItemDataRole::CheckStateRole)
    {
      if (column == Columns::NAME)
        return plugin.isLoaded() ? Qt::Checked : Qt::Unchecked;
    }
  }

  return QVariant();
}

bool PluginTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (index.isValid())
  {
    auto iid = PluginLoader::getInstance()->getAvailableIIDs().at(index.row());
    PluginWrapper& plugin = PluginLoader::getInstance()->getPluginByIID(iid);

    Columns column = static_cast<Columns>(index.column());

    if (role == Qt::CheckStateRole)
    {
      if (column == Columns::NAME)
      {
        if (value == Qt::Checked)
        {
          if (!plugin.load())
            QMessageBox::warning(nullptr, tr("eCAL Monitor"), tr("Unable to activate plugin. For more information see log output."), QMessageBox::Ok, QMessageBox::Ok);
        }
        else
        {
          if (!plugin.unload())
            QMessageBox::information(nullptr, tr("eCAL Monitor"), tr("The plugin cannot be deactivated because it is still in use. Please close all associated visualistion windows and try again."), QMessageBox::Ok, QMessageBox::Ok);
        }
        return true;
      }
    }
  }
  return false;
}

Qt::ItemFlags PluginTableModel::flags(const QModelIndex & index) const
{
  if (index.isValid())
  {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsUserCheckable;
  }
  return QAbstractItemModel::flags(index);
}

QModelIndex PluginTableModel::index(int row, int column, const QModelIndex & /*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QVariant PluginTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Orientation::Horizontal) && (role == Qt::ItemDataRole::DisplayRole))
  {
    return header_data[(Columns)section];
  }
  return QVariant();
}

QModelIndex PluginTableModel::parent(const QModelIndex & /*index*/) const
{
  return QModelIndex();
}

void PluginTableModel::rediscover()
{
  beginResetModel();
  PluginLoader::getInstance()->discover();
  endResetModel();
}
