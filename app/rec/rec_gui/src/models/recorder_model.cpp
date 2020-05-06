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

#include "recorder_model.h"

#include <QFont>
#include <QStyle>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QApplication>

#include "qecalrec.h"
#include <algorithm>

#include <ecal/ecal.h>
#include "ecal_utils/string.h"
#include "item_data_roles.h"

//////////////////////////////////////////////////////
// Constructor & destructor
//////////////////////////////////////////////////////

RecorderModel::RecorderModel(QObject *parent)
  : QAbstractItemModel(parent)
  , icon_client_connected   (":/ecalicons/CONNECTED")
  , icon_client_disconnected(":/ecalicons/DISCONNECTED")
  , icon_waiting            (":/ecalicons/WAIT")
  , icon_connected          (":/ecalicons/CHECKMARK")
  , icon_disconnected       (":/ecalicons/CROSS")
  , icon_recording          (":/ecalicons/RECORD")
  , icon_flushing           (":/ecalicons/SAVE_TO_DISK")
  , icon_error              (":/ecalicons/ERROR")
  , icon_warning            (":/ecalicons/WARNING")
{
  connect(QEcalRec::instance(), &QEcalRec::stateUpdateSignal,              this, &RecorderModel::stateUpdate);
  connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal,           this, &RecorderModel::monitorUpdated);

  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal, this, &RecorderModel::setRecorderInstances);
  connect(QEcalRec::instance(), &QEcalRec::hostFilterChangedSignal,        this, &RecorderModel::setHostFilter);

  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,    this, &RecorderModel::recordingStateChanged);

  connect(QEcalRec::instance(), &QEcalRec::usingBuiltInRecorderEnabledChangedSignal, this,
      [this]()
      {
        // Update the local recorder
        for (int i = 0; i < (int)recorder_list_.size(); i++)
        {
          if (recorder_list_[i].hostname_ != eCAL::Process::GetHostName())
          {
            emitDataChangedAll(i);
            break;
          }
        }
      });

  // Initial states
  setRecorderInstances(QEcalRec::instance()->recorderInstances());
  recordingStateChanged(QEcalRec::instance()->recordersRecording());
}

RecorderModel::~RecorderModel()
{}

//////////////////////////////////////////////////////
// Reimplemented from QAbstractItemModel
//////////////////////////////////////////////////////

QVariant RecorderModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant::Invalid;

  const int     row    = index.row();
  const Columns column = (Columns)index.column();

  if (role == Qt::ItemDataRole::FontRole)
  {
    if (!recorder_list_[row].ecal_rec_visible_
      && (!QEcalRec::instance()->usingBuiltInRecorderEnabled() || (recorder_list_[row].hostname_ != eCAL::Process::GetHostName())))
    {
      QFont font;
      font.setItalic(true);
      return font;
    }
  }

#ifdef WIN32
  // On windows, the rows height changes every time the tree view is supposed to
  // display an icon. Thus, we override the size hint, here.
  // The implementation is not good at all, so we don't want it on linux, but
  // for windows it does solve the problem.
  if (role == Qt::ItemDataRole::SizeHintRole)
  {
    int icon_size       = qApp->style()->pixelMetric(QStyle::PixelMetric::PM_ListViewIconSize);
    int checkbox_width  = qApp->style()->pixelMetric(QStyle::PixelMetric::PM_IndicatorWidth);
    int checkbox_height = qApp->style()->pixelMetric(QStyle::PixelMetric::PM_IndicatorHeight);
    int frame_size      = qApp->style()->pixelMetric(QStyle::PixelMetric::PM_DefaultFrameWidth);
    int layout_spacing  = qApp->style()->pixelMetric(QStyle::PixelMetric::PM_DefaultLayoutSpacing);

    QString text = data(index, Qt::ItemDataRole::DisplayRole).toString();
    QFont font   = data(index, Qt::ItemDataRole::FontRole).value<QFont>();
    font.setPointSize(font.pointSize() + 1);
    QFontMetrics font_metrics(font);

    int text_width  = font_metrics.horizontalAdvance(text);
    int text_height = font_metrics.height();

    bool display_icon     = data(index, Qt::ItemDataRole::DecorationRole).isValid();
    bool display_checkbox = data(index, Qt::ItemDataRole::CheckStateRole).isValid();

    QSize size_hint;

    // Make sure we never have to adjust the height
    size_hint.setHeight(std::max(std::max(text_height, icon_size), checkbox_height));
    size_hint.setHeight(size_hint.height() + 2 * frame_size);

    // Use the actual width as width
    size_hint.setWidth(text_width + layout_spacing * 2 + (display_icon ? icon_size + layout_spacing * 2: 0) + (display_checkbox ? checkbox_width + layout_spacing * 2 : 0));

    return size_hint;
  }
