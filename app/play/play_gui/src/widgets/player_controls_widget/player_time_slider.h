#pragma once

#include <QSlider>
#include <QPoint>

/**
 * QSlider that immediatelly jumps when you click on it.
 */
class PlayerTimeSlider : public QSlider
{
  Q_OBJECT

////////////////////////////////////
// Constructor
////////////////////////////////////
public:
  PlayerTimeSlider(Qt::Orientation orientation, QWidget* parent = nullptr);
  PlayerTimeSlider(QWidget* parent = nullptr);

////////////////////////////////////
// Change Value by clicking
////////////////////////////////////
protected:
  void mousePressEvent(QMouseEvent* ev) override;
};

