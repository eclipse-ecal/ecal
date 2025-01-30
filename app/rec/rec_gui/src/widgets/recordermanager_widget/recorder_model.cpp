/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  #include <QDesktopWidget>
#endif // QT_VERSION < QT_VERSION_CHECK(5, 15, 0)

#include <QDesktopServices>
#include <QApplication>
#include <QStyleOption>
#include <QBrush>
#include <QVariant>
#include <QDateTime>

#include "qecalrec.h"
#include <algorithm>
#include <chrono>

#include <ecal/ecal.h>
#include <ecal_utils/string.h>
#include "models/item_data_roles.h"

//////////////////////////////////////////////////////
// Constructor & destructor
//////////////////////////////////////////////////////

RecorderModel::RecorderModel(QObject *parent)
  : QAbstractItemModel(parent)
  , icon_client_connected        (":/ecalicons/HOST_CONNECTED")
  , icon_client_connected_warning(":/ecalicons/HOST_CONNECTED_WARNING")
  , icon_client_connected_error  (":/ecalicons/HOST_CONNECTED_ERROR")
  , icon_client_disconnected     (":/ecalicons/HOST_DISCONNECTED")
  , icon_addon_connected         (":/ecalicons/ADDON_CONNECTED")
  , icon_addon_connected_warning (":/ecalicons/ADDON_CONNECTED_WARNING")
  , icon_addon_connected_error   (":/ecalicons/ADDON_CONNECTED_ERROR")
  , icon_addon_disconnected      (":/ecalicons/ADDON_DISCONNECTED")
  , icon_waiting                 (":/ecalicons/WAIT")
  , icon_connected               (":/ecalicons/CHECKMARK")
  , icon_disconnected            (":/ecalicons/CROSS")
  , icon_recording               (":/ecalicons/RECORD")
  , icon_flushing                (":/ecalicons/SAVE_TO_DISK")
  , icon_error                   (":/ecalicons/ERROR")
  , icon_warning                 (":/ecalicons/WARNING")
{
 connect(QEcalRec::instance(), &QEcalRec::recorderStatusUpdateSignal,     this,
        [this](const eCAL::rec_server::RecorderStatusMap_T recorder_statuses, const std::list<eCAL::rec_server::JobHistoryEntry>& /*job_history*/)
        {
          recorderStatusUpdate(recorder_statuses);
        });

 connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal,           this, &RecorderModel::monitorUpdated);

 connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal, this, &RecorderModel::setEnabledRecClients);
 //connect(QEcalRec::instance(), &QEcalRec::hostFilterChangedSignal,        this, &RecorderModel::setHostFilter);

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
 setEnabledRecClients(QEcalRec::instance()->enabledRecClients());
 recordingStateChanged(QEcalRec::instance()->recording());
}

RecorderModel::~RecorderModel()
{}

//////////////////////////////////////////////////////
// Reimplemented from QAbstractItemModel
//////////////////////////////////////////////////////

