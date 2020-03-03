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
#include <QSortFilterProxyModel>

/**
 * @brief The QStableSortFilterProxyModel provides a deterministic sort behaviour (compared to the @see{QSortFilterProxyModel})
 *
 * When using a @see{QSortFilterProxyModel}, two equal items may constantly
 * switch positions when the Proxy Model re-sorts the elements. This class fixes
 * that "bug" by using the source model's order as tie breaker.
 *
 * This class also provides a recursive filtering backport for Qt versions prior
 * to Qt 5.10.
 */
class QStableSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  QStableSortFilterProxyModel(QObject* parent = 0);
  ~QStableSortFilterProxyModel();

  bool lessThan(const QModelIndex &rLeft, const QModelIndex &rRight) const override;

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

  virtual bool filterDirectAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  // The recursiveFilteringEnabled property was added in Qt 5.10, so we need to
  // backport it for older Qt Versions, especially for Ubuntu 16.04, which uses
  // an ancient Qt 5.5
public:
  /**
   * @brief Sets whether recursive filtering shall be enabled.
   *
   * If enabled, the filter will be applied recursively on children, and for any
   * matching child, its parents will be visible as well.
   * By default, recursive filtering is turned off.
   *
   * @param recursive Whether recursive filtering shall be enabled
   */
  void setRecursiveFilteringEnabled(bool recursive);

  /**
   * @brief Returns whether recursive filtering is enabled.
   *
   * If enabled, the filter will be applied recursively on children, and for any
   * matching child, its parents will be visible as well.
   */
  bool isRecursiveFilteringEnabled() const;

  void setSourceModel(QAbstractItemModel *source_model) override;

private slots:
  void invalidateFilterIfRecursiveFilteringEnabled();

private:
  bool recursive_filtering_enabled;                                             /**< Whether recursive filtering is enabled */
#endif //QT_VERSION
};

