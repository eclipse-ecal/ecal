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
#include <QDesktopServices>
#include <QMimeDatabase>
#include <QProcess>
#include <QMessageBox>

SystemInformationWidget::SystemInformationWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  connect(ui_.system_information_label, &QLabel::linkActivated, this, &SystemInformationWidget::openEcalIni);

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

void SystemInformationWidget::openEcalIni(const QUrl& url)
{
  bool success = false;

#ifdef __linux__
  success = openEcalIniElevated(url);
  if (!success)
  {
#endif

    // For non-linux this is the only relevant code. On linux it acts as fallback.
    if (url.isLocalFile())
    {
      QString path = url.toLocalFile();
      QFileInfo file_info(path);
      if (file_info.isFile())
        success = QDesktopServices::openUrl(url);
      else
        success = false;
    }
    else
    {
      success = QDesktopServices::openUrl(url);
    }

#ifdef __linux__
  }
#endif

  if (!success)
  {
    QMessageBox warning(
      QMessageBox::Icon::Warning
      , tr("Error opening file")
      , tr("Failed to open file:") + "\n" + url.toString()
      , QMessageBox::Button::Ok
      , this);

    warning.exec();
  }
}

#ifdef __linux__
  bool SystemInformationWidget::openEcalIniElevated(const QUrl& url)
  {
    // 1) Check if the file is a local file
    if (!url.isLocalFile())
      return false;

    // 2) Check if we even need root
    {
      QFileInfo file_info(url.toLocalFile());
      bool can_write = file_info.permission(QFile::WriteUser);

      if (!file_info.exists() || !file_info.isFile() || can_write)
        return false;
    }

    // 3) Get the MIME type
    QMimeType mime_type;
    {
      QMimeDatabase mimedatabase;
      mime_type = mimedatabase.mimeTypeForFile(url.toLocalFile());

      if (!mime_type.isValid())
        return false;
    }

    // 4) Get the associated *.desktop (-> the application) via "xdg-mime"
    QString xdg_mime_query_output;
    {
      QProcess xdg_mime_process;
      xdg_mime_process.start("xdg-mime", {"query", "default", mime_type.name()});

      if (xdg_mime_process.waitForFinished(1000) && (xdg_mime_process.exitCode() == 0))
      {
        xdg_mime_query_output = QString::fromUtf8(xdg_mime_process.readAllStandardOutput()).trimmed();
      }
      else
      {
        xdg_mime_process.terminate();
        return false;
      }
    }

    // 5) Check the xdg-mime output. Fallback to plain opening for anything else than gedit
    if (xdg_mime_query_output.isEmpty()
         || ((xdg_mime_query_output != "org.gnome.gedit.desktop")
             && (xdg_mime_query_output != "org.gnome.TextEditor.desktop"))
       )
    {
      return false;
    }

    // 5) If it is gedit / gnome-text-editor launch it via "gtk-launch"
    {
      QProcess gtk_launch_process;
      gtk_launch_process.start("gtk-launch", {xdg_mime_query_output, "admin://" + url.toLocalFile()});

      if (gtk_launch_process.waitForFinished(1000))
      {
        if (gtk_launch_process.exitCode() == 0)
          return true;
        else
          return false;
      }
      else
      {
        gtk_launch_process.terminate();
        return false;
      }
    }
  }
#endif
