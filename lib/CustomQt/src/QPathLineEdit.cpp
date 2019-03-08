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

#include "CustomQt/QPathLineEdit.h"

#include <QAbstractItemView>
#include <QTimer>

#include <QCompleter>
#include <CustomQt/QCustomFileSystemModel.h>

QPathLineEdit::QPathLineEdit(QWidget *parent)
  : QPathLineEdit("", parent)
{}

QPathLineEdit::QPathLineEdit(const QString &contents, QWidget *parent)
  : QLineEdit(contents, parent)
{
  completer_       = new QCompleter(this);
  completer_model_ = new QCustomFileSystemModel(completer_);
  completer_       ->setModel(completer_model_);
  completer_       ->setMaxVisibleItems(10);
  completer_       ->popup()->setTextElideMode(Qt::TextElideMode::ElideLeft);

  connect(completer_, static_cast<void(QCompleter::*)(const QString &)>(&QCompleter::activated), this,
    [this]() {emit editingFinished(); });

  setCompleter(completer_);

  connect(this, &QLineEdit::textChanged,
    [this](const QString & text)
        {
          if (text.isEmpty())
          {
            completer_model_->setRootPath("");
            return;
          }

          QString root_path;
#ifdef WIN32
          if (text.endsWith('\\') || text.endsWith('/'))
#else //WIN32
          if (text.endsWith('/'))
#endif //WIN32
          {
            root_path = text;
            // Usually, the completer disappears, when typing a directory
            // separator, but those are the cases when the completer is especially
            // usefull. We use a small hack here, in order to force-show it again.
            QTimer::singleShot(10, 
                [this]() 
                {
                  if ((completer_->completionCount() > 0) && completer_->popup()->isHidden())
                  {
                    completer_->complete();
                  }
                });
          }
          else
          {
            QFileInfo file_info(text);
            root_path = file_info.absoluteDir().absolutePath();
          }

          if (root_path != completer_model_->rootPath())
          {
            completer_model_->setRootPath(root_path);
          }
        }
  );

}

QPathLineEdit::~QPathLineEdit()
{}

void QPathLineEdit::setFilter(QDir::Filters filters)
{
  completer_model_->setFilter(filters);
}

QDir::Filters QPathLineEdit::filter() const
{
  return completer_model_->filter();
}