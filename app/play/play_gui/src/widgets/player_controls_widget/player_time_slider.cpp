#include "player_time_slider.h"

#include <QMouseEvent>

PlayerTimeSlider::PlayerTimeSlider(Qt::Orientation orientation, QWidget* parent)
  : QSlider(orientation, parent)
{}

PlayerTimeSlider::PlayerTimeSlider(QWidget* parent)
  : QSlider(parent)
{}

void PlayerTimeSlider::mousePressEvent(QMouseEvent* ev)
{
  if (ev->button() == Qt::MouseButton::LeftButton)
  {
    ev->accept();
    int x = ev->pos().x();
    int jump_to_value = (maximum() - minimum()) * x / width() + minimum();
    setValue(jump_to_value);
  }
  else
  {
    return QSlider::mousePressEvent(ev);
  }
}