#endif // WIN32

  if (column == Columns::ENABLED)
  {
    if (role == Qt::ItemDataRole::CheckStateRole)
    {
      return (recorder_list_[row].recording_enabled_ ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (recorder_list_[row].recording_enabled_
        && !recorder_list_[row].ecal_rec_visible_
        && (!QEcalRec::instance()->usingBuiltInRecorderEnabled() || (recorder_list_[row].hostname_ != eCAL::Process::GetHostName())))
      {
        return icon_warning;
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      if (recorder_list_[row].recording_enabled_
        && !recorder_list_[row].ecal_rec_visible_
        && (!QEcalRec::instance()->usingBuiltInRecorderEnabled() || (recorder_list_[row].hostname_ != eCAL::Process::GetHostName())))
      {
        return tr("eCALRec is not running");
      }
    }
  }

  else if (column == Columns::HOSTNAME)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return recorder_list_[row].hostname_.c_str();
    }
    if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        if (recorder_list_[row].request_pending_)
        {
          return icon_waiting;
        }
        else if (recorder_list_[row].connected_to_client_)
        {
          return icon_client_connected;
        }
        else
        {
          return icon_client_disconnected;
        }
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        if (recorder_list_[row].request_pending_)
        {
          return tr("Request pending");
        }
        else if (recorder_list_[row].connected_to_client_)
        {
          return tr("Connected");
        }
        else
        {
          return tr("Not connected");
        }
      }
    }
  }

  else if (column == Columns::HOST_FILTER)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        if (recorder_list_[row].host_filter_.empty())
        {
          return "All";
        }
        else
        {
          QString host_filter_list;
          auto host_filter_it = recorder_list_[row].host_filter_.begin();
          for (; host_filter_it != recorder_list_[row].host_filter_.end(); host_filter_it++)
          {
            if (host_filter_it != recorder_list_[row].host_filter_.begin())
              host_filter_list += ", ";
            host_filter_list += QString::fromStdString(*host_filter_it);
          }
          return host_filter_list;
        }
      }
    }
    else if (role == Qt::ItemDataRole::EditRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        QStringList host_filter;
        for (const std::string& host : recorder_list_[row].host_filter_)
        {
          host_filter.push_back(QString::fromStdString(host));
        }
        return host_filter;
      }
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (recorder_list_[row].double_host_filter_warning_)
      {
        return icon_warning;
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      if (recorder_list_[row].double_host_filter_warning_)
      {
        return "Some hosts are recorded by more than one recorder!";
      }
      else
      {
        return data(index, Qt::ItemDataRole::DisplayRole);
      }
    }
  }

  else if (column == Columns::STATE)
  {
    if (recorder_list_[row].recording_enabled_)
    {
      if (!recorder_list_[row].connected_to_client_)
      {
        if (role == Qt::ItemDataRole::DisplayRole)
        {
          return "Offline";
        }
      }
      else if (recorder_list_[row].state_.main_recorder_state_.flushing_)
      {
        if (role == Qt::ItemDataRole::DisplayRole)
        {
          return "Flushing ("
            + QString::number(recorder_list_[row].state_.main_recorder_state_.recording_queue_.first) + " frames left)";
        }
        else if (role == Qt::ItemDataRole::DecorationRole)
        {
          return icon_flushing;
        }
      }
      else if (recorder_list_[row].state_.main_recorder_state_.recording_)
      {
        if (role == Qt::ItemDataRole::DisplayRole)
        {
          return "Recording ("
            + QString::number(std::chrono::duration_cast<std::chrono::duration<double>>(recorder_list_[row].state_.main_recorder_state_.recording_length_.second).count(), 'f', 1) + " s / "
            + QString::number(recorder_list_[row].state_.main_recorder_state_.recording_length_.first) + " frames)";
        }
        else if (role == Qt::ItemDataRole::DecorationRole)
        {
          return icon_recording;
        }
        else if (role == ItemDataRoles::SortRole)
        {
          return std::chrono::duration_cast<std::chrono::duration<double>>(recorder_list_[row].state_.main_recorder_state_.recording_length_.second).count();
        }
      }
      else if (recorder_list_[row].state_.initialized_)
      {
        if (role == Qt::ItemDataRole::DisplayRole)
        {
          return tr("Subscribing (") + QString::number(recorder_list_[row].state_.subscribed_topics_.size()) + tr(" Topics)");
        }
        else if (role == Qt::ItemDataRole::DecorationRole)
        {
          return icon_connected;
        }
      }
      else
      {
        if (role == Qt::ItemDataRole::DisplayRole)
        {
          return tr("Not subscribing");
        }
        else if (role == Qt::ItemDataRole::DecorationRole)
        {
          return icon_disconnected;
        }
      }

      if (role == Qt::ItemDataRole::ToolTipRole)
      {
        QString tooltip;
        if (!recorder_list_[row].connected_to_client_)
        {
          tooltip += "Offline";
        }
        else
        {
          tooltip += "Online";

          if (!recorder_list_[row].state_.initialized_)
          {
            tooltip += "\nDisconnected from eCAL";
          }
          else
          {
            tooltip += "\nSubscribed to " + QString::number(recorder_list_[row].state_.subscribed_topics_.size()) + " Topics";
          }

          if (recorder_list_[row].state_.main_recorder_state_.recording_ || recorder_list_[row].state_.main_recorder_state_.flushing_)
          {
            tooltip += "\n" + data(index, Qt::ItemDataRole::DisplayRole).toString();
          }
        }

        return tooltip;
      }
    }
  }

  else if (column == Columns::BUFFER)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      if (recorder_list_[row].connected_to_client_)
      {
        return QString::number(std::chrono::duration_cast<std::chrono::duration<double>>(recorder_list_[row].state_.pre_buffer_length_.second).count(), 'f', 1) + " s / "
          + QString::number(recorder_list_[row].state_.pre_buffer_length_.first) + " frames";
      }
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (recorder_list_[row].connected_to_client_)
      {
        if (recorder_list_[row].state_.buffer_writers_.size() > 0)
        {
          return icon_flushing;
        }
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      if (recorder_list_[row].state_.buffer_writers_.size() > 0)
      {
        size_t frames_to_write = 0;
        for (const auto& writer_state : recorder_list_[row].state_.buffer_writers_)
        {
          frames_to_write += writer_state.recording_queue_.first;
        }
        QString tooltip = (recorder_list_[row].state_.buffer_writers_.size() > 1
          ? "Saving " + QString::number(recorder_list_[row].state_.buffer_writers_.size()) + " buffers ("
          : "Saving 1 buffer (");
        tooltip += (QString::number(frames_to_write) + " frames left)");
        return tooltip;
      }
    }
    else if (role == ItemDataRoles::SortRole)
    {
      return std::chrono::duration_cast<std::chrono::duration<double>>(recorder_list_[row].state_.pre_buffer_length_.second).count();
    }
  }

  else if (column == Columns::ERROR)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return recorder_list_[row].last_response_.second.c_str();
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      return data(index, Qt::ItemDataRole::DisplayRole);
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (!recorder_list_[row].last_response_.first)
      {
        return icon_error;
      }
    }
  }

  if ((role == ItemDataRoles::SortRole)
    || (role == ItemDataRoles::FilterRole))
  {
    // If the column has not explicitely defined a sort or filter role, we fall back to the display role.
    return data(index, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant::Invalid;
}

bool RecorderModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || QEcalRec::instance()->recordersRecording())
    return false;

  if (role == Qt::ItemDataRole::EditRole)
  {
    std::set<std::string> host_filter;
    for (const auto& host : value.toStringList())
    {
      host_filter.emplace(host.toStdString());
    }

    bool success = QEcalRec::instance()->setHostFilter(recorder_list_[index.row()].hostname_, host_filter);

    if (success)
    {
      // Only actually change the model, if setting the host filter succeeded
      recorder_list_[index.row()].host_filter_ = host_filter;
      emit dataChanged(index, index);

      // Update the host filter warnings
      updateHostFilterWarnings();
    }

    return success;
  }

  if (role == Qt::ItemDataRole::CheckStateRole)
  {
    Qt::CheckState checked = (Qt::CheckState)value.toInt();
    if (index.column() == (int)Columns::ENABLED)
    {
      bool success;
      if (checked == Qt::CheckState::Checked)
        success = QEcalRec::instance()->addRecorderInstance(recorder_list_[index.row()].hostname_, recorder_list_[index.row()].host_filter_);
      else
        success = QEcalRec::instance()->removeRecorderInstance(recorder_list_[index.row()].hostname_);

      if (success)
      {
        recorder_list_[index.row()].recording_enabled_ = (checked == Qt::CheckState::Checked);
        dataChanged(this->index(index.row(), 0), this->index(index.row(), (int)Columns::COLUMN_COUNT - 1));
        // Update the host filter warnings
        updateHostFilterWarnings();
      }

      return success;
    }
  }

  return false;
}

