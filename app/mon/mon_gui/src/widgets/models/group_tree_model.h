/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#pragma once

#include <CustomQt/QAbstractTreeModel.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "group_tree_item.h"

#include <list>
#include <QVector>
#include <QPair>
#include <QFont>

class GroupTreeModel : public QAbstractTreeModel
{
  Q_OBJECT

public:
  GroupTreeModel(const QVector<int>& group_by_columns, QObject *parent = nullptr);
  ~GroupTreeModel();

  void insertItemIntoGroups(QAbstractTreeItem* item);
  void removeItemFromGroups(QAbstractTreeItem* item, bool remove_empty_groups = true);

  void setGroupByColumns(const QVector<int>& group_by_columns);

  void setGroupColumnHeader(const QVariant& header);
  QVariant groupColumnHeader() const;

  QVector<int> groupByColumns() const;

  QVector<QPair<int, QString>> getTreeItemColumnNameMapping() const;

  virtual void monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb) = 0;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

  virtual int groupColumn() const = 0;

private:
  //std::list<std::pair<QVariant, GroupTreeItem*>> group_map_;                                    /*< group_identifier -> TreeItem mapping*/

  struct cmp
  {
    bool operator()(const QVariant& lhs, const QVariant& rhs) const
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      return QVariant::compare(lhs, rhs) == QPartialOrdering::Less;
#else

  // deactivate warning about deprecated QVariant::compare
  #ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning (disable : 4996)
  #elif __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  #endif

      return lhs < rhs;

  #ifdef _MSC_VER
    #pragma warning(pop)
  #elif __GNUC__
    #pragma GCC diagnostic pop
  #endif

#endif
    }
  };
  std::map<QVariant, GroupTreeItem*, cmp> group_map_;


  QList<QAbstractTreeItem*> items_list_;

  QVariant group_column_header_;

  QVector<int> group_by_columns_;

  QFont standardFont;
};
