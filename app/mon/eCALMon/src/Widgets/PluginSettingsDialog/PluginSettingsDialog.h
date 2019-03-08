#include "ui_PluginSettingsDialog.h"

#include <QDialog>

class PluginSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginSettingsDialog(QWidget *parent = 0);
    ~PluginSettingsDialog();

private:
    Ui::PluginSettingsDialog ui_;
};
