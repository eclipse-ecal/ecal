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

#include <functional>

#include <QList>
#include <QVariant>

/**
 * @brief The QAbstractTreeItem provides an abstract tree-based impelementation of an Item model.
 *
 * Together with the QAbstractTreeModel, the QAbstractTreeItem provides a base-
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
class QAbstractTreeItem
{
public:
  /**
   * @brief Creates a new TreeItem.
   *
   * It does neither have a parent nor any children, thus effectively making it
   * a root item. When the object however is added as child to any other item,
   * the parent will be set automatically.
   */
  explicit QAbstractTreeItem();

  virtual ~QAbstractTreeItem();

  /**
   * @brief Adds a an item as child
   * @param child the new child
   * @param row the row to add the child. The item will be added before that row, so after inserting, the row will contain the inserted item. By default, the child is added as last element.
   */
  void insertChild(QAbstractTreeItem* child, int row = -1);

  /**
   * @brief removes the given row. The child is deleted automatically.
   *
   * If the child item must not be deleted, @see{detachChild()} can be used.
   *
   * @param row The row to remove
   */
  void removeChild(int row);

  /**
   * @brief removes and deletes all children of this item
   */
  void removeAllChildren();

  /**
   * @brief Detaches a child item without deleting it
   *
   * This function is meant for use cases in which the child item has to be
   * reused, for instance by inserting it again at another place. Until the item
   * has been inserted at another place, the user is responsible for memory
   * management.
   * If the item does not have to exist any more, @see{removeChild()} should be
   * used, which automatically deletes the item.
   *
   * @param row The row to remove
   *
   * @return The detached Tree Item
   */
  QAbstractTreeItem* detachChild(int row);

  /**
   * @brief Detaches this Tree Item from its parent.
   *
   * The item is not deleted. This function is meant for use cases in which the
   * item has to be reused, for instance by inserting it again at another place.
   * Until the item has been inserted again, the user is responsible for memory
   * management.
   */
  void detachFromParent();

  /**
   * @brief Moves one child to another location in the child-list
   *
   * @param from  The child that shall be moved
   * @param to    The destination row. The child will be moved before that row.
   */
  void moveChild(int from, int to);

  /**
   * @brief Recursively applies the predicate to all children and returns a list of children where the predicate is true.
   * @param p the predicate to check
   * @return a list of all (recursive) children for which the predicate is true
   */
  QList<QAbstractTreeItem*> findChildren(std::function<bool(QAbstractTreeItem*)> p);

  /**
   * @brief get the child-item from the given row
   * @param row the row to get the child-item for
   * @return the child item of the given row
   */
  QAbstractTreeItem* child(int row) const;

  /**
   * @brief gets the number of children
   * @return the number of children
   */
  int childCount() const;

  /**
   * @brief Returns the QVariant data for the given column and role of this item
   *
   * This is the most important function when subclassing the QAbstractTreeItem,
   * as the QAbstractTreeModel delegates all data calls to the items.
   *
   * The default implementation is returning an invalid QVariant.
   *
   * @param column The column to return data for
   * @param role   The role to return data for
   * @return Data for the given column an role
   */
  virtual QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;


  /**
   * @brief sets the data of this item  for the given column.
   *
   * This function is totally up to the user to implement. The default
   * implementation does nothing and returns false.
   * TreeItems that support this feature should parse the QVariant-packed data
   * appropriately and return true if setting the data was successfull.
   *
   * @param column  The column to set the data for
   * @param data    The new data
   * @param role    The Qt::ItemDataRole to set the data for
   *
   * @return        True if setting the data was successfull.
   */
  virtual bool setData(int column, const QVariant& data, Qt::ItemDataRole role = Qt::ItemDataRole::EditRole);

  /**
   * @brief returns a bitmask containing all Qt::ItemFlags for the given column
   *
   * By default, the item is enabled (ItemIsEnabled) and is allows to be
   * selected (ItemIsSelectable)
   *
   * @param column the column to get the flags for
   * 
   * @return the flasgs fr the given column
   */
  virtual Qt::ItemFlags flags(int column) const;

  /**
   * @brief Returns the row of this item in the parent
   *
   * If this item is a root item, 0 is returned.
   *
   * @return the row of this item
   */
  int row() const;

  /**
   * @brief returns a pointer to the parent
   *
   * If this item is a root item, a nullptr is returned.
   *
   * @return the parent item
   */
  QAbstractTreeItem *parentItem() const;

  /**
   * @brief Returns the type of this item
   *
   * This function is meant for determining the actual type of this object.
   * Based on that information the implementation should be able to cast the
   * pointer to anything more meaningfull than QAbstractTreeItem*.
   *
   * @return The type of this item
   */
  virtual int type() const;

private:
  QList<QAbstractTreeItem*> child_items_;    /**< The list of all children */
  QAbstractTreeItem *parent_item_;           /**< The parent item */
};
