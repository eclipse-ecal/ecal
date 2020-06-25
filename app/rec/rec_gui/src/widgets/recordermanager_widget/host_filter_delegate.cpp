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

#include "host_filter_delegate.h"

#include <QComboBox>
#include <QStandardItemModel>
#include <QPainter>

#include "host_picker.h"

HostFilterDelegate::HostFilterDelegate(QAbstractItemView* parent)
  : QStyledItemDelegate(parent)
  , first_run_(true)
  , dialog_open_(false)
  , open_popup_(false)
{
  parent->viewport()->setAttribute(Qt::WA_Hover); // Important for mouse-hover events
   
  available_hosts_model_ = new QStandardItemModel(this);

  QStandardItem* all_item = new QStandardItem();
  all_item->setData("All", Qt::ItemDataRole::DisplayRole);
  all_item->setData((int)HostItemType::All, ItemTypeRole);
  all_item->setData(QStringList(), HostFilterListRole);

  QStandardItem* custom_item = new QStandardItem();
  custom_item->setData("Custom...", Qt::ItemDataRole::DisplayRole);
  custom_item->setData((int)HostItemType::Custom, ItemTypeRole);

  available_hosts_model_->appendRow(all_item);
  available_hosts_model_->appendRow(custom_item);
}

QWidget *HostFilterDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex& /*index*/) const
{
  open_popup_ = true;

  QComboBox *editor = new QComboBox(parent);

  editor->setModel(available_hosts_model_);
  
  if (first_run_)
  {
    // Only add the separator once, they would otherwise stack up
    editor->insertSeparator(2);
    first_run_ = false;
  }

  connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &HostFilterDelegate::indexActivated);

  return editor;
}

void HostFilterDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  if (dialog_open_)
    return;

  QComboBox *combobox = static_cast<QComboBox*>(editor);

  QStringList current_host_filter = index.model()->data(index, Qt::ItemDataRole::EditRole).toStringList();

  // The custom element will hold the current host filter, just in case.
  combobox->model()->setData(combobox->model()->index(1, 0), current_host_filter, HostFilterListRole);

  if (current_host_filter.empty())
  {
    // Entry "All"
    combobox->setCurrentIndex(0);
  }
  else if (current_host_filter.size() > 1)
  {
    // Entry "Custom"
    combobox->setCurrentIndex(1);
  }
  else
  {
    // Specific host entry
    QString hostname = current_host_filter.front();
    int host_index = indexOf(hostname);

    if (host_index >= 0)
    {
      // Use the specific host entry for existing hosts
      combobox->setCurrentIndex(host_index);
    }
    else
    {
      // Use the custom entry for unknown hosts
      combobox->setCurrentIndex(1);
    }
  }

  if (open_popup_)
  {
    open_popup_ = false;
    combobox->showPopup();
  }
}

void HostFilterDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  QComboBox *combobox = static_cast<QComboBox*>(editor);

  QStringList selected_hosts = combobox->currentData(HostFilterListRole).toStringList();
  model->setData(index, selected_hosts, Qt::ItemDataRole::EditRole);
}

void HostFilterDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

void HostFilterDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter, option, index); // call base

  // Get the ItemView of this delegate
  const QAbstractItemView* item_view = static_cast<const QAbstractItemView*>(option.widget);

  // Check whether the row and / or the item is hovered
  QPoint p = item_view->viewport()->mapFromGlobal(QCursor::pos());

  bool row_hover  = option.state & QStyle::State_MouseOver;
  bool cell_hover = (item_view->indexAt(p) == index);

  if (row_hover && (index.flags() & Qt::ItemFlag::ItemIsEditable))
  {
    QStyleOptionComboBox combobox_options;
    
    // At least on Ubuntu 16.04 with Qt 5.5 we can only paint elements starting
    // at 0,0. Thus, we create a rect with the same width and height as the
    // target rect and then move the painter back and forth to paint in the
    // given rect.
    painter->translate(option.rect.topLeft());
    combobox_options.rect = QRect(0, 0, option.rect.width(), option.rect.height());
    
    combobox_options.currentText = index.data(Qt::ItemDataRole::DisplayRole).toString();
    combobox_options.fontMetrics = QFontMetrics(index.data(Qt::ItemDataRole::FontRole).value<QFont>());
    combobox_options.editable = false;
    combobox_options.state = QFlags<QStyle::StateFlag>(QStyle::StateFlag::State_Enabled & ~QStyle::StateFlag::State_HasFocus);

    if (cell_hover)
      combobox_options.state |= (QStyle::StateFlag::State_HasFocus | QStyle::StateFlag::State_MouseOver);

    combobox_options.frame = true;
    combobox_options.currentIcon = index.data(Qt::ItemDataRole::DecorationRole).value<QIcon>();
    
    QRect text_rect = QApplication::style()->subControlRect(QStyle::CC_ComboBox, &combobox_options, QStyle::SC_ComboBoxEditField);
    combobox_options.iconSize = QSize(text_rect.height(), text_rect.height());

    QApplication::style()->drawComplexControl(QStyle::CC_ComboBox, &combobox_options, painter);
    QApplication::style()->drawControl(QStyle::CE_ComboBoxLabel, &combobox_options, painter);

    painter->translate(-option.rect.topLeft().x(), -option.rect.topLeft().y());
  }
}

QSize HostFilterDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  auto original_size = QStyledItemDelegate::sizeHint(option, index);

  int frame_width;

  if (option.widget != nullptr)
    frame_width = option.widget->style()->pixelMetric(QStyle::PixelMetric::PM_DefaultFrameWidth);
  else
    frame_width = QApplication::style()->pixelMetric(QStyle::PixelMetric::PM_DefaultFrameWidth);

  return QSize(original_size.width(), option.decorationSize.height() + 4 * frame_width);
}