QVariant RecorderModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant(); // Invalid QVariant

  const int     row    = index.row();
  const Columns column = (Columns)index.column();

  if (role == Qt::ItemDataRole::FontRole)
  {
    if (!recorder_list_[row].ecal_rec_visible_in_monitor_
      && (!QEcalRec::instance()->usingBuiltInRecorderEnabled() || (recorder_list_[row].hostname_ != eCAL::Process::GetHostName())))
    {
      QFont font;
      font.setItalic(true);
      return font;
    }
  }

  if (role == Qt::ItemDataRole::ForegroundRole)
  {
    if (!recorder_list_[row].recording_enabled_)
    {
      return QColor(150, 150, 150);
    }
  }

  if (column == Columns::ENABLED)
  {
    if (role == Qt::ItemDataRole::CheckStateRole)
    {
      return (recorder_list_[row].recording_enabled_ ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (recorder_list_[row].recording_enabled_
        && !recorder_list_[row].ecal_rec_visible_in_monitor_
        && (!QEcalRec::instance()->usingBuiltInRecorderEnabled() || (recorder_list_[row].hostname_ != eCAL::Process::GetHostName())))
      {
        return icon_warning;
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      if (recorder_list_[row].recording_enabled_
        && !recorder_list_[row].ecal_rec_visible_in_monitor_
        && (!QEcalRec::instance()->usingBuiltInRecorderEnabled() || (recorder_list_[row].hostname_ != eCAL::Process::GetHostName())))
      {
        return tr("eCALRec is not running");
      }
    }
    else if (role == ItemDataRoles::SortRole)
    {
      return (recorder_list_[row].recording_enabled_ ? 1 : 0);
    }
  }

  else if (column == Columns::HOSTNAME)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      if (recorder_list_[row].addon_id_.empty())
      {
        // Normal recorder
        return QString::fromStdString(recorder_list_[row].hostname_);
      }
      else
      {
        // Addon
        if (recorder_list_[row].addon_name_.empty())
        {
          return QString::fromStdString(recorder_list_[row].hostname_) + " (" + QString::fromStdString(recorder_list_[row].addon_id_) + ")";
        }
        else
        {
          return QString::fromStdString(recorder_list_[row].hostname_) + " (" + QString::fromStdString(recorder_list_[row].addon_name_) + ")";
        }
      }
    }
    if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        if (recorder_list_[row].request_pending_)
        {
          return icon_waiting;
        }
        else if (recorder_list_[row].connection_established_)
        {
          if (recorder_list_[row].addon_id_.empty())
          {
            if(!recorder_list_[row].info_.first)
            {
              return icon_client_connected_error;
            }
            else if (recorder_list_[row].time_error_warning_ || recorder_list_[row].double_host_filter_warning_)
            {
              return icon_client_connected_warning;
            }
            else
            {
              return icon_client_connected;
            }
          }
          else
          {
            if (!recorder_list_[row].info_.first)
            {
              return icon_addon_connected_error;
            }
            else
            {
              return icon_addon_connected;
            }
          }
        }
        else
        {
          if (recorder_list_[row].addon_id_.empty())
            return icon_client_disconnected;
          else
            return icon_addon_disconnected;
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
        else if (recorder_list_[row].connection_established_)
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

  else if (column == Columns::ADDON_ID)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return QString::fromStdString(recorder_list_[row].addon_id_);
    }
  }

  else if (column == Columns::ADDON_EXECUTABLE_PATH)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return QString::fromStdString(recorder_list_[row].addon_execuable_path_);
    }
  }

  else if (column == Columns::PROCESS_ID)
  {
    if ((role == Qt::ItemDataRole::DisplayRole)
      && (recorder_list_[row].recording_enabled_)
      && (recorder_list_[row].pid_ > 0))
    {
      return recorder_list_[row].pid_;
    }
  }

  else if (column == Columns::HOST_FILTER)
  {
    if (recorder_list_[row].addon_id_.empty())
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
  }

  else if (column == Columns::STATE)
  {
    if (recorder_list_[row].addon_id_.empty())
    {
      if (recorder_list_[row].recording_enabled_)
      {
        if (!recorder_list_[row].connection_established_)
        {
          if (role == Qt::ItemDataRole::DisplayRole)
          {
            return "Offline";
          }
        }
        else if (recorder_list_[row].recording_)
        {
            if (role == Qt::ItemDataRole::DisplayRole)
            {
              return tr("Recording");
            }
            else if (role == Qt::ItemDataRole::DecorationRole)
            {
              return icon_recording;
            }
        }
        else if (recorder_list_[row].initialized_)
        {
          if (role == Qt::ItemDataRole::DisplayRole)
          {
            return tr("Subscribing (") + QString::number(recorder_list_[row].subscribed_topics_.size()) + tr(" Topics)");
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
          if (!recorder_list_[row].connection_established_)
          {
            tooltip += "Offline";
          }
          else
          {
            tooltip += "Online";

            if (!recorder_list_[row].initialized_)
            {
              tooltip += "\nDisconnected from eCAL";
            }
            else
            {
              tooltip += "\nSubscribed to " + QString::number(recorder_list_[row].subscribed_topics_.size()) + " Topics";
            }
          }

          return tooltip;
        }
      }
    }
    else
    {
      if (recorder_list_[row].recording_enabled_)
      {
        if (!recorder_list_[row].connection_established_)
        {
          if (role == Qt::ItemDataRole::DisplayRole)
          {
            return "Offline";
          }
        }
        else if (recorder_list_[row].recording_)
        {
          if (role == Qt::ItemDataRole::DisplayRole)
          {
            return tr("Recording");
          }
          else if (role == Qt::ItemDataRole::DecorationRole)
          {
            return icon_recording;
          }
        }
        else if (recorder_list_[row].initialized_)
        {
          if (role == Qt::ItemDataRole::DisplayRole)
          {
            return tr("Initialized");
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
            return tr("Not initialized");
          }
          else if (role == Qt::ItemDataRole::DecorationRole)
          {
            return icon_disconnected;
          }
        }

      }
    }
  }

  else if (column == Columns::BUFFER)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        if (recorder_list_[row].connection_established_)
        {
          if (recorder_list_[row].addon_id_.empty())
          {
            return QString::number(std::chrono::duration_cast<std::chrono::duration<double>>(recorder_list_[row].pre_buffer_length_.second).count(), 'f', 1) + " s / "
              + QString::number(recorder_list_[row].pre_buffer_length_.first) + " frames";
          }
          else
          {
            return QString::number(recorder_list_[row].pre_buffer_length_.first) + " frames";
          }
        }
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      //if (recorder_list_[row].state_.buffer_writers_.size() > 0)
      //{
      //  size_t frames_to_write = 0;
      //  for (const auto& writer_state : recorder_list_[row].state_.buffer_writers_)
      //  {
      //    frames_to_write += writer_state.recording_queue_.first;
      //  }
      //  QString tooltip = (recorder_list_[row].state_.buffer_writers_.size() > 1
      //    ? "Saving " + QString::number(recorder_list_[row].state_.buffer_writers_.size()) + " buffers ("
      //    : "Saving 1 buffer (");
      //  tooltip += (QString::number(frames_to_write) + " frames left)");
      //  return tooltip;
      //}
    }
    else if (role == ItemDataRoles::SortRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        return (qint64)recorder_list_[row].pre_buffer_length_.first;
      }
      else
      {
        return qint64(0);
      }
    }
  }

  else if ((column == Columns::TIMESTAMP) || (column == Columns::TIME_ERROR))
  {
    if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (recorder_list_[row].recording_enabled_ && recorder_list_[row].time_error_warning_)
      {
        return icon_warning;
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      if (recorder_list_[row].recording_enabled_ && recorder_list_[row].time_error_warning_)
      {
        return "One of the recorders appears to be out of sync!";
      }
    }

    if (column == Columns::TIMESTAMP)
    {
      if (recorder_list_[row].recording_enabled_ && recorder_list_[row].addon_id_.empty())
      {
        if (role == Qt::ItemDataRole::DisplayRole)
        {
          QDateTime time = QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(recorder_list_[row].timestamp_.time_since_epoch()).count(), Qt::TimeSpec::UTC);
          return time.toString("yyyy-MM-dd hh:mm:ss.zzz");
        }
        else if (role == ItemDataRoles::SortRole)
        {
          return static_cast<qlonglong>(std::chrono::duration_cast<std::chrono::nanoseconds>(recorder_list_[row].timestamp_.time_since_epoch()).count());
        }
      }
    }
    else if (column == Columns::TIME_ERROR)
    {
      if (recorder_list_[row].recording_enabled_ && recorder_list_[row].addon_id_.empty())
      {
        if (role == Qt::ItemDataRole::DisplayRole)
        {
          double time_error_seonds = std::chrono::duration_cast<std::chrono::duration<double>>(recorder_list_[row].time_error_).count();
          return QString::number(time_error_seonds, 'f', 3) + " s";
        }
        else if (role == ItemDataRoles::SortRole)
        {
          return static_cast<qlonglong>(std::chrono::duration_cast<std::chrono::nanoseconds>(recorder_list_[row].time_error_).count());
        }
      }
    }
  }

  else if (column == Columns::INFO)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        return recorder_list_[row].info_.second.c_str();
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        return data(index, Qt::ItemDataRole::DisplayRole);
      }
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (recorder_list_[row].recording_enabled_)
      {
        if (!recorder_list_[row].info_.first)
        {
          return icon_error;
        }
      }
    }
  }

  if ((role == ItemDataRoles::SortRole)
    || (role == ItemDataRoles::FilterRole)
    || (role == Qt::ItemDataRole::ToolTipRole))
  {
    // If the column has not explicitely defined those roles, we fall back to the display role.
    return data(index, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant(); // Invalid QVariant
}

bool RecorderModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || QEcalRec::instance()->recording())
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
    bool enabled = ((Qt::CheckState)value.toInt() == Qt::CheckState::Checked);

    std::vector<int> rows_to_update;
    rows_to_update.reserve(2);

    if (enabled)
    {
      // Enabled the element itself
      if (!recorder_list_[index.row()].recording_enabled_)
      {
        recorder_list_[index.row()].recording_enabled_ = true;
        rows_to_update.push_back(index.row());
      }

      // Also enable the host element, if this element is an addon
      if (!recorder_list_[index.row()].addon_id_.empty())
      {
        int host_row = rowOfHost(recorder_list_[index.row()].hostname_);
        if ((host_row >= 0)
          && (!recorder_list_[host_row].recording_enabled_))
        {
          recorder_list_[host_row].recording_enabled_ = true;
          rows_to_update.push_back(host_row);
        }
      }
    }
    else
    {
      // Disable the element itself
      if (recorder_list_[index.row()].recording_enabled_)
      {
        recorder_list_[index.row()].recording_enabled_ = false;
        rows_to_update.push_back(index.row());
      }

      // Also disable all addons, if this element is a host element
      if (recorder_list_[index.row()].addon_id_.empty())
      {
        for (int i = 0; i < (int)recorder_list_.size(); i++)
        {
          if ((recorder_list_[i].recording_enabled_)
            && (!recorder_list_[i].addon_id_.empty())
            && (recorder_list_[i].hostname_ == recorder_list_[index.row()].hostname_))
          {
            recorder_list_[i].recording_enabled_ = false;
            rows_to_update.push_back(i);
          }
        }
      }
    }

    bool success = QEcalRec::instance()->setEnabledRecClients(enabledRecClients());

    if (success)
    {
      // If we are not successfull, QEcalRec will send us the original enabledRecClients object again and we will automatically change it back.
      for (int i : rows_to_update)
      {
        dataChanged(this->index(i, 0), this->index(i, (int)Columns::COLUMN_COUNT - 1));
      }
      updateHostFilterWarnings();
    }
    return true;
  }

  return false;
}

