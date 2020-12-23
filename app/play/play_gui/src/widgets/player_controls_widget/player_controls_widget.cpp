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

#include "player_controls_widget.h"

#include "q_ecal_play.h"

#include <chrono>
#include <algorithm>
#include <iostream>

#include <ecal/ecal.h>

#include <QDebug>
#include <QTimer>
#include <QMainWindow>
#include <QListView>
#include <QAction>
#include <QSettings>

#include "widgets/models/tree_item_type.h"

PlayerControlsWidget::PlayerControlsWidget(QWidget* parent)
  : QWidget(parent)
  , play_button_state_is_play(true)
  , settings_visible_(false)
  , measurement_boundaries_(std::make_pair(eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0)), eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0))))
  , measurement_frame_count_(0)
{
  ui_.setupUi(this);

  // Settings panel
  settings_widget_ = new SettingsWidget(this);
  ui_.settings_frame_layout->addWidget(settings_widget_);
  setSettingsVisible(false);
  connect(ui_.view_settings_button, &QAbstractButton::clicked, this, [this]() {setSettingsVisible(!settings_visible_); });

  // Tree
  step_reference_channel_model_ = new ChannelTreeModel(this);
  step_reference_channel_model_->setEditEnabled(false);
  step_reference_channel_proxy_model_ = new QStableSortFilterProxyModel(this);
  step_reference_channel_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  step_reference_channel_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  step_reference_channel_proxy_model_->setSourceModel(step_reference_channel_model_);
  step_reference_channel_proxy_model_->sort((int)ChannelTreeModel::Columns::CHANNEL_NAME);

  ui_.step_channel_toolbutton->setModel(step_reference_channel_proxy_model_);
  ui_.step_channel_toolbutton->setModelColumn((int)ChannelTreeModel::Columns::CHANNEL_NAME);
  ui_.step_channel_toolbutton->setMaxVisibleItems(10);
  stepReferenceChannelChanged("");

  // Set a semi-fixed size to the position label
  QTimer::singleShot(1,
      [this]()
      {
        ui_.position_label->setText("9999.999 / 9999.999 s");
        QSize label_size = ui_.position_label->size();
        ui_.position_label->setMinimumSize(label_size);
        playStateChanged(QEcalPlay::instance()->currentPlayState());
      });

  // connect QEcalPlay -> this
  connect(QEcalPlay::instance(), &QEcalPlay::measurementLoadedSignal,           this, &PlayerControlsWidget::measurementLoaded);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementClosedSignal,           this, &PlayerControlsWidget::measurementClosed);
  connect(QEcalPlay::instance(), &QEcalPlay::playStateChangedSignal,            this, &PlayerControlsWidget::playStateChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::repeatEnabledChangedSignal,        this, &PlayerControlsWidget::repeatEnabledChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::stepReferenceChannelChangedSignal, this, &PlayerControlsWidget::stepReferenceChannelChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::channelMappingChangedSignal,       this, &PlayerControlsWidget::channelMappingChanged);

  // connect this -> QEcalPlay
  connect(ui_.open_button,       &QAbstractButton::clicked, QEcalPlay::instance(), &QEcalPlay::loadMeasurementFromFileDialog);
  connect(ui_.play_pause_button, &QPushButton::clicked, QEcalPlay::instance(),
      [this]()
      {
        if (play_button_state_is_play)
          QEcalPlay::instance()->play();
        else
          QEcalPlay::instance()->pause();
      });
  connect(ui_.stop_button,                     &QPushButton::clicked,          QEcalPlay::instance(), &QEcalPlay::stop);
  connect(ui_.step_forward_button,             &QPushButton::clicked,          QEcalPlay::instance(), [](){QEcalPlay::instance()->stepForward(); });
  connect(ui_.repeat_button,                   &QPushButton::toggled,          QEcalPlay::instance(), &QEcalPlay::setRepeatEnabled);
  connect(ui_.step_channel_toolbutton,         &QToolButton::clicked,          QEcalPlay::instance(), []() {QEcalPlay::instance()->stepChannel(); });

  // Slider
  connect(ui_.position_slider, &QSlider::valueChanged, this,
      [this](int value)
      {
        auto timestamp = measurement_boundaries_.first + std::chrono::milliseconds(value);
        QEcalPlay::instance()->jumpTo(timestamp);
      });
  connect(ui_.step_channel_toolbutton, &QListMenuToolButton::itemSelected,
      [this](const QModelIndex& proxy_index)
      {
        if (proxy_index.isValid())
        {
          auto source_index = step_reference_channel_proxy_model_->mapToSource(proxy_index);
          const ChannelTreeItem* channel_item = static_cast<const ChannelTreeItem*>(step_reference_channel_model_->item(source_index));
          QEcalPlay::instance()->setStepReferenceChannel(channel_item->sourceChannelName());
        }
        else
        {
          QEcalPlay::instance()->setStepReferenceChannel("");
        }
      });

  if (QEcalPlay::instance()->isMeasurementLoaded())
  {
    measurementLoaded(QEcalPlay::instance()->measurementPath());
  }
  repeatEnabledChanged(QEcalPlay::instance()->isRepeatEnabled());
  stepReferenceChannelChanged(QEcalPlay::instance()->stepReferenceChannel());

  restoreLayout();
}

