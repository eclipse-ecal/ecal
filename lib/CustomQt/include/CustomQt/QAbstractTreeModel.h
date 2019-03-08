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

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "QAbstractTreeItem.h"

/**
 * @brief The QAbstractTreeModel provides an abstract tree-based impelementation of an Item model.
 *
 * Together with the QAbstractTreeItem, the QAbstractTreeModel provides a base-
 * implementation for displaying trees (or lists) in a QTreeView (or similar).
 * The QAbstractTreeModel is not meant to be instanciated directly, although it
 * is possible, as all functions provide a default implementation.
 *
 * The approach of this model is item-based, i.e. most of the implementation is
 * delegated to the TreeItems and not implemented in the TreeModel itself.
 *
 * When subclassing QAbstractTreeModel and QAbstractTreeItem, you should override
 * the following functions:
 *
 *   QAbstractTreeModel:
 *
 *      mapColumnToItem(): Return a mapping TreeModel Column -> TreeItem Column
 *                     for any type of TreeItem, that the model is supposed to
 *                     hold.
 *
 *      columnCount(): return how many columns your tree shall have
 *
 *      headerData():  Return e.g. a string for each column that will show up as
 *                     header
 *
 *   QAbstractTreeItem:
 *
 *      data(): return all required data for all required roles that this item
 *              shall implement. Your actual implementation for displaying any
 *              data should go here.
 *
 *      type(): as QModelIndex::InternalPointer() returns a void*, you should
 *              implement this function to tell the actual type of the TreeItem.
 *              This is also the value used for the mapColumnToItem() method of
 *              the TreeModel.
 */
class QAbstractTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit QAbstractTreeModel(QObject *parent = 0);
  ~QAbstractTreeModel();

  QVariant data(const QModelIndex &index, int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * @brief Returns the item pointed to by the index
   * @param index the index of the item
   * @return the item that is pointed to by the index
   */
  QAbstractTreeItem* item(const QModelIndex& index) const;

  /**
   * @brief Returns the root element of the model
   * @return the root element of the model
   */
  QAbstractTreeItem* root() const;

  /**
   * @brief Sets the root element of the model
   * 
   * The old root element and all of its children are deleted.
   */
  virtual void setRoot(QAbstractTreeItem* root);

  /**
   * @brief Inserts a list of items as children of the given parent
   *
   * @param items   The items to insert
   * @param parent  The parent for the new items
   * @param row     The row to insert the items in. The items will be inserted before the given row, so after inserting the given row will contain the first inserted item. By default, elements are inserted at the end.
   */
  virtual void insertItems(QList<QAbstractTreeItem*>& items, const QModelIndex &parent = QModelIndex(), int row = -1);
  void insertItem(QAbstractTreeItem* item, const QModelIndex &parent = QModelIndex(), int row = -1)                  { QList<QAbstractTreeItem*> item_list{item}; insertItems(item_list, parent, row); };

  /**
   * @brief Removes all given items from the model
   * @param indices The list of items that shall be removed
   */
  virtual void removeItems(const QList<QModelIndex>& indices);
  void removeItem(const QModelIndex& index)               { removeItems(QList<QModelIndex>{index}); };
  void removeItem(const QAbstractTreeItem* item)           { removeItem(index(item)); };
  void removeItems(const QList<QAbstractTreeItem*>& items) { removeItems(index(items)); };

  /**
   * @brief Removes all items from the given parent node and deletes them
   * @param index the parent item
   */
  virtual void removeAllChildren(const QModelIndex& index = QModelIndex());

  /**
   * @brief Moves an item from one row to another
   * @param from    The row to move
   * @param to      The target row
   * @param parent  The parent item
   */
  virtual void moveItem(int from, int to, const QModelIndex& parent = QModelIndex());

  /**
   * @brief Filteres an indices list by removing all elements of which a parent is also included in the list
   *
   * @param indices A list of all indices
   * @return A list of only the top-most indices
   */
  static QList<QModelIndex> reduceToTopMostIndices(const QList<QModelIndex>& indices);

  /**
   * @brief Recursively applies the predicate to all children and returns a list of children where the predicate is true.
   * @param p the predicate (QAbstractTreeItem*)->bool to check
   * @return a list of all (recursive) children for which the predicate is true
   */
  QList<QAbstractTreeItem*> findItems(std::function<bool(QAbstractTreeItem*)> p) const { return root_item_->findChildren(p); }

  /**
   * @brief creates a QModelIndex for the given item
   * @param item    The item to create an index for
   * @param column  The index column. Defaults to column 0.
   * @return A QModelIndex for the given item
   */
  QModelIndex index(const QAbstractTreeItem* item, int column = 0) const;

  /**
   * @brief Creates a list of QModelIndex for the given items
   * @param items   The items to create an indices for
   * @param column  The index column. Defaults to column 0.
   * @return A list of QModelIndex representing the givnen items
   */
  QList<QModelIndex> index(const QList<QAbstractTreeItem*>& items, int column = 0) const;

protected:
  /**
   * @brief Returns a mapping TreeModel Column -> TreeItem Column
   *
   * In order to support multiple different types of TreeItems in one TreeModel,
   * this method maps the TreeModel's columns to the TreeItem's columns.
   * Thus, a tree Item may support more / less / different columns than the
   * TreeModel it is contained in.
   *
   * If there is no mapping, this function should return -1. In the default
   * data() implemenetation, this will result in empty data.
   *
   * The default implementation returns the given column, i.e. it assumes that
   * the TreeModel and TreeItem are column-equivalent.
   *
   * @param model_column    The column number of the model
   * @param tree_item_type  The type of the desired TreeItem (@see{QAbstractTreeItem::type()})
   *
   * @return The column number of the TreeItem (or -1, if there is no mapping available)
   */
  virtual int mapColumnToItem(int model_column, int tree_item_type) const;

public slots:
  void updateItem(const QAbstractTreeItem* item, const QVector<int>& roles = QVector<int>());
  void updateAll(const QVector<int>& roles = QVector<int>());

private:
  QAbstractTreeItem* root_item_; /**< The (insivible) root item of this model */

  void updateChildrenRecursive(const QAbstractTreeItem* parent, const QVector<int>& roles = QVector<int>());
};
