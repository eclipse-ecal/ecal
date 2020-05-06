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

#include <QWidget>
#include "ui_channel_widget.h"

#include <QList>

#include <CustomQt/QStableSortFilterProxyModel.h>
#include <CustomQt/QCheckboxHeaderView.h>

#include <string>

#include "ecal_play_state.h"
#include "q_ecal_play.h"

#include "widgets/models/channel_tree_model.h"
#include "widgets/models/channel_tree_item.h"

class ChannelWidget : public QWidget
{
  Q_OBJECT

public:
  ChannelWidget(QWidget *parent = Q_NULLPTR);
  ~ChannelWidget();

public slots:
  void resetLayout();

private slots:
  void measurementLoaded(const QString& path);
  void measurementClosed();
  void publishersInitStateChanged(bool publishers_initialized);

  void channelMappingChanged(const std::map<std::string, std::string>& channel_mapping);

  // Internal
  void headerClicked(int column, bool checked);
  void updateHeader();
  void updateSelectionCountLabel();
  void treeDataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &roles);

  void treeContextMenu(const QPoint &pos);

  // Context Menu slots
  void setSelectedChecked(bool checked);
  void revertRenamingSelected();
  void uncheckPublished();

private:
  Ui::ChannelWidget ui_;

  bool init_button_state_is_init_;

  ChannelTreeModel*            channel_model_;
  QStableSortFilterProxyModel* channel_proxy_model_;
  QCheckboxHeaderView*         tree_header_;
  QTimer*                      message_counter_update_timer_;

  QByteArray initial_tree_state_;

  QList<ChannelTreeItem*> getSelectedChannels() const;

  QList<ChannelTreeItem*> getCheckedChannels() const;

  std::map<std::string, std::string> getChannelMapping() const;

  void setInitButtonToInit();
  void setInitButtonToDeInit();

  void autoSizeColumns();

  void saveLayout();
  void restoreLayout();
  void saveInitialLayout();
};