PlayerControlsWidget::~PlayerControlsWidget()
{
  saveLayout();
}

void PlayerControlsWidget::setSettingsVisible(bool visible)
{
  if (visible)
  {
    //                                    ◄◄
    ui_.view_settings_button->setText(tr("\342\226\272\342\226\272"));
    ui_.view_settings_button->setToolTip(tr("Hide Settings"));
  }
  else
  {
    //                                    ►►
    ui_.view_settings_button->setText(tr("\342\227\204\342\227\204"));
    ui_.view_settings_button->setToolTip(tr("Show Settings"));
  }
  ui_.settings_frame->setVisible(visible);
  settings_visible_ = visible;
}

////////////////////////////////////////////////////////////////////////////////
//// Slots  for QEcalPlay                                                   ////
////////////////////////////////////////////////////////////////////////////////
void PlayerControlsWidget::measurementLoaded(const QString& /*path*/)
{
  measurement_boundaries_  = QEcalPlay::instance()->measurementBoundaries();
  measurement_frame_count_ = QEcalPlay::instance()->frameCount();

  ui_.position_slider->blockSignals(true);
  ui_.position_slider->setMinimum(-10);
  ui_.position_slider->setMaximum(std::chrono::duration_cast<std::chrono::milliseconds>(measurement_boundaries_.second - measurement_boundaries_.first).count());
  ui_.position_slider->blockSignals(false);

  playStateChanged(QEcalPlay::instance()->currentPlayState());

  ui_.position_slider                ->setEnabled(true);
  ui_.play_pause_button              ->setEnabled(true);
  ui_.stop_button                    ->setEnabled(true);
  ui_.step_forward_button            ->setEnabled(true);
  ui_.step_channel_toolbutton        ->setEnabled(true);
}

void PlayerControlsWidget::measurementClosed()
{
  measurement_boundaries_.first  = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
  measurement_boundaries_.second = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
  measurement_frame_count_ = 0;
  playStateChanged(QEcalPlay::instance()->currentPlayState());

  ui_.position_slider                ->setEnabled(false);
  ui_.play_pause_button              ->setEnabled(false);
  ui_.stop_button                    ->setEnabled(false);
  ui_.step_forward_button            ->setEnabled(false);
  ui_.step_channel_toolbutton        ->setEnabled(false);
}

void PlayerControlsWidget::playStateChanged(const EcalPlayState& current_state)
{
  // Play / Pause button
  if (current_state.playing_)
  {
    setPlayPauseButtonToPause();
  }
  else
  {
    setPlayPauseButtonToPlay();
  }

  // Position Slider
  ui_.position_slider->blockSignals(true);
  if (!ui_.position_slider->isSliderDown())
  {
    ui_.position_slider->setValue(std::chrono::duration_cast<std::chrono::milliseconds>(current_state.current_frame_timestamp - measurement_boundaries_.first).count());
  }
  ui_.position_slider->blockSignals(false);

  // Position label
  auto relative_position = std::chrono::duration_cast<std::chrono::duration<double>>(current_state.current_frame_timestamp - measurement_boundaries_.first);
  auto measurement_length = std::chrono::duration_cast<std::chrono::duration<double>>(measurement_boundaries_.second - measurement_boundaries_.first);
  QString label = QString::number(relative_position.count(), 'f', 3) + " / " + QString::number(measurement_length.count(), 'f', 3) +  " s";
  ui_.position_label->setText(label);
}

void PlayerControlsWidget::repeatEnabledChanged(bool enabled)
{
  if (ui_.repeat_button->isChecked() != enabled)
  {
    ui_.repeat_button->blockSignals(true);
    ui_.repeat_button->setChecked(enabled);
    ui_.repeat_button->blockSignals(false);
  }
}

void PlayerControlsWidget::stepReferenceChannelChanged(const QString& step_reference_channel)
{
  if (!step_reference_channel.isEmpty())
  {
    auto channel_mapping   = QEcalPlay::instance()->channelMapping();
    auto target_channel_it = channel_mapping.find(step_reference_channel.toStdString());

    if (target_channel_it != channel_mapping.end())
    {
      ui_.step_channel_toolbutton->setText(target_channel_it->second.c_str());
    }
    else
    {
      ui_.step_channel_toolbutton->setText(step_reference_channel);
    }

    ui_.step_channel_toolbutton->setStyleSheet("QToolButton{}");
  }
  else
  {
    ui_.step_channel_toolbutton->setText(tr("Step channel"));
    ui_.step_channel_toolbutton->setStyleSheet("QToolButton{font: italic;}");
  }



  //if (step_reference_channel != getSelectedStepReferenceChannel().first)
  //{
    ui_.step_channel_toolbutton->blockSignals(true);
    setSelectedStepReferenceChannel(step_reference_channel);
    ui_.step_channel_toolbutton->blockSignals(false);
  //}
}

