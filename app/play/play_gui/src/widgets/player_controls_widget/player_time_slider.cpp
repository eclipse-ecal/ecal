#include "player_time_slider.h"

#include <QMouseEvent>
#include <QStyleOptionSlider> 

////////////////////////////////////
// Constructor
////////////////////////////////////

PlayerTimeSlider::PlayerTimeSlider(Qt::Orientation orientation, QWidget* parent)
  : QSlider(orientation, parent)
{}

PlayerTimeSlider::PlayerTimeSlider(QWidget* parent)
  : QSlider(parent)
{}

////////////////////////////////////
// Change Value by clicking
////////////////////////////////////

void PlayerTimeSlider::mousePressEvent(QMouseEvent* ev)
{
  QStyleOptionSlider opt;
  initStyleOption(&opt);
  QRect slider_handle_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

  if (ev->button() == Qt::LeftButton && !slider_handle_rect.contains(ev->pos()))
  {
    int jump_to_value;
    if (orientation() == Qt::Vertical)
    {
      double half_handle_height = (0.5 * slider_handle_rect.height()) + 0.5; // Correct rounding
      int adapted_pos_y = ev->y();

      if (adapted_pos_y < half_handle_height)
        adapted_pos_y = half_handle_height;
      if (adapted_pos_y > height() - half_handle_height)
        adapted_pos_y = height() - half_handle_height;

      // get new dimensions accounting for slider handle height
      double new_height = (height() - half_handle_height) - half_handle_height;
      double normalized_position = (adapted_pos_y - half_handle_height) / new_height;

      jump_to_value = minimum() + ((maximum() - minimum()) * normalized_position);
    }
    else
    {
      double half_handle_width = (0.5 * slider_handle_rect.width()) + 0.5; // Correct rounding
      int adapted_pos_x = ev->x();

      if (adapted_pos_x < half_handle_width)
        adapted_pos_x = half_handle_width;
      if (adapted_pos_x > width() - half_handle_width)
        adapted_pos_x = width() - half_handle_width;

      // get new dimensions accounting for slider handle width
      double new_width = (width() - half_handle_width) - half_handle_width;
      double normalizedPosition = (adapted_pos_x - half_handle_width) / new_width;

      jump_to_value = minimum() + ((maximum() - minimum()) * normalizedPosition);
    }

    if (invertedAppearance())
      setValue(maximum() - jump_to_value);
    else
      setValue(jump_to_value);

    ev->accept();
  }
  QSlider::mousePressEvent(ev);
}