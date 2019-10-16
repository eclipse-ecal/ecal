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
#include "ui_recordermanager_widget.h"

#include "models/recorder_model.h"

#include <CustomQt/QStableSortFilterProxyModel.h>
#include <CustomQt/QCheckboxHeaderView.h>

#include "host_filter_delegate.h"

class RecorderManagerWidget : public QWidget
{
  Q_OBJECT

public:
  RecorderManagerWidget(QWidget *parent = Q_NULLPTR);
  ~RecorderManagerWidget();

////////////////////////////////////
/// Internal slots
////////////////////////////////////
private slots:
  void preBufferingEnabledChanged(bool enabled);
  void maxPreBufferLengthChanged(std::chrono::steady_clock::duration max_buffer_length);

  void headerClicked(int column, bool checked);
  void updateHeader();
  void treeDataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &roles);

  void localRecordingButtonPressed();

  void addNewRecorder();
  void removeSelectedRecorders();
  void enableSelectedRecorders();
  void disableSelectedRecorders();
  void invertRecorderSelection();

  void updateRemoveButton();
  void updateLocalRecordingButton();

  void contextMenu(const QPoint &pos);

  void setCurrentlyInUseTopicsToDelegate(const std::vector<std::pair<std::string, std::set<std::string>>>& recorder_instances);

////////////////////////////////////
/// Helper methods
////////////////////////////////////
  QModelIndexList getSelectedRows() const;
  QModelIndexList getSelectedRemovableRows() const;

////////////////////////////////////
/// Member variables
////////////////////////////////////
private:
  Ui::RecorderManagerWidget ui_;

  bool activate_button_state_is_ativate_;

  RecorderModel*               recorder_model_;
  QStableSortFilterProxyModel* recorder_proxy_model_;
  HostFilterDelegate*          host_filter_delegate_;
  QCheckboxHeaderView*         tree_header_;
};