void HostFilterDelegate::indexActivated(int index)
{
  QComboBox *combobox = static_cast<QComboBox*>(sender());

  if (index >= 0)
  {
    QModelIndex selected_index = combobox->model()->index(index, 0);

    if (combobox->model()->data(selected_index, ItemTypeRole) == (int)HostItemType::All)
    {
      emit commitData(combobox);
    }
    else if (combobox->model()->data(selected_index, ItemTypeRole) == (int)HostItemType::Custom)
    {
      dialog_open_ = true;
      QStringList available_hosts;
      for (int i = 0; i < available_hosts_model_->rowCount(); i++)
      {
        if (available_hosts_model_->item(i)->data(ItemTypeRole).toInt() == (int)HostItemType::Host)
        {
          QString host_at_current_index = available_hosts_model_->item(i)->data(HostFilterListRole).toStringList().front();
          available_hosts.push_back(host_at_current_index);
        }
      }

      HostPicker host_picker(available_hosts, combobox->model()->data(selected_index, HostFilterListRole).toStringList(), combobox);
      int result = host_picker.exec();

      if (result == QDialog::DialogCode::Accepted)
      {
        QStringList selected_hosts = host_picker.getSelectedHosts();
        combobox->model()->setData(combobox->model()->index(index, 0), selected_hosts, HostFilterListRole);
        emit commitData(combobox);
        emit closeEditor(combobox, QAbstractItemDelegate::EndEditHint::SubmitModelCache);
      }
      else
      {
        emit closeEditor(combobox, QAbstractItemDelegate::EndEditHint::RevertModelCache);
      }
      dialog_open_ = false;
    }
    else if (combobox->model()->data(selected_index, ItemTypeRole) == (int)HostItemType::Host)
    {
      emit commitData(combobox);
    }
  }
}

int HostFilterDelegate::indexOf(const QString& host) const
{
  int host_index = -1;
  for (int i = 0; i < available_hosts_model_->rowCount(); i++)
  {
    // Lookup the specific host entry
    QModelIndex available_hosts_index = available_hosts_model_->index(i, 0);
    if ((available_hosts_model_->data(available_hosts_index, ItemTypeRole).toInt() == (int)HostItemType::Host)
      && (available_hosts_model_->data(available_hosts_index, HostFilterListRole).toStringList().front() == host))
    {
      host_index = i;
      break;
    }
  }
  return host_index;
}

void HostFilterDelegate::setMonitorHosts(const std::set<QString>& host_list)
{
  setHosts(host_list, true, false);
}

void HostFilterDelegate::setHostsCurrentlyInUse(const std::set<QString>& host_list)
{
  setHosts(host_list, false, true);
}

void HostFilterDelegate::setHosts(const std::set<QString>& host_list, bool monitor_visible, bool currently_in_use)
{
  // Update existing entries
  for (int i = 0; i < available_hosts_model_->rowCount(); i++)
  {
    QStandardItem* const item = available_hosts_model_->item(i);
    if (item->data(ItemTypeRole).toInt() == (int)HostItemType::Host)
    {
      const QString hostname = item->data(Qt::ItemDataRole::DisplayRole).toString();
      if (monitor_visible)
        item->setData((host_list.find(hostname) != host_list.end()), AvailableInMonitoring);
      if (currently_in_use)
        item->setData((host_list.find(hostname) != host_list.end()), CurrentlyInUse);
    }
  }

  // Add new entries
  for (const QString& host : host_list)
  {
    int host_index = indexOf(host);
    if (host_index < 0)
    {
      addAvailableHost(host, monitor_visible, currently_in_use);
    }
  }

  // Remove items that are neither visible in the monitor nor currently in use
  cleanModel();
}

void HostFilterDelegate::addAvailableHost(const QString& new_host, bool monitor_visible, bool currently_in_use)
{
  int i = 0;

  // Iterate to the first Host element
  for (; i < available_hosts_model_->rowCount(); i++)
  {
    if (available_hosts_model_->item(i)->data(ItemTypeRole).toInt() == (int)HostItemType::Host)
      break;
  }

  // Iterate to the element where we want to insert the new host alphabetically
  for (; i < available_hosts_model_->rowCount(); i++)
  {
    if (available_hosts_model_->item(i)->data(ItemTypeRole).toInt() == (int)HostItemType::Host)
    {
      QString host_at_current_index = available_hosts_model_->item(i)->data(HostFilterListRole).toStringList().front();

      if (host_at_current_index == new_host)
        return; // No double entries!
      else if (host_at_current_index.compare(new_host, Qt::CaseSensitivity::CaseInsensitive) > 0)
        break;  // i now holds the position where we want to insert the new host
    }
  }

  QStandardItem* new_item = new QStandardItem();
  new_item->setData(new_host, Qt::ItemDataRole::DisplayRole);
  new_item->setData((int)HostItemType::Host, ItemTypeRole);
  new_item->setData(QStringList{ new_host }, HostFilterListRole);
  new_item->setData(monitor_visible, AvailableInMonitoring);
  new_item->setData(currently_in_use, CurrentlyInUse);
  available_hosts_model_->insertRow(i, new_item);
}

void HostFilterDelegate::cleanModel()
{
  for (int i = 0; i < available_hosts_model_->rowCount(); i++)
  {
    if (available_hosts_model_->item(i)->data(ItemTypeRole).toInt() == (int)HostItemType::Host)
    {
      const QStandardItem* const item = available_hosts_model_->item(i);
      if (!(item->data(AvailableInMonitoring).toBool() || item->data(CurrentlyInUse).toBool()))
      {
        available_hosts_model_->removeRow(i);
        i++;
      }
    }
  }
}
