#pragma once

#include <QSlider>

class PlayerTimeSlider : public QSlider
{
  Q_OBJECT

public:
  PlayerTimeSlider(Qt::Orientation orientation, QWidget* parent = nullptr);
  PlayerTimeSlider(QWidget* parent = nullptr);

protected:
  void mousePressEvent(QMouseEvent* ev) override;
};

