/*
 *  MatraKi Keyboard
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

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFontDatabase>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

#include "TopBar.h"
#include "SingleInstance.h"
#include "AboutDialog.h"
#include "AboutLayoutDialog.h"
#include "SettingsDialog.h"
#include "AutoCorrectDialog.h"
#include "HarperDialog.h"
#include "LayoutViewer.h"
#include "UpdateDialog.h"
#include "Layout.h"
#include "Settings.h"
#include "MatraKiIcons.h"
#include "MatraKiMenu.h"
#include "MatraKiToggle.h"
#include "ThemeManager.h"

namespace {

/**
 * Development helper: render every window of the redesigned interface to a
 * PNG file so the artwork can be reviewed without a display server.
 *
 * Usage:  MATRAKI_UI_SHOTS=/tmp/shots ./matraki-gui
 */
void renderUiPreviews(const QString &dirPath) {
  QDir dir(dirPath);
  if (!dir.exists()) dir.mkpath(".");

  /* The toolbar owns the global settings and layout objects, so it is kept
     alive for the whole preview run. */
  TopBar *bar = new TopBar();

  const QStringList modes = {"dark", "light"};

  for (const QString &mode : modes) {
    gSettings->setThemeMode(mode);
    ThemeManager::instance().setModeString(mode);

    const MatraKiPalette p = ThemeManager::instance().palette();

    /* ---- Toolbar ---- */
    bar->show();
    QApplication::processEvents();
    bar->grab().save(dir.filePath(QString("toolbar-%1.png").arg(mode)));
    bar->hide();

    /* ---- Logo menu ---- */
    {
      MatraKiMenu menu(QStringLiteral("মাত্রা-কী"));
      menu.setRowWidth(288);
      menu.setStyleSheet(ThemeManager::instance().iconMenuStyle());
      menu.addRow(QStringLiteral("বর্তমান কীবোর্ড লেআউট সম্পর্কে"),
                  MatraKiIcons::pixmap(MatraKiIcons::Keyboard, p.icon, 18));
      menu.addRow(QStringLiteral("মাত্রা-কী সম্পর্কে"),
                  MatraKiIcons::pixmap(MatraKiIcons::Info, p.icon, 18));
      menu.addSeparatorRow();
      menu.addRow(QStringLiteral("অফিসিয়াল ওয়েবসাইট"),
                  MatraKiIcons::pixmap(MatraKiIcons::World, p.icon, 18),
                  QStringLiteral("লাইভ কীবোর্ড"));
      MatraKiToggle *toggle = new MatraKiToggle(&menu);
      toggle->setChecked(true);
      toggle->setOnColor(p.brand);
      toggle->setOffColor(p.dark ? QColor("#2E4038") : QColor("#CBD7D1"));
      menu.addToggleRow(QStringLiteral("হার্পার ব্যাকরণ ও বানান যাচাই"),
                        MatraKiIcons::pixmap(MatraKiIcons::SpellCheck, p.icon, 18), toggle);
      menu.addSeparatorRow();
      menu.addRow(QStringLiteral("সহায়তা ও ডোনেশন"),
                  MatraKiIcons::pixmap(MatraKiIcons::Heart, p.danger, 18));
      menu.addRow(QStringLiteral("আপডেট চেক করুন"),
                  MatraKiIcons::pixmap(MatraKiIcons::Refresh, p.icon, 18));
      menu.show();
      QApplication::processEvents();
      menu.grab().save(dir.filePath(QString("menu-%1.png").arg(mode)));
      menu.hide();
    }

    struct Shot {
      QString name;
      QWidget *widget;
    };

    QList<Shot> shots;

    AboutDialog about;
    AboutLayoutDialog aboutLayout;
    SettingsDialog settings;
    AutoCorrectDialog autoCorrect;
    HarperDialog harper;
    LayoutViewer viewer;
    UpdateDialog updateDlg;
    updateDlg.setUpdateInfo(false, QStringLiteral(PROJECT_VERSION), QString(), QString(), QString());

    shots << Shot{"about", &about};
    shots << Shot{"about-layout", &aboutLayout};
    shots << Shot{"settings", &settings};
    shots << Shot{"autocorrect", &autoCorrect};
    shots << Shot{"harper", &harper};
    shots << Shot{"layout-viewer", &viewer};
    shots << Shot{"update", &updateDlg};

    for (const Shot &shot : shots) {
      shot.widget->show();
      QApplication::processEvents();
      QApplication::processEvents();
      const QString path = dir.filePath(QString("%1-%2.png").arg(shot.name, mode));
      shot.widget->grab().save(path);
      shot.widget->hide();
    }
  }
}

} // namespace

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("MatraKi");
  app.setApplicationVersion(PROJECT_VERSION);

  /* Load Tiro Bangla font family into Qt application */
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/TiroBangla-Regular.ttf"));
  QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/TiroBangla-Italic.ttf"));

  QFont defaultFont(QStringLiteral("Tiro Bangla"), 10);
  defaultFont.setStyleHint(QFont::SansSerif);
  app.setFont(defaultFont);

  /* The developer preview runs independently of an already running
     instance, so it is handled before the single instance guard. */
  const QString shotsDir = qEnvironmentVariable("MATRAKI_UI_SHOTS");
  if (!shotsDir.isEmpty()) {
    renderUiPreviews(shotsDir);
    return 0;
  }

  // Prevent many instances of the app to be launched
  QString name = "com.matraki.keyboard";
  SingleInstance instance;
  if (instance.hasPrevious(name)) {
    QMessageBox msgBox(QMessageBox::Information,
                       "MatraKi",
                       "MatraKi is already running on this system.",
                       QMessageBox::Ok);
    msgBox.exec();
    return 0;
  }

  instance.listen(name);

  TopBar w;
  w.show();
  return app.exec();
}