Qt::ItemFlags RecorderModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return QAbstractItemModel::flags(index);

  if (index.column() == (int)Columns::ENABLED)
  {
    if (QEcalRec::instance()->recordersRecording())
      return (QAbstractItemModel::flags(index) | Qt::ItemFlag::ItemIsUserCheckable) & ~Qt::ItemFlag::ItemIsEnabled;
    else
      return QAbstractItemModel::flags(index) | Qt::ItemFlag::ItemIsUserCheckable;
  }
  else if ((index.column() == (int)Columns::HOST_FILTER)
    && (recorder_list_[index.row()].recording_enabled_)
    && !QEcalRec::instance()->recordersRecording())
  {
    return QAbstractItemModel::flags(index) | Qt::ItemFlag::ItemIsEditable;
  }
  else
  {
    return QAbstractItemModel::flags(index);
  }
}

QVariant RecorderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((role == Qt::ItemDataRole::DisplayRole)
    && (orientation == Qt::Orientation::Horizontal))
  {
    return column_labels_.at((Columns)section);
  }
  return QVariant::Invalid;
}

QModelIndex RecorderModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QModelIndex RecorderModel::index(const std::string& hostname, int column) const
{
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    if (hostname == recorder_list_[i].hostname_)
      return index((int)i, column);
  }
  return QModelIndex();
}

