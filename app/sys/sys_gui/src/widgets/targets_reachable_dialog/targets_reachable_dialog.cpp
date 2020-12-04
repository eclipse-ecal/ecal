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

#include "targets_reachable_dialog.h"

#include <globals.h>
#include <QDialogButtonBox>
#include <QPushButton>

TargetsReachableDialog::TargetsReachableDialog(QWidget *parent, std::vector<std::string> targets)
  : QDialog(parent)
  , targets_(targets)
  , start_tasks_(false)
  , close_dialog_(false)
{
  ui_.setupUi(this);

  cancel_button_ = ui_.buttonBox->addButton(tr("Cancel"), QDialogButtonBox::ButtonRole::RejectRole);
  start_button_ = ui_.buttonBox->addButton(tr("Start anyway"), QDialogButtonBox::ButtonRole::ActionRole);

  connect(cancel_button_, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(start_button_, SIGNAL(clicked()), this, SLOT(startClicked()));
  
  for (int i = 0; i < (int)targets_.size(); i++)
  {
    QLabel* icon_label = new QLabel(this);
    icon_label->setAlignment(Qt::AlignCenter);
    icon_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    icon_label->setMinimumSize(IMAGE_SIZE, IMAGE_SIZE);

    QLabel* target_name_label = new QLabel(this);
    target_name_label->setText(targets_[i].c_str());
    QFont bold_font;
    bold_font.setBold(true);
    target_name_label->setFont(bold_font);

    QLabel* status_label = new QLabel(this);

    ui_.content_widget_gridlayout->addWidget(icon_label,        i, ICON_COL);
    ui_.content_widget_gridlayout->addWidget(target_name_label, i, TARGET_NAME_COL);
    ui_.content_widget_gridlayout->addWidget(status_label,      i, STATUS_COL);
  }

  adjustSize();

  update();
}

TargetsReachableDialog::~TargetsReachableDialog()
{
}

void TargetsReachableDialog::update()
{
  if (close_dialog_)
  {
    start_tasks_ = true;
    close();
  }

  bool all_available = true;
  for (int i = 0; i < (int)targets_.size(); i++)
  {
    QLabel* icon_label   = (QLabel*)ui_.content_widget_gridlayout->itemAtPosition(i, ICON_COL)->widget();
    QLabel* status_label = (QLabel*)ui_.content_widget_gridlayout->itemAtPosition(i, STATUS_COL)->widget();

    if ((targets_[i] == eCAL::Process::GetHostName())
      || Globals::EcalSysInstance()->IseCALSysClientRunningOnHost(targets_[i]))
    {
      icon_label->setPixmap(checkmark_icon);
      status_label->setText("Available");
    }
    else
    {
      icon_label->setPixmap(x_icon);
      status_label->setText("Waiting for sys client...");
      all_available = false;
    }
  }

  if (all_available)
  {
    // Close the dialog in the next monitor iteration. Starting all tasks right
    // now might cause some of them to fail starting, as the ecal sys client might
    // not be fully available, yet
    close_dialog_ = true;
  }
}

void TargetsReachableDialog::cancelClicked()
{
  close();
}

void TargetsReachableDialog::startClicked()
{
  start_tasks_ = true;
  close();
}

bool TargetsReachableDialog::shallStartTasks()
{
  return start_tasks_;
}
