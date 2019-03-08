#pragma once

#include <QToolButton>
#include <QMainWindow>
#include <QListView>

class QListMenuToolButton
  : public QToolButton
{
  Q_OBJECT

public:
  QListMenuToolButton(QWidget* parent = nullptr);
  ~QListMenuToolButton();

  void setModel(QAbstractItemModel* model);
  void setModelColumn(int visible_column);
  void setMaxVisibleItems(int max_items);

  QAbstractItemModel* model() const;
  int modelColumn() const;
  int maxVisibleItems() const;

  void setSelection(const QModelIndex& selection);
  void setSelection(int row);
  void clearSelection();

signals:
  void itemSelected(const QModelIndex& index);

public slots:
  void showListMenu();

private slots:
  void listItemClicked(const QModelIndex& index);

protected:
  void mousePressEvent(QMouseEvent *e) override;

private:
  QMainWindow* popup_;
  QListView*   list_view_;

  int max_visible_items_;
};