QModelIndex RecorderModel::parent(const QModelIndex& /*index*/) const
{
  return QModelIndex();
}

int RecorderModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    // the invisible root node has children
    return (int)recorder_list_.size();
  }

  // The entries do not have children
  return 0;
}

int RecorderModel::columnCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    // the invisible root node has children
    return (int)Columns::COLUMN_COUNT;
  }

  // The entries do not have children
  return 0;
}

//////////////////////////////////////////////////////
// Setter & Getter
//////////////////////////////////////////////////////

std::vector<std::pair<std::string, std::set<std::string>>> RecorderModel::recorderInstances() const
{
  std::vector<std::pair<std::string, std::set<std::string>>> recorder_instances;
  recorder_instances.reserve(recorder_list_.size());

  for (const auto& recorder : recorder_list_)
  {
    if (recorder.recording_enabled_)
    {
      recorder_instances.emplace_back(std::make_pair(recorder.hostname_, recorder.host_filter_));
    }
  }

  return recorder_instances;
}

void RecorderModel::setRecorderInstances(const std::vector<std::pair<std::string, std::set<std::string>>>& host_hostfilter_pairs)
{
  // Update existing entries
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    auto host_hostfilter_it = std::find_if(host_hostfilter_pairs.begin(), host_hostfilter_pairs.end(),
      [this, i](const auto& host_hostfilter_pair) { return host_hostfilter_pair.first == recorder_list_[i].hostname_; });

    if (host_hostfilter_it == host_hostfilter_pairs.end())
    {
      // De-activate recorder
      if (recorder_list_[i].recording_enabled_)
      {
        recorder_list_[i].recording_enabled_          = false;
        recorder_list_[i].double_host_filter_warning_ = false;
        //recorder_list_[i].host_filter_       = {};
        //recorder_list_[i].running_           = false;
        //recorder_list_[i].request_pending_   = false;
        //recorder_list_[i].state_             = eCAL::rec::RecorderState();
        //recorder_list_[i].last_response_     = {true, ""};

        emitDataChangedAll((int)i);
      }
    }
    else
    {
      // activate recorder
      if (!recorder_list_[i].recording_enabled_)
      {
        recorder_list_[i].recording_enabled_ = true;
        recorder_list_[i].host_filter_       = host_hostfilter_it->second;

        emitDataChangedAll((int)i);
      }
      else if (recorder_list_[i].host_filter_ != host_hostfilter_it->second)
      {
        recorder_list_[i].host_filter_ = host_hostfilter_it->second;
        emitDataChangedColumn((int)i, Columns::HOST_FILTER);
      }
    }
  }

  // Add new entries
  {
    for (const auto& host_hostfilter_pair : host_hostfilter_pairs)
    {
      auto existing_rec_it = std::find_if(recorder_list_.begin(), recorder_list_.end(),
        [&host_hostfilter_pair](const auto& existing_rec) { return host_hostfilter_pair.first == existing_rec.hostname_; });

      if (existing_rec_it == recorder_list_.end())
      {
        RecorderEntry new_entry;
        new_entry.hostname_          = host_hostfilter_pair.first;
        new_entry.host_filter_       = host_hostfilter_pair.second;
        new_entry.recording_enabled_ = true;

        beginInsertRows(QModelIndex(), (int)recorder_list_.size(), (int)recorder_list_.size());
        recorder_list_.push_back(new_entry);
        endInsertRows();
      }
    }
  }

   // Update the host filter warnings
  updateHostFilterWarnings();
}

