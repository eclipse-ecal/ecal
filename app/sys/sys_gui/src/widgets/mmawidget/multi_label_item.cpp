/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "multi_label_item.h"

#include <QLayout>
#include <QSpacerItem>

MultiLabelItem::MultiLabelItem(unsigned int number_of_labels, QWidget *parent)
  : QWidget(parent)
{
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(6);

  for (unsigned int i = 0; i < number_of_labels; i++)
  {
    QLabel* label = new QLabel(this);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label->setSizePolicy(sizePolicy);

    layout->addWidget(label);
    label_list_.push_back(label);
  }


  QSpacerItem* horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(horizontalSpacer);

  QMetaObject::connectSlotsByName(this);
}

MultiLabelItem::~MultiLabelItem()
{
}

void MultiLabelItem::setText(unsigned int number, const QString& text)
{
  label_list_[number]->setText(text);
}

void MultiLabelItem::setPixmap(unsigned int number, const QPixmap& pixmap)
{
  label_list_[number]->setPixmap(pixmap);
}

void MultiLabelItem::setMinimumWidth(unsigned int number, int width)
{
  label_list_[number]->setMinimumSize(QSize(width, 0));
}

void MultiLabelItem::setFont(unsigned int number, const QFont& font)
{
  label_list_[number]->setFont(font);
}

void MultiLabelItem::setStyleSheet(unsigned int number, const QString& stylesheet)
{
  label_list_[number]->setStyleSheet(stylesheet);
}