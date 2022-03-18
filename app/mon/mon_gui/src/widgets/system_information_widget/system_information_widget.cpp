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

#include "system_information_widget.h"

#include <ecal/ecal.h>

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QTimer>

SystemInformationWidget::SystemInformationWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  setLabelText();
}

SystemInformationWidget::~SystemInformationWidget()
{}

void SystemInformationWidget::changeEvent(QEvent* event)
{
  QWidget::changeEvent(event);

  if (event->type() == QEvent::PaletteChange)
  {
    // Re-create the HTML for the textfield, as the label color may have changed
    setLabelText();
  }
}

void SystemInformationWidget::setLabelText()
{
  std::string config_dump;
  eCAL::Process::DumpConfig(config_dump);

  ui_.system_information_label->setTextFormat(Qt::TextFormat::RichText);
  QString html_text = toHtml(QString::fromStdString(config_dump));
  ui_.system_information_label->setText(html_text);
}

QString SystemInformationWidget::toHtml(const QString& system_information)
{
  auto current_palette = QApplication::palette();
  QColor link_color = current_palette.color(QPalette::ColorRole::Link);

  QStringList lines = system_information.split('\n');
  
  for (QString& line : lines)
  {
    if (line.contains("Default INI"))
    {
      QString line_with_link;

      int colon_pos = line.indexOf(":");
      if (colon_pos > 0)
      {
        QString front = line.mid(0, colon_pos + 1);
        QString rest  = line.mid(colon_pos + 1, -1);

        QString path = rest.trimmed();
        bool file_exists = QFile::exists(path);

        if (file_exists)
        {
          QUrl file_url = QUrl::fromLocalFile(path);
          line_with_link = front.toHtmlEscaped() + " <a href=\"" + file_url.toString() + "\"><span style=\"color:" + link_color.name() + "; \">" + path.toHtmlEscaped() + "</span></a>";
        }
      }

      if (!line_with_link.isEmpty())
        line = line_with_link;
      else
        line = line.trimmed().toHtmlEscaped();
    }
    else
    {
      line = line.trimmed().toHtmlEscaped();
    }
  }

  lines.push_front("<html><head/><body><pre>");
  lines.push_back("</pre></body>");

  return lines.join("\n");
}