void RecorderModel::setHostFilter(const std::string& hostname, const std::set<std::string>& host_filter)
{
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    if ((recorder_list_[i].hostname_ == hostname)
      && (recorder_list_[i].host_filter_ != host_filter))
    {
      recorder_list_[i].host_filter_ = host_filter;
      emitDataChangedColumn((int)i, Columns::HOST_FILTER);
      break;
    }
  }

  // Update the host filter warnings
  updateHostFilterWarnings();
}

void RecorderModel::setAllChecked(bool checked)
{
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    if (recorder_list_[i].recording_enabled_ != checked)
    {
      recorder_list_[i].recording_enabled_ = checked;
      emitDataChangedAll((int)i);
    }
  }

  // Update the host filter warnings
  updateHostFilterWarnings();
}

void RecorderModel::invertSelection()
{
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    recorder_list_[i].recording_enabled_ = !recorder_list_[i].recording_enabled_;
  }

  // Update the host filter warnings
  updateHostFilterWarnings(false);

  // Update the view of the entire model
  emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

bool RecorderModel::isHostRemovable(const QModelIndex& row) const
{
  if (!row.isValid())
    return false;

  return isHostRemovable(row.row());
}

bool RecorderModel::isHostRemovable(int row) const
{
  return !recorder_list_[row].ecal_rec_visible_ && !recorder_list_[row].recording_enabled_;
}

bool RecorderModel::containsHost(const std::string& hostname) const
{
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    if (recorder_list_[i].hostname_ == hostname)
      return true;
  }
  return false;
}

bool RecorderModel::addHost(const std::string& hostname)
{
  if (containsHost(hostname))
  {
    return false;
  }
  else
  {
    beginInsertRows(QModelIndex(), (int)recorder_list_.size(), (int)recorder_list_.size());
    RecorderEntry new_host_entry;
    new_host_entry.hostname_ = hostname;
    recorder_list_.push_back(new_host_entry);
    endInsertRows();

    return true;
  }
}

