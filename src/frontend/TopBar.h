/*
 *  MatraKi
 *  Copyright (C) 2016-2018 Muhammad Mominul Huque <mominul2082@gmail.com>
 *  Copyright (C) 2026 Md Sojib Hossain — Emerald Design System
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOPBAR_H
#define TOPBAR_H

#include <QMainWindow>

namespace Ui {
class TopBar;
}

class QActionGroup;

class QAction;

class MatraKiMenu;

class MatraKiToggle;

class QSystemTrayIcon;

class LayoutViewer;

class AboutDialog;

class AboutFile;

class SettingsDialog;

class AutoCorrectDialog;

class HarperDialog;

class QSimpleUpdater;

class TopBar : public QMainWindow {
Q_OBJECT

public:
  explicit TopBar(QWidget *parent = nullptr);

  ~TopBar() override;

protected:
  void closeEvent(QCloseEvent *event) override;

  bool eventFilter(QObject *object, QEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;

  void mouseReleaseEvent(QMouseEvent *event) override;

private slots:

  void layoutMenuInstall_clicked();

  void iconMenuLayout_clicked();

  void iconMenuHarper_clicked();

  void iconMenuAbout_clicked();

  void iconMenuDonate_clicked();

  void iconMenuOnTray_clicked();

  void trayMenuRestore_clicked();

  void on_buttonIcon_clicked();

  void on_buttonSetLayout_clicked();

  void on_buttonShutdown_clicked();

  void on_buttonViewLayout_clicked();

  void on_buttonSettings_clicked();

  void applyTheme();

private:
  Ui::TopBar *ui;
  bool canMoveTopbar = false;
  bool positionChanged = false;
  int pressedMouseX, pressedMouseY;
  QSystemTrayIcon *tray;
  QSimpleUpdater *updater;

  /* Dialogs */
  AboutDialog *aboutDialog;
  LayoutViewer *layoutViewer;
  SettingsDialog *settingsDialog;
  HarperDialog *harperDialog;

  /* Layout Popup Menu */
  MatraKiMenu *layoutMenu;
  /* Icon Button Popup Menu */
  MatraKiMenu *iconMenu;
  QAction *iconMenuOnTray;
  QAction *iconMenuLayout;
  QAction *iconMenuAbout;
  QAction *iconMenuHarper;
  QAction *iconMenuWebsite;
  QAction *iconMenuDonate;
  QAction *iconMenuUpdate;
  MatraKiToggle *harperToggle;

  /* Tray Popup Menu */
  QMenu *trayMenu;
  QAction *trayMenuRestore;

  void SetupTopBar();

  void SetupPopupMenus();

  void SetupTrayIcon();

  void checkForUpdate(bool silent = false);

  void RefreshLayouts();

  void DataMigration();

  /* Re-tint every icon of the toolbar and of the logo menu. */
  void refreshMenuIcons();
};

#endif // TOPBAR_H
