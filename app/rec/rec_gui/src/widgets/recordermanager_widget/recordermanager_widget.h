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
#include <QShowEvent>

#include "ui_recordermanager_widget.h"

#include <CustomQt/QMulticolumnSortFilterProxyModel.h>
#include <CustomQt/QCheckboxHeaderView.h>

#include "recorder_model.h"

#include "host_filter_delegate.h"

#include <rec_server_core/rec_server_config.h>

class RecorderManagerWidget : public QWidget
{
  Q_OBJECT

public:
  RecorderManagerWidget(QWidget *parent = Q_NULLPTR);
  ~RecorderManagerWidget();

////////////////////////////////////
// Save / Restore layout
////////////////////////////////////
protected:
  virtual void showEvent(QShowEvent *event) override;

private:
  void saveLayout();
  void restoreLayout();
  void saveInitialLayout();

public slots:
  void resetLayout();

////////////////////////////////////
/// Internal slots
////////////////////////////////////
private slots:
  void preBufferingEnabledChanged(bool enabled);
  void maxPreBufferLengthChanged(std::chrono::steady_clock::duration max_buffer_length);
  void updatePreBufferWarning();

  void headerClicked(int column, bool checked);
  void updateHeader();

  void treeDataChanged (const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &roles);
  void treeRowsInserted(const QModelIndex &parent, int first, int last);

  void localRecordingButtonPressed();

  void addButtonClicked();
  void removeSelectedRows();
  void enableSelectedRecorders();
  void disableSelectedRecorders();

  void updateAddButton();
  void updateRemoveButton();
  void updateLocalRecordingButton();

  void contextMenu(const QPoint &pos);

  void setCurrentlyInUseHostsToDelegate(const std::map<std::string, eCAL::rec_server::ClientConfig>& enabled_rec_clients);

////////////////////////////////////
/// Helper methods
////////////////////////////////////
  QModelIndexList getSelectedRowIndexes() const;
  std::set<int> getSelectedRows() const;

////////////////////////////////////
/// Member variables
////////////////////////////////////
private:
  Ui::RecorderManagerWidget ui_;

  bool activate_button_state_is_ativate_;

  RecorderModel*                    recorder_model_;
  QMulticolumnSortFilterProxyModel* recorder_proxy_model_;
  HostFilterDelegate*               host_filter_delegate_;
  QCheckboxHeaderView*              tree_header_;

  // Initial layout
  bool first_show_event_;
  QByteArray initial_recorder_list_state_;
};