bool RecorderModel::removeRow(int row)
{
  if (isHostRemovable(row))
  {
    beginRemoveRows(QModelIndex(), row, row);
    recorder_list_.erase(std::next(recorder_list_.begin(), row));
    endRemoveRows();
    return true;
  }
  else
  {
    return false;
  }
}

//////////////////////////////////////////////////////
// External control
//////////////////////////////////////////////////////

void RecorderModel::stateUpdate(std::map<std::string, eCAL::rec::RecorderState> recorder_states)
{
  auto pending_requests = QEcalRec::instance()->areRequestsPending();
  auto last_responses   = QEcalRec::instance()->lastResponses();

  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    bool update_view = false;

    // request pending
    {
      auto pending_request_it = pending_requests.find(recorder_list_[i].hostname_);
      bool is_request_pending = ((pending_request_it != pending_requests.end()) && pending_request_it->second);
      if (recorder_list_[i].request_pending_ != is_request_pending)
      {
        recorder_list_[i].request_pending_ = is_request_pending;
        update_view = true;
      }
    }

    // Error
    {
      auto last_response_it = last_responses.find(recorder_list_[i].hostname_);
      std::pair<bool, std::string> last_response;

      if (last_response_it != last_responses.end())
        last_response = last_response_it->second;
      else
        last_response = { true, "" };

      if (recorder_list_[i].last_response_ != last_response)
      {
        recorder_list_[i].last_response_ = last_response;
        update_view = true;
      }
    }

    // state
    auto recorder_state_it = recorder_states.find(recorder_list_[i].hostname_);
    bool rec_connected = (recorder_state_it != recorder_states.end());

    if (!rec_connected)
    {
      if (recorder_list_[i].connected_to_client_)
      {
        recorder_list_[i].connected_to_client_ = false;
        recorder_list_[i].state_               = eCAL::rec::RecorderState();
        update_view = true;
      }
    }
    else
    {
      if (!recorder_list_[i].connected_to_client_)
      {
        recorder_list_[i].connected_to_client_ = true;
        update_view = true;
      }

      if (recorder_list_[i].state_ != recorder_state_it->second)
      {
        recorder_list_[i].state_ = recorder_state_it->second;
        update_view = true;
      }
    }

    if (update_view)
    {
      emitDataChangedState((int)i);
    }
  }
}

void RecorderModel::monitorUpdated()
{
  auto hosts_running_ecal_rec = QEcalRec::instance()->hostsRunningEcalRec();

  // Update existing hosts and add new ones
  for (const auto& host : hosts_running_ecal_rec)
  {
    bool recorder_existed = false;

    // Check if we already have an element with that hostname
    for (size_t i = 0; i < recorder_list_.size(); i++)
    {
      if (recorder_list_[i].hostname_ == host.first)
      {
        recorder_existed = true;

        // We need to update the view if any old value does not match
        bool update_view = (recorder_list_[i].ecal_rec_visible_ != host.second);
        recorder_list_[i].ecal_rec_visible_ = host.second;

        if (update_view)
        {
          emitDataChangedAll((int)i);
        }
      }
    }

    // Host not found => add a new one if it has a running eCALRec instance
    if (!recorder_existed && host.second)
    {
      RecorderEntry new_entry;
      new_entry.hostname_         = host.first;
      new_entry.ecal_rec_visible_ = host.second;

      beginInsertRows(QModelIndex(), (int)recorder_list_.size(), (int)recorder_list_.size());
      recorder_list_.push_back(new_entry);
      endInsertRows();
    }
  }

  // Update non-existing hosts
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    auto visible_host_it = hosts_running_ecal_rec.find(recorder_list_[i].hostname_);

    if ((visible_host_it == hosts_running_ecal_rec.end()) || !visible_host_it->second)
    {
      bool update_view = (recorder_list_[i].ecal_rec_visible_);

      recorder_list_[i].ecal_rec_visible_  = false;

      if (update_view)
      {
        emitDataChangedAll((int)i);
      }
    }
  }
}

