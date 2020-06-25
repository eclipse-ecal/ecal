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

#include "ui_recording_history_widget.h"
#include "push_button_delegate.h"

#include "job_history_model.h"
#include "job_history_recorderitem.h"
#include "job_history_jobitem.h"

#include <CustomQt/QStableSortFilterProxyModel.h>

class RecordingHistoryWidget : public QWidget
{
  Q_OBJECT


////////////////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////////////////
public:
  RecordingHistoryWidget(QWidget *parent = Q_NULLPTR);
  ~RecordingHistoryWidget();

private:
  void contextMenu(const QPoint &pos);

////////////////////////////////////////////////
// Save / Restore layout
////////////////////////////////////////////////

protected:
  void showEvent(QShowEvent *event) override;

private:
  void saveLayout();
  void restoreLayout();
  void saveInitialLayout();

public slots:
  void resetLayout();

////////////////////////////////////////////////
// Upload / Merge
////////////////////////////////////////////////
private:
  void uploadSelectedMeasurements();
  void treeViewClicked(const QModelIndex& proxy_index);

////////////////////////////////////////////////
// Delete
////////////////////////////////////////////////
private:
  void deleteSelectedMeasurements();

////////////////////////////////////////////////
// Auxiliary Methods
////////////////////////////////////////////////
private:
  QModelIndexList selectedSourceIndexes(int column = (int)JobHistoryModel::Columns::JOB) const;
  QModelIndexList selectedJobItemIndexes(int column = (int)JobHistoryModel::Columns::JOB) const;
  QList<JobHistoryJobItem*> selectedJobItems() const;

////////////////////////////////////////////////
// Member variables
////////////////////////////////////////////////
private:
  Ui::RecordingHistoryWidget ui_;

  JobHistoryModel*             job_history_model_;
  QStableSortFilterProxyModel* job_history_proxy_model_;

  PushButtonDelegate*          add_comment_delegate_;
  PushButtonDelegate*          upload_button_delegate_;

  // Initial layout
  bool first_show_event_;
  QByteArray initial_job_history_tree_state_;
};
