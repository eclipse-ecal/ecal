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

#include <QTreeView>
#include <QPoint>
#include <QKeyEvent>
#include <QDataStream>

/**
 * @brief The QAdvancedTreeView class is a QTreeView with more features
 *
 * The added featurs compared to an ordinary QTreeView are:
 *   - A header context menu that lets the user select / deselect visible
 *     columns
 *   - The possbilty to save the state (i.e. the visibile columns and the size
 *     of all columns) to a QByteArray and restore the state again
 *   - Auto-expanding up to a certain depth, if new elements are added to the
 *     model
 *   - Deselection of elements with the ESC key
 *   - Signals for often used keys and key sequences
 *
 * Note that some functions of the QTreeView are not virtual, although they have
 * to be overridden:
 *   - setModel
 *   - setHeader
 * Therefore, the object should always be of type @code{QAdvancedTreeView}, not
 * of type @code{QTreeView} for this class to operate correctly.
 */
class QAdvancedTreeView : public QTreeView
{
  Q_OBJECT

public:
  explicit QAdvancedTreeView(QWidget *parent = Q_NULLPTR);
  ~QAdvancedTreeView();
  void setHeader(QHeaderView *new_header); // Not virtual, but needs to be overridden

  /**
   * @brief Returns a list of all forced columns
   *
   * A Forced column cannot be hidden by the user. This is necessary, as he
   * would not be able to enable any column once he disabled all columns
   * (disabling all columns hides the header).
   *
   * By default, column 0 is the only forced column.
   *
   * @return The list of all forced columns
   */
  QVector<int> forcedColumns() const;

  /**
   * @brief Sets the list of forced columns
   *
   * A Forced column cannot be hidden by the user. This is necessary, as the
   * would not be able to enable any column once he disabled all columns
   * (disabling all columns hides the header).
   *
   * By default, column 0 is the only forced column.
   *
   * @param forced_columns The list of forced columns
   */
  void setForcedColumns(const QVector<int>& forced_columns);

  /**
   * @brief Check whether the given column is forced, i.e. cannot be disabled by the user
   * @param column The column to check
   * @return True, if the column is forced
   */
  bool isForced(int column);

  void setModel(QAbstractItemModel *model) override; // Not virtual, but needs to be overridden

  /**
   * @brief Sets the maximum depth that the tree view will expand a node to, if children are added
   *
   * A depth < 0 means that the tree view should not auto-expand any nodes.
   * The default is -1.
   * The invisible root node does not count as depth, i.e. a depth of 0 applies
   * to the visible elements of a flat list.
   *
   * When a Node is added to the Model, its parent will be expanded, if it did
   * not have any children before. If it did have children, the expansion state
   * will not be changed in order to give the user the ability to manually
   * collapse nodes.
   *
   * @param auto_expand_depth The maximum depth of a node that should be auto-expanded.
   */
  void setAutoExpandDepth(int auto_expand_depth);

  /**
   * @brief Returns the current auto expand depth.
   *
   * A depth < 0 means that the tree view should not auto-expand any nodes.
   * The default is -1.
   * The invisible root node does not count as depth, i.e. a depth of 0 applies
   * to the visible elements of a flat list.
   *
   * When a Node is added to the Model, its parent will be expanded, if it did
   * not have any children before. If it did have children, the expansion state
   * will not be changed in order to give the user the ability to manually
   * collapse nodes.
   *
   * @return the current auto-expand depth
   */
  int autoExpandDepth() const;

  /**
   * @brief Save the tree state (visible columns, column size and order) to a byte array.
   *
   * The byte array will also contain information about the column count and the
   * version number that is given as parameter. Those information will be used
   * as sanity check when restoring the state.
   *
   * Calling this function will temporarily set all hidden columns visible and
   * hide them again, afterwards. Usually, this process is fast enough to not be
   * visible. Nevertheless, if called in a loop (e.g. by connecting it to the
   * @code{QHeaderView::sectionResized} signal, this may cause undesired effects
   * and should be avoided.
   *
   * @param version A number that will be included in the QByteArray and used to perform a sanity check when restoring the state
   *
   * @return The Tree State as QByteArray
   */
  QByteArray saveState(int32_t version = 0);

  /**
   * @brief restores the tree state (visible columns, column size and order) from a given QByteArray.
   *
   * Before restoring, a sanity check is performed:
   *    - The column count must match this treeView
   *    - The version number from the state must match the given version number
   *
   * @param state    The state to restore
   * @param version  The version number to use for the sanity check
   *
   * @return True, if the state could be restored
   */
  bool restoreState(const QByteArray& state, int32_t version = 0);

  QStyleOptionViewItem viewOptions() const override;

protected:
  void keyPressEvent(QKeyEvent* key_event) override;

signals:
  void keySequenceDeletePressed();
  void keySequenceCopyPressed();
  void keyEnterPressed();

private slots:
 void headerContextMenu(const QPoint& pos);
 void expandInsertedRows(const QModelIndex& parent, int first, int last);

private:
  QVector<int> forced_columns_;                                                 /** The list of columns that cannot be hidden by the user */
  int auto_expand_depth_;                                                       /** The maximum depth up to which a node will automatically be expanded, if a child is added */

  /**
   * @brief internal recursive helper function for expanding nodes
   */
  void expandNodeToDepth(const QModelIndex& node, int remaining_depth);

  /**
   * @brief Counts the depth of the node
   * @param node The node to count the depth for
   * @return The depth of the node
   */
  int depth(const QModelIndex& node) const;

  struct ColumnState
  {
    bool     visible  = false;
    uint32_t size     = 0;
    uint32_t position = 0;

    friend QDataStream& operator<< (QDataStream &stream, const QAdvancedTreeView::ColumnState& state)
    {
      return stream << state.visible << state.size << state.position;
    }

    friend QDataStream& operator>> (QDataStream &stream, QAdvancedTreeView::ColumnState& state)
    {
      stream >> state.visible;
      stream >> state.size;
      stream >> state.position;
      return stream;
    }
  };
};
