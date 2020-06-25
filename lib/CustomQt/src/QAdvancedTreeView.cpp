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

#include "CustomQt/QAdvancedTreeView.h"

#include <QHeaderView>
#include <QMenu>

QAdvancedTreeView::QAdvancedTreeView(QWidget* parent)
  : QTreeView(parent)
  , forced_columns_ { 0 }
  , auto_expand_depth_ (-1)
{
  header()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(header(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(headerContextMenu(const QPoint&)));
}

QAdvancedTreeView::~QAdvancedTreeView() {}

void QAdvancedTreeView::setHeader(QHeaderView *new_header)
{
  // Set the new Header
  QTreeView::setHeader(new_header);

  // Re-register the context menu
  header()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(new_header, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(headerContextMenu(const QPoint&)));

  // No need to disconnect the old slot, as the old header is auto-deleted
}

QVector<int> QAdvancedTreeView::forcedColumns() const
{
  return forced_columns_;
}

void QAdvancedTreeView::setForcedColumns(const QVector<int>& forced_columns)
{
  forced_columns_ = forced_columns;
}

bool QAdvancedTreeView::isForced(int column)
{
  for (int forced_column : forced_columns_)
  {
    if (column == forced_column)
      return true;
  }
  return false;
}

void QAdvancedTreeView::setModel(QAbstractItemModel *model)
{
  if (this->model())
  {
    disconnect(this, SLOT(expandInsertedRows(const QModelIndex&, int, int)));
  }
  QTreeView::setModel(model);
  connect(this->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(expandInsertedRows(const QModelIndex&, int, int)));
}

void QAdvancedTreeView::setAutoExpandDepth(int auto_expand_depth)
{
  auto_expand_depth_ = auto_expand_depth;
}

int QAdvancedTreeView::autoExpandDepth() const
{
  return auto_expand_depth_;
}

int QAdvancedTreeView::depth(const QModelIndex& node) const
{
  if (!node.isValid())
    // The invisible root has no depth
    return -1;
  else if (!node.parent().isValid())
    // The root node has a depth of 0
    return 0;
  else
    // recursively walk upwards and count the steps
    return (1 + (depth(node.parent())));
}

void QAdvancedTreeView::keyPressEvent(QKeyEvent* key_event)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
  if (key_event->key() == Qt::Key::Key_Escape)
#else // QT_VERSION
  if (key_event->matches(QKeySequence::StandardKey::Cancel))
#endif // QT_VERSION
  {
    if (((selectionMode() == (QAbstractItemView::SelectionMode::ExtendedSelection))
      || (selectionMode() == (QAbstractItemView::SelectionMode::ContiguousSelection))
      || (selectionMode() == (QAbstractItemView::SelectionMode::MultiSelection)))
      && (selectionModel()->selection().size() > 0))
    {
      selectionModel()->clearSelection();
    }
    else
    {
      QTreeView::keyPressEvent(key_event);
    }
  }
  else if (key_event->matches(QKeySequence::StandardKey::Delete))
  {
    emit keySequenceDeletePressed();
  }
  else if (key_event->matches(QKeySequence::StandardKey::Copy))
  {
    emit keySequenceCopyPressed();
  }
  else if ((key_event->key() == Qt::Key::Key_Return) || (key_event->key() == Qt::Key::Key_Enter))
  {
    emit keyEnterPressed();
  }
  else
  {
    QTreeView::keyPressEvent(key_event);
  }
}

void QAdvancedTreeView::headerContextMenu(const QPoint& pos)
{
  QMenu context_menu(tr("Context menu"), this);

  for (int col = 0; col < model()->columnCount(); col++)
  {
    QString column_name = model()->headerData(col, Qt::Horizontal).toString();
    QAction* column_action = new QAction(column_name, &context_menu);

    column_action->setEnabled(!isForced(col));

    column_action->setCheckable(true);
    column_action->setChecked(!isColumnHidden(col));
    column_action->setData(col); // As we want to connect everything to one slot, the action itself must carry information about the selected column
    connect(column_action, &QAction::triggered, [col, this]() { setColumnHidden(col, !isColumnHidden(col)); });
    context_menu.addAction(column_action);
  }
  context_menu.exec(header()->mapToGlobal(pos));
}