void PlayerControlsWidget::channelMappingChanged(const std::map<std::string, std::string>& channel_mapping)
{
  // Remove non-existing items
  auto children_to_remove = step_reference_channel_model_->root()->findChildren(
      [&channel_mapping](const QAbstractTreeItem* const item)
      {
        if (item && item->type() == (int)TreeItemType::Channel)
        {
          auto channel_item = static_cast<const ChannelTreeItem*>(item);
          if (channel_mapping.find(channel_item->sourceChannelName().toStdString()) == channel_mapping.end())
          {
            return true;
          }
        }
        return false;
      });

  step_reference_channel_model_->removeItems(children_to_remove);


  // Update or create existing items
  for (auto& channel_pair : channel_mapping)
  {
    auto exisiting_items = step_reference_channel_model_->root()->findChildren(
        [&channel_pair](const QAbstractTreeItem* const item)
        {
          if (item && item->type() == (int)TreeItemType::Channel)
          {
            auto channel_item = static_cast<const ChannelTreeItem*>(item);
            if (channel_pair.first == channel_item->sourceChannelName().toStdString())
            {
              return true;
            }
          }
          return false;
        });

    if (exisiting_items.empty())
    {
      // Create a new item
      ChannelTreeItem* channel_item = new ChannelTreeItem(channel_pair.first.c_str());
      channel_item->setTargetChannelName(channel_pair.second.c_str());
      step_reference_channel_model_->insertItem(channel_item);
    }
    else
    {
      // Update existing item
      QAbstractTreeItem* channel_item = exisiting_items.at(0);
      if (static_cast<const ChannelTreeItem*>(channel_item)->targetChannelName() != channel_pair.second.c_str())
      {
        QModelIndex index = step_reference_channel_model_->index(channel_item, (int)ChannelTreeModel::Columns::CHANNEL_NAME);
        step_reference_channel_model_->setData(index, channel_pair.second.c_str(), Qt::ItemDataRole::EditRole);
      }
    }
  }

  // Update step channel button
  stepReferenceChannelChanged(QEcalPlay::instance()->stepReferenceChannel());
}

////////////////////////////////////////////////////////////////////////////////
//// Helpers                                                                ////
////////////////////////////////////////////////////////////////////////////////

void PlayerControlsWidget::setPlayPauseButtonToPlay()
{
  if (!play_button_state_is_play)
  {
    ui_.play_pause_button->setIcon(QPixmap(":/ecalicons/START"));
    play_button_state_is_play = true;
  }
}

void PlayerControlsWidget::setPlayPauseButtonToPause()
{
  if (play_button_state_is_play)
  {
    ui_.play_pause_button->setIcon(QPixmap(":/ecalicons/PAUSE"));
    play_button_state_is_play = false;
  }
}

void PlayerControlsWidget::setSelectedStepReferenceChannel(const QString& step_reference_channel)
{
  auto exisiting_items = step_reference_channel_model_->root()->findChildren(
      [&step_reference_channel](const QAbstractTreeItem* const item)
      {
        if (item && item->type() == (int)TreeItemType::Channel)
        {
          auto channel_item = static_cast<const ChannelTreeItem*>(item);
          if (step_reference_channel == channel_item->sourceChannelName())
          {
            return true;
          }
        }
        return false;
      });

  if (!exisiting_items.isEmpty())
  {
    auto source_index = step_reference_channel_model_->index(exisiting_items.at(0), (int)ChannelTreeModel::Columns::CHANNEL_NAME);
    auto proxy_index = step_reference_channel_proxy_model_->mapFromSource(source_index);
    ui_.step_channel_toolbutton->setSelection(proxy_index.row());
  }
  else
  {
    ui_.step_channel_toolbutton->setSelection(QModelIndex());
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Save Layout                                                            ////
////////////////////////////////////////////////////////////////////////////////

void PlayerControlsWidget::saveLayout()
{
  QSettings settings;
  settings.beginGroup("player_controls");
  settings.setValue("settings_visible", settings_visible_);
  settings.endGroup();
}

void PlayerControlsWidget::restoreLayout()
{
  QSettings settings;
  settings.beginGroup("player_controls");
  QVariant settings_visible_variant = settings.value("settings_visible");
  if (settings_visible_variant.isValid())
  {
    bool settings_visible = settings_visible_variant.toBool();
    setSettingsVisible(settings_visible);
  }
  settings.endGroup();
}

void PlayerControlsWidget::resetLayout()
{
  setSettingsVisible(false);
  settings_widget_->resetLayout();
  QEcalPlay::instance()->setStepReferenceChannel("");
}