void RecorderModel::recordingStateChanged(bool /*recording*/)
{
  emit dataChanged(index(0, (int)Columns::ENABLED), index(rowCount(), (int)Columns::ENABLED));
  emit dataChanged(index(0, (int)Columns::HOST_FILTER), index(rowCount(), (int)Columns::HOST_FILTER));
}

//////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////

void RecorderModel::updateHostFilterWarnings(bool update_view)
{
  int hosts_count_recording_all_hosts = 0;
  int hosts_count_recording_specific_hosts = 0;

  // Count hosts recording all hosts
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    if (recorder_list_[i].recording_enabled_)
    {
      if (recorder_list_[i].host_filter_.size() == 0)
        hosts_count_recording_all_hosts++;
      else
        hosts_count_recording_specific_hosts++;
    }
    else
    {
      // While we are iterating over all hosts we also set the warning to false on all hosts that are not enabled
      recorder_list_[i].double_host_filter_warning_ = false;
    }
  }

  if ((hosts_count_recording_all_hosts >= 1)
    && ((hosts_count_recording_all_hosts + hosts_count_recording_specific_hosts) > 1))
  {
    // All recorders get a warning (there is at least one hosts recording everything and at least one host recording anything)
    for (size_t i = 0; i < recorder_list_.size(); i++)
    {
      if (recorder_list_[i].recording_enabled_)
      {
        recorder_list_[i].double_host_filter_warning_ = true;
      }
    }
  }
  else if (hosts_count_recording_all_hosts == 1)
  {
    // That one host that recordes everything does not get a warning
    for (size_t i = 0; i < recorder_list_.size(); i++)
    {
      if (recorder_list_[i].host_filter_.size() == 0)
      {
        recorder_list_[i].double_host_filter_warning_ = false;
      }
    }
  }
  else
  {
    // we need to create a map and actually check which host records what
    std::map<std::string, std::set<size_t>> specific_hosts_from_hostfilters;
    for (size_t i = 0; i < recorder_list_.size(); i++)
    {
      if (recorder_list_[i].recording_enabled_)
      {
        bool any_double_host_found = false;
        for (const std::string& host : recorder_list_[i].host_filter_)
        {
          // Check if anybody else records that host
          auto host_recorder_pair_it = specific_hosts_from_hostfilters.find(host);
          if (host_recorder_pair_it == specific_hosts_from_hostfilters.end())
          {
            // Nobody records that host, yet
            specific_hosts_from_hostfilters.emplace(host, std::set<size_t>{i});
          }
          else
          {
            // Someone else records that host
            any_double_host_found = true;
            host_recorder_pair_it->second.emplace(i);
            for (size_t index : host_recorder_pair_it->second)
            {
              // also set the warning of the other host that recorded this host previously
              recorder_list_[index].double_host_filter_warning_ = true;
            }
          }
        }
        recorder_list_[i].double_host_filter_warning_ = any_double_host_found;
      }
    }
  }

  // Update the view
  if (update_view)
  {
    emit dataChanged(index(0, (int)Columns::HOST_FILTER), index(rowCount() - 1, (int)Columns::HOST_FILTER), { Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::ToolTipRole });
  }
}

void RecorderModel::emitDataChangedAll(int row)
{
  emit dataChanged(index(row, 0), index(row, (int)Columns::COLUMN_COUNT - 1));
}

void RecorderModel::emitDataChangedColumn(int row, Columns column)
{
  emit dataChanged(index(row, (int)column), index(row, (int)column));
}

void RecorderModel::emitDataChangedConfig(int row)
{
  emit dataChanged(index(row, (int)Columns::ENABLED),     index(row, (int)Columns::ENABLED));
  emit dataChanged(index(row, (int)Columns::HOST_FILTER), index(row, (int)Columns::HOST_FILTER));

}

void RecorderModel::emitDataChangedState(int row)
{
  emit dataChanged(index(row, (int)Columns::HOSTNAME), index(row, (int)Columns::HOSTNAME));
  emit dataChanged(index(row, (int)Columns::STATE),    index(row, (int)Columns::COLUMN_COUNT - 1));
}