void QAdvancedTreeView::expandInsertedRows(const QModelIndex& parent, int first, int last)
{
  if (auto_expand_depth_ < 0)
    return;

  // If the parent did not have any children previously, we may expand it. If
  // it had children, we don't expand it, as the user might have collapsed it 
  // and does not want it to automatically expand again
  int new_item_count = last - first + 1;
  int parent_depth = depth(parent);
  if ((new_item_count == (this->model()->rowCount(parent)))
    && (parent_depth <= auto_expand_depth_))
  {
    expand(parent);
  }

  // The added node may already have children, so we need to expand those as well
  for (int i = first; i <= last; i++)
  {
    QModelIndex new_node_index = this->model()->index(i, 0, parent);
    expandNodeToDepth(new_node_index, auto_expand_depth_ - parent_depth - 1);
  }
}

void QAdvancedTreeView::expandNodeToDepth(const QModelIndex& node, int remaining_depth)
{
  if (remaining_depth >= 0)
  {
    expand(node);
    for (int i = 0; i < model()->rowCount(node); i++)
    {
      QModelIndex child_index = model()->index(i, 0, node);
      expandNodeToDepth(child_index, remaining_depth - 1);
    }
  }
}

QByteArray QAdvancedTreeView::saveState(int32_t version)
{
  QByteArray state;
  QDataStream state_stream(&state, QIODevice::WriteOnly);

  // Save the column count, so we can make sure to not run out of data, later
  state_stream << version;
  state_stream << (int32_t)model()->columnCount();

  for (int i = 0; i < model()->columnCount(); i++)
  {
    ColumnState column_state;
    column_state.visible = !isColumnHidden(i);
    column_state.position = header()->visualIndex(i);

    // In order to obtain the section size of hidden sections we need to temporarily show them.
    setColumnHidden(i, false);
    column_state.size = header()->sectionSize(i);
    setColumnHidden(i, !column_state.visible);

    state_stream << column_state;
  }
  return state;
}

bool QAdvancedTreeView::restoreState(const QByteArray& state, int32_t version)
{
  QDataStream state_stream(state);

  // The state must at least contain the version and number of columns
  if (state.size() < (int)(2 * sizeof(int32_t)))
  {
    return false;
  }

  int32_t state_version;
  state_stream >> state_version;
  if (state_version != version)
  {
    return false;
  }

  int32_t column_count;
  state_stream >> column_count;

  // Abort when the state binary aparently does not describe the column count of the current treeview
  if (column_count != model()->columnCount())
  {
    return false;
  }

  // Measure how many bytes a ColumnState actually occupies in a QByteArray. This is most likely NOT the same as sizeof(ColumnState)
  QByteArray temp_array;
  QDataStream temp_stream(&temp_array, QIODevice::WriteOnly);
  ColumnState temp_state;
  temp_stream << temp_state;
  int size_of_column_state = temp_array.size();

  // Abort when the state binary data contains too few / too much elements to describe all columns
  if (state.size() != (2 * (int)sizeof(int32_t) + (column_count * size_of_column_state)))
  {
    return false;
  }

  std::list<std::pair<int, ColumnState>> column_state_list;

  for (int column = 0; column < model()->columnCount(); column++)
  {
    ColumnState column_state;
    state_stream >> column_state;
    column_state_list.emplace_back(std::make_pair(column, column_state));
  }

  column_state_list.sort([](const std::pair<int, ColumnState>& s1, const std::pair<int, ColumnState>& s2) {return s1.second.position < s2.second.position; });

  for (auto& column_state_pair : column_state_list)
  {
    setColumnHidden(column_state_pair.first, false);
    if (column_state_pair.second.size > 0)
    {
      header()->resizeSection(column_state_pair.first, column_state_pair.second.size);
    }
    setColumnHidden(column_state_pair.first, (!column_state_pair.second.visible && !isForced(column_state_pair.first)));
    header()->moveSection(header()->visualIndex(column_state_pair.first), column_state_pair.second.position);
  }

  return true;
}

QStyleOptionViewItem QAdvancedTreeView::viewOptions() const
{
  return QTreeView::viewOptions();
}