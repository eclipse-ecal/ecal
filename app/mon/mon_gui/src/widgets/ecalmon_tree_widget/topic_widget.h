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

#include "ecalmon_tree_widget.h"

#include "widgets/models/topic_tree_model.h"
#include "widgets/models/topic_sort_filter_proxy_model.h"
#include "widgets/visualisation_widget/visualisation_window.h"

#include <QMap>

class TopicWidget : public EcalmonTreeWidget
{
  Q_OBJECT

public:
  TopicWidget(QWidget *parent = Q_NULLPTR);
  ~TopicWidget();

public slots:
  void openReflectionWindowForSelection();
  void resetLayout() override;
  void setParseTimeEnabled(bool enabled);

protected:
  void fillContextMenu(QMenu& menu, const QList<QAbstractTreeItem*>& selected_items) override;

private:
  bool parse_time_;
  TopicTreeModel* topic_tree_model_;
  TopicSortFilterProxyModel* topic_sort_filter_proxy_model_;

  QList<QRegExp> topic_exclude_regexp_list_;
  QList<QRegExp> topic_include_regexp_list_;

  QMap<QString, VisualisationWindow*> visualisation_windows_;

  void loadRegExpLists();
  void autoSizeColumns();
};