Qt::ItemFlags RecorderModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return QAbstractItemModel::flags(index);

  if (index.column() == (int)Columns::ENABLED)
  {
    if (QEcalRec::instance()->recording())
    {
      return (QAbstractItemModel::flags(index) | Qt::ItemFlag::ItemIsUserCheckable) & ~Qt::ItemFlag::ItemIsEnabled;
    }
    else
    {
      return QAbstractItemModel::flags(index) | Qt::ItemFlag::ItemIsUserCheckable;
    }
  }
  else if ((index.column() == (int)Columns::HOST_FILTER)
    && (recorder_list_[index.row()].recording_enabled_)
    && (recorder_list_[index.row()].addon_id_.empty())
    && !QEcalRec::instance()->recording())
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
  return QVariant(); // Invalid QVariant
}

QModelIndex RecorderModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QModelIndex RecorderModel::index(const std::string& hostname, const std::string& addon_id, int column) const
{
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    if ((hostname == recorder_list_[i].hostname_)
      && (addon_id == recorder_list_[i].addon_id_))
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

std::map<std::string, eCAL::rec_server::ClientConfig> RecorderModel::enabledRecClients() const
{
  std::map<std::string, eCAL::rec_server::ClientConfig> enabled_clients;

  // Add Main recorder clients
  for (const auto& recorder : recorder_list_)
  {
    if (recorder.recording_enabled_ && recorder.addon_id_.empty())
    {
      eCAL::rec_server::ClientConfig client_settings;
      client_settings.host_filter_ = recorder.host_filter_;
      enabled_clients.emplace(recorder.hostname_, std::move(client_settings));
    }
  }

  // Add Addons
  for (const auto& recorder : recorder_list_)
  {
    if (recorder.recording_enabled_ && !recorder.addon_id_.empty())
    {
      auto client_settings_it = enabled_clients.find(recorder.hostname_);
      if (client_settings_it != enabled_clients.end())
      {
        client_settings_it->second.enabled_addons_.emplace(recorder.addon_id_);
      }
    }
  }

  return enabled_clients;
}

void RecorderModel::setEnabledRecClients(const std::map<std::string, eCAL::rec_server::ClientConfig>& enabled_recorders)
{
  // Update existing entries
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    auto hostname_clientsettings_it = enabled_recorders.find(recorder_list_[i].hostname_);

    if (hostname_clientsettings_it == enabled_recorders.end())
    {
      // De-activate recorder / addon (doesn't matter, if the host does not exist, we de-activate it)
      if (recorder_list_[i].recording_enabled_)
      {
        recorder_list_[i].recording_enabled_ = false;
        recorder_list_[i].double_host_filter_warning_ = false;

        emitDataChangedAll((int)i);
      }
    }
    else
    {
      // Update the element (We need to differentiate between addons and recorders)

      if (recorder_list_[i].addon_id_.empty())
      {
        // The entry is NOT an addon
        if (!recorder_list_[i].recording_enabled_)
        {
          // activate main recorder
          recorder_list_[i].recording_enabled_ = true;
          recorder_list_[i].host_filter_       = hostname_clientsettings_it->second.host_filter_;

          emitDataChangedAll((int)i);
        }
        else if (recorder_list_[i].host_filter_ != hostname_clientsettings_it->second.host_filter_)
        {
          // Update host-filter
          recorder_list_[i].host_filter_ = hostname_clientsettings_it->second.host_filter_;
          emitDataChangedColumn((int)i, Columns::HOST_FILTER);
        }
      }
      else
      {
        // The entry is an addon!
        auto addon_it = hostname_clientsettings_it->second.enabled_addons_.find(recorder_list_[i].addon_id_);

        if (addon_it == hostname_clientsettings_it->second.enabled_addons_.end())
        {
          // De-activate the addon
          if (recorder_list_[i].recording_enabled_)
          {
            recorder_list_[i].recording_enabled_ = false;
            emitDataChangedAll((int)i);
          }
        }
        else
        {
          // Activate the addon
          if (!recorder_list_[i].recording_enabled_)
          {
            recorder_list_[i].recording_enabled_ = true;
            emitDataChangedAll((int)i);
          }
        }
      }
    }

  }

  // Add new entries
  for (const auto& hostname_clientsettings_pair : enabled_recorders)
  {
    // Check for main recorder element
    auto existing_rec_it = std::find_if(recorder_list_.begin(), recorder_list_.end(),
                                        [&hostname_clientsettings_pair](const auto& existing_rec)
                                        {
                                          return (hostname_clientsettings_pair.first == existing_rec.hostname_)
                                            && (existing_rec.addon_id_.empty());
                                        });

    if (existing_rec_it == recorder_list_.end())
    {
      RecorderEntry new_entry;
      new_entry.hostname_          = hostname_clientsettings_pair.first;
      new_entry.host_filter_       = hostname_clientsettings_pair.second.host_filter_;
      new_entry.recording_enabled_ = true;

      beginInsertRows(QModelIndex(), (int)recorder_list_.size(), (int)recorder_list_.size());
      recorder_list_.push_back(std::move(new_entry));
      endInsertRows();
    }

    // Check all Addons
    for (const std::string& addon_id : hostname_clientsettings_pair.second.enabled_addons_)
    {
      auto existing_addon_it = std::find_if(recorder_list_.begin(), recorder_list_.end(),
                                          [&hostname_clientsettings_pair, &addon_id](const auto& existing_rec)
                                          {
                                            return (hostname_clientsettings_pair.first == existing_rec.hostname_)
                                              && (existing_rec.addon_id_ == addon_id);
                                          });

      if (existing_addon_it == recorder_list_.end())
      {
        RecorderEntry new_entry;
        new_entry.hostname_          = hostname_clientsettings_pair.first;
        new_entry.addon_id_          = addon_id;
        new_entry.recording_enabled_ = true;

        beginInsertRows(QModelIndex(), (int)recorder_list_.size(), (int)recorder_list_.size());
        recorder_list_.push_back(std::move(new_entry));
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
      && (recorder_list_[i].addon_id_.empty())
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

QStringList RecorderModel::getAllHosts() const
{
  QStringList all_hosts;
  for (const auto& recorder : recorder_list_)
  {
    if (recorder.addon_id_.empty())
    {
      all_hosts.push_back(QString::fromStdString(recorder.hostname_));
    }
  }

  all_hosts.removeDuplicates();
  all_hosts.sort(Qt::CaseSensitivity::CaseInsensitive);
  
  return all_hosts;
}

std::string RecorderModel::getHostname(int row) const { return recorder_list_[row].hostname_; }

//////////////////////////////////////////////////////
// Delete and add rows
//////////////////////////////////////////////////////

bool RecorderModel::isRowRemovable(int row) const
{
  return !recorder_list_[row].ecal_rec_visible_in_monitor_ && !recorder_list_[row].connection_established_;
}

std::set<int> RecorderModel::getRowsThatWillBeRemoved(const std::set<int>& rows_to_remove)
{
  std::set<int> host_rows_to_remove;
  std::set<int> addon_rows_to_remove;

  // Filter out all un-removable rows and divide them into hosts and addons
  {
    for (auto row : rows_to_remove)
    {
      if (isRowRemovable(row))
      {
        if (recorder_list_[row].addon_id_.empty())
          host_rows_to_remove.emplace(row);
        else
          addon_rows_to_remove.emplace(row);
      }

    }
  }

  // Iterate over all recorder rows and collect all addons from the hosts that will be removed
  {
    for (int i = 0; i < (int)recorder_list_.size(); i++)
    {
      if (!recorder_list_[i].addon_id_.empty())
      {
        // If the host of this addon shall be removed, this addon will be removed along with it!
        int row_of_host = rowOfHost(recorder_list_[i].hostname_);
        if (host_rows_to_remove.find(row_of_host) != host_rows_to_remove.end())
        {
          addon_rows_to_remove.emplace(i);
        }
      }
    }
  }

  // Merge the two sets again to return one uniform set
  for (const int i : addon_rows_to_remove)
    host_rows_to_remove.emplace(i);

  return host_rows_to_remove;
}

bool RecorderModel::removeRecorderRows(const std::set<int>& rows)
{
  auto rows_to_remove = getRowsThatWillBeRemoved(rows);

  if (!rows_to_remove.empty())
  {
    for (auto i = rows_to_remove.rbegin(); i != rows_to_remove.rend(); ++i)
    {
      beginRemoveRows(QModelIndex(), *i, *i);
      recorder_list_.erase(std::next(recorder_list_.begin(), *i));
      endRemoveRows();
    }
    updateHostFilterWarnings(true);

    QEcalRec::instance()->setEnabledRecClients(enabledRecClients());

    return true;
  }
  else
  {
    return false;
  }
}

bool RecorderModel::containsHost(const std::string& hostname) const
{
  return rowOfHost(hostname) >= 0;
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

int RecorderModel::rowOfHost(const std::string& hostname) const
{
  for (int i = 0; i < (int)recorder_list_.size(); i++)
  {
    if ((recorder_list_[i].hostname_ == hostname)
      && (recorder_list_[i].addon_id_.empty()))
    {
      return i;
    }
  }

  return -1;
}

bool RecorderModel::containsAddon(const std::string& hostname, const std::string& addon_id) const
{
  return rowOfAddon(hostname, addon_id) >= 0;
}

bool RecorderModel::addAddon(const std::string& hostname, const std::string& addon_id)
{
  if (containsAddon(hostname, addon_id) || addon_id.empty())
  {
    return false;
  }
  else
  {
    beginInsertRows(QModelIndex(), (int)recorder_list_.size(), (int)recorder_list_.size());
    RecorderEntry new_host_entry;
    new_host_entry.hostname_ = hostname;
    new_host_entry.addon_id_ = addon_id;
    recorder_list_.push_back(new_host_entry);
    endInsertRows();

    return true;
  }

}

int RecorderModel::rowOfAddon(const std::string& hostname, const std::string& addon_id) const
{
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    if ((recorder_list_[i].hostname_ == hostname)
      && (recorder_list_[i].addon_id_ == addon_id))
    {
      return (int)i;
    }
  }
  return -1;
}

//////////////////////////////////////////////////////
// External control
//////////////////////////////////////////////////////

void RecorderModel::recorderStatusUpdate(const eCAL::rec_server::RecorderStatusMap_T& recorder_statuses)
{
  auto hosts_with_pending_requests = QEcalRec::instance()->hostsWithPendingRequests();


  // Update Existing Recorders
  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    auto recorder_status_it = recorder_statuses.find(recorder_list_[i].hostname_);

    bool update_view = false;

    // request pending
    {
      bool request_pending = (hosts_with_pending_requests.find(recorder_list_[i].hostname_) != hosts_with_pending_requests.end());

      if (recorder_list_[i].request_pending_ != request_pending)
      {
        recorder_list_[i].request_pending_ = request_pending;
        update_view = true;
      }
    }

    {
      bool status_found_successfull(false);

      if (recorder_status_it != recorder_statuses.end())
      {
        if (recorder_list_[i].addon_id_.empty())
        {
          //-----------------------------
          // Status of Main HDF5 recorder
          //-----------------------------

          status_found_successfull = true;

          // pid_
          if (recorder_list_[i].pid_ != recorder_status_it->second.first.pid_)
          {
            recorder_list_[i].pid_ = recorder_status_it->second.first.pid_;
            update_view = true;
          }

          // connection_established_
          if (!recorder_list_[i].connection_established_)
          {
            recorder_list_[i].connection_established_ = true;
            update_view = true;
          }

          // timestamp (HDF5)
          if (recorder_list_[i].timestamp_ != recorder_status_it->second.first.timestamp_)
          {
            recorder_list_[i].timestamp_ = recorder_status_it->second.first.timestamp_;
            update_view = true;
          }

          // time_error (HDF5)
          eCAL::Time::ecal_clock::duration time_error = (recorder_status_it->second.second - recorder_status_it->second.first.timestamp_);
          if (recorder_list_[i].time_error_ != time_error)
          {
            recorder_list_[i].time_error_ = time_error;
            update_view = true;
          }

          // initialized (HDF5)
          if (recorder_list_[i].initialized_ != recorder_status_it->second.first.initialized_)
          {
            recorder_list_[i].initialized_ = recorder_status_it->second.first.initialized_;
            update_view = true;
          }

          // pre_buffer_length_ (HDF5)
          if (recorder_list_[i].pre_buffer_length_ != recorder_status_it->second.first.pre_buffer_length_)
          {
            recorder_list_[i].pre_buffer_length_ = recorder_status_it->second.first.pre_buffer_length_;
            update_view = true;
          }

          // subscribed_topics_ (HDF5)
          if (recorder_list_[i].subscribed_topics_ != recorder_status_it->second.first.subscribed_topics_)
          {
            recorder_list_[i].subscribed_topics_ = recorder_status_it->second.first.subscribed_topics_;
            update_view = true;
          }

          // recording (HDF5)
          auto currently_recording_job = std::find_if(recorder_status_it->second.first.job_statuses_.begin()
                                                    , recorder_status_it->second.first.job_statuses_.end()
                                                    , [](const eCAL::rec::JobStatus& job_status) -> bool
                                                          {
                                                            return job_status.state_ == eCAL::rec::JobState::Recording;
                                                          });

          bool this_node_is_recording = (currently_recording_job != recorder_status_it->second.first.job_statuses_.end());
          if (recorder_list_[i].recording_ != this_node_is_recording)
          {
            recorder_list_[i].recording_ = this_node_is_recording;
            update_view = true;
          }

          // Info (HDF5)
          if (recorder_list_[i].info_ != recorder_status_it->second.first.info_)
          {
            recorder_list_[i].info_ = recorder_status_it->second.first.info_;
            update_view = true;
          }
        }
        else
        {
          auto addon_status_it = std::find_if(recorder_status_it->second.first.addon_statuses_.begin()
                                            , recorder_status_it->second.first.addon_statuses_.end()
                                            , [this, i](const eCAL::rec::RecorderAddonStatus& addon_status)
                                              {
                                                return addon_status.addon_id_ == recorder_list_[i].addon_id_;
                                              });

          if (addon_status_it != recorder_status_it->second.first.addon_statuses_.end())
          {
            //-----------------------------
            // Status of Rec Addon
            //-----------------------------

            status_found_successfull = true;

            // connection_established_
            if (!recorder_list_[i].connection_established_)
            {
              recorder_list_[i].connection_established_ = true;
              update_view = true;
            }

            // initialized (Addon)
            if (recorder_list_[i].initialized_ != addon_status_it->initialized_)
            {
              recorder_list_[i].initialized_ = addon_status_it->initialized_;
              update_view = true;
            }

            // name (Addon)
            if (recorder_list_[i].addon_name_ != addon_status_it->name_)
            {
              recorder_list_[i].addon_name_ = addon_status_it->name_;
              update_view = true;
            }

            // addon_executable_path (Addon)
            if (recorder_list_[i].addon_execuable_path_ != addon_status_it->addon_executable_path_)
            {
              recorder_list_[i].addon_execuable_path_ = addon_status_it->addon_executable_path_;
              update_view = true;
            }

            // pre_buffer_length_ (Addon)
            auto pre_buffer_length = std::make_pair(addon_status_it->pre_buffer_length_frame_count_, std::chrono::steady_clock::duration(0));
            if (recorder_list_[i].pre_buffer_length_ != pre_buffer_length)
            {
              recorder_list_[i].pre_buffer_length_ = pre_buffer_length;
              update_view = true;
            }

            // recording (Addon)
            auto currently_recording_job = std::find_if(recorder_status_it->second.first.job_statuses_.begin()
                                                      , recorder_status_it->second.first.job_statuses_.end()
                                                      , [this, i](const eCAL::rec::JobStatus& job_status) -> bool
                                                            {
                                                              auto rec_addon_status = job_status.rec_addon_statuses_.find(recorder_list_[i].addon_id_);
                                                              return ((rec_addon_status != job_status.rec_addon_statuses_.end())
                                                                && (rec_addon_status->second.state_ == eCAL::rec::RecAddonJobStatus::State::Recording));
                                                            });
            bool this_node_is_recording = (currently_recording_job != recorder_status_it->second.first.job_statuses_.end());

            if (recorder_list_[i].recording_ != this_node_is_recording)
            {
              recorder_list_[i].recording_ = this_node_is_recording;
              update_view = true;
            }

            // Info (Addon)
            if (recorder_list_[i].info_ != addon_status_it->info_)
            {
              recorder_list_[i].info_ = addon_status_it->info_;
              update_view = true;
            }
          }
        }
      }

      if (!status_found_successfull)
      {
        // connection_established_
        if (recorder_list_[i].connection_established_)
        {
          recorder_list_[i].connection_established_ = false;
          update_view = true;
        }

        // initialized
        if (recorder_list_[i].initialized_)
        {
          recorder_list_[i].initialized_ = false;
          update_view = true;
        }

        // pre_buffer_length_
        if (recorder_list_[i].pre_buffer_length_ != std::make_pair(int64_t(0), std::chrono::steady_clock::duration(0)))
        {
          recorder_list_[i].pre_buffer_length_ = std::make_pair(int64_t(0), std::chrono::steady_clock::duration(0));
          update_view = true;
        }

        // subscribed_topics_
        if (!recorder_list_[i].subscribed_topics_.empty())
        {
          recorder_list_[i].subscribed_topics_.clear();
          update_view = true;
        }

        // recording_
        if (recorder_list_[i].recording_)
        {
          recorder_list_[i].recording_ = false;
          update_view = true;
        }

        // info_
        auto info = std::make_pair(false, std::string("No information available"));
        if (recorder_list_[i].info_ != info)
        {
          recorder_list_[i].info_ = info;
          update_view = true;
        }
      }
    }

    if (update_view)
    {
      emitDataChangedState((int)i);
    }
  }

  updateTimeErrorWarnings(true);

  // Add new recorder entries
  std::vector<RecorderEntry> new_entries;
  for (const auto& recorder_status_pair : recorder_statuses)
  {
    // Add the host if necessary
    {
      auto main_recorder_entry_it = std::find_if(recorder_list_.begin(), recorder_list_.end(),
                                                [&recorder_status_pair](const RecorderEntry& entry)
                                                {
                                                  return (entry.hostname_ == recorder_status_pair.first)
                                                     &&  (entry.addon_id_.empty());
                                                });
      if (main_recorder_entry_it == recorder_list_.end())
      {
        RecorderEntry new_recorder_entry;
        new_recorder_entry.ecal_rec_visible_in_monitor_ = true;
        new_recorder_entry.connection_established_      = true;
        new_recorder_entry.recording_enabled_           = false;
        new_recorder_entry.hostname_                    = recorder_status_pair.first;
        new_recorder_entry.addon_id_                    = "";
        new_recorder_entry.addon_name_                  = "";
        new_recorder_entry.host_filter_                 = {};
        new_recorder_entry.double_host_filter_warning_  = false;
        new_recorder_entry.request_pending_             = (hosts_with_pending_requests.find(recorder_status_pair.first) != hosts_with_pending_requests.end());

        new_entries.push_back(std::move(new_recorder_entry));
      }
    }

    // Add the addons if necessary
    for (const auto& addon_status : recorder_status_pair.second.first.addon_statuses_)
    {
      auto addon_entry = std::find_if(recorder_list_.begin(), recorder_list_.end(),
                                      [&recorder_status_pair, &addon_status](const RecorderEntry& entry)
                                      {
                                        return (entry.hostname_ == recorder_status_pair.first)
                                            &&  (entry.addon_id_ == addon_status.addon_id_);
                                      });

      if (addon_entry == recorder_list_.end())
      {
        RecorderEntry new_addon_entry;
        new_addon_entry.ecal_rec_visible_in_monitor_ = true;
        new_addon_entry.connection_established_      = true;
        new_addon_entry.recording_enabled_           = false;
        new_addon_entry.hostname_                    = recorder_status_pair.first;
        new_addon_entry.addon_id_                    = addon_status.addon_id_;
        new_addon_entry.addon_name_                  = addon_status.name_;
        new_addon_entry.host_filter_                 = {};
        new_addon_entry.double_host_filter_warning_  = false;
        new_addon_entry.request_pending_             = (hosts_with_pending_requests.find(recorder_status_pair.first) != hosts_with_pending_requests.end());

        new_entries.push_back(std::move(new_addon_entry));
      }
    }
  }
  
  if (!new_entries.empty())
  {
    recorder_list_.reserve(recorder_list_.size() + new_entries.size());
    
    beginInsertRows(QModelIndex(), (int)recorder_list_.size(), (int)(recorder_list_.size() + new_entries.size()) - 1);
    
    for (auto new_entry_it = new_entries.begin(); new_entry_it != new_entries.end(); ++new_entry_it)
    {
      recorder_list_.push_back(std::move(*new_entry_it));
    }

    endInsertRows();
  }  
}

void RecorderModel::monitorUpdated(const eCAL::rec_server::TopicInfoMap_T& /*topic_info_map*/, const eCAL::rec_server::HostsRunningEcalRec_T& hosts_running_ecal_rec)
{
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

        // We need to update the view if any old decoration_variant does not match
        bool update_view = (recorder_list_[i].ecal_rec_visible_in_monitor_ != host.second);
        recorder_list_[i].ecal_rec_visible_in_monitor_ = host.second;

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
      new_entry.ecal_rec_visible_in_monitor_ = host.second;

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
      bool update_view = (recorder_list_[i].ecal_rec_visible_in_monitor_);

      recorder_list_[i].ecal_rec_visible_in_monitor_  = false;

      if (update_view)
      {
        emitDataChangedAll((int)i);
      }
    }
  }
}

void RecorderModel::recordingStateChanged(bool /*recording*/)
{
  // call the function via its class becase it's a virtual function that is called directly/indirectly in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  emit dataChanged(RecorderModel::index(0, (int)Columns::ENABLED), RecorderModel::index(RecorderModel::rowCount(), (int)Columns::ENABLED));
  emit dataChanged(RecorderModel::index(0, (int)Columns::HOST_FILTER), RecorderModel::index(RecorderModel::rowCount(), (int)Columns::HOST_FILTER));
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
    if (!recorder_list_[i].addon_id_.empty())
      continue; // Skip addons

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
      if (!recorder_list_[i].addon_id_.empty())
        continue; // Skip addons

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
      if (!recorder_list_[i].addon_id_.empty())
        continue; // Skip addons

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
      if (!recorder_list_[i].addon_id_.empty())
        continue; // Skip addons

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

void RecorderModel::updateTimeErrorWarnings(bool update_view)
{
  eCAL::Time::ecal_clock::duration min_time_error(eCAL::Time::ecal_clock::duration::max());
  eCAL::Time::ecal_clock::duration max_time_error(eCAL::Time::ecal_clock::duration::min());

  for (size_t i = 0; i < recorder_list_.size(); i++)
  {
    if (recorder_list_[i].recording_enabled_ && recorder_list_[i].addon_id_.empty())
    {
      min_time_error = std::min(min_time_error, recorder_list_[i].time_error_);
      max_time_error = std::max(max_time_error, recorder_list_[i].time_error_);
    }
  }

  bool show_warning = ((max_time_error - min_time_error) > std::chrono::milliseconds(500))
                   || ((max_time_error - min_time_error) < std::chrono::milliseconds(-500));
  
  for (int i = 0; i < (int)recorder_list_.size(); i++)
  {
    if (recorder_list_[i].recording_enabled_
      && recorder_list_[i].addon_id_.empty()
      && (recorder_list_[i].time_error_warning_ != show_warning))
    {
      recorder_list_[i].time_error_warning_ = show_warning;

      if (update_view)
      {
        emit dataChanged(index(i, (int)Columns::TIMESTAMP), index(i, (int)Columns::TIME_ERROR), { Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::ToolTipRole });
      }
    }
  }
}

void RecorderModel::emitDataChangedAll(int row)
{
  emit dataChanged(RecorderModel::index(row, 0), RecorderModel::index(row, (int)Columns::COLUMN_COUNT - 1));
}

void RecorderModel::emitDataChangedColumn(int row, Columns column)
{
  emit dataChanged(RecorderModel::index(row, (int)column), RecorderModel::index(row, (int)column));
}

void RecorderModel::emitDataChangedConfig(int row)
{
  emit dataChanged(index(row, (int)Columns::ENABLED),     index(row, (int)Columns::ENABLED));
  emit dataChanged(index(row, (int)Columns::HOST_FILTER), index(row, (int)Columns::HOST_FILTER));

}

void RecorderModel::emitDataChangedState(int row)
{
  emit dataChanged(index(row, (int)Columns::HOSTNAME),  index(row, (int)Columns::PROCESS_ID));
  emit dataChanged(index(row, (int)Columns::TIMESTAMP), index(row, (int)Columns::COLUMN_COUNT - 1));
}
