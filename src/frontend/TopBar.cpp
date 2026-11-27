/*
 *  MatraKi
 *  Copyright (C) 2016-2018 Muhammad Mominul Huque <mominul2082@gmail.com>
 *  Copyright (C) 2026 Md Sojib Hossain — Onyx Design System
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

#include <QSystemTrayIcon>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMenu>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDialog>
#include <QFrame>
#include "TopBar.h"
#include "Layout.h"
#include "Settings.h"
#include "FileSystem.h"
#include "LayoutViewer.h"
#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "LayoutConverter.h"
#include "AutoCorrectDialog.h"
#include "HarperDialog.h"
#include "MatraKiIcons.h"
#include "MatraKiLogo.h"
#include "MatraKiMenu.h"
#include "MatraKiToggle.h"
#include "MatraKiUpdater.h"
#include "QSimpleUpdater.h"
#include "ThemeManager.h"
#include "ui_TopBar.h"

static const QString WEBSITE_URL = "https://sojibcreates-maker.github.io/MatraKi-Keyboard/";
static const QString UPDATE_PAGE_URL = "https://sojibcreates-maker.github.io/MatraKi-Keyboard/#downloads";
static const QString DEFS_URL = "https://sojibcreates-maker.github.io/MatraKi-Keyboard/updates.json";
static const QString GITHUB_API_LATEST_URL = "https://api.github.com/repos/sojibcreates-maker/MatraKi-Keyboard/releases/latest";
static const QString GITHUB_RELEASES_URL = "https://github.com/sojibcreates-maker/MatraKi-Keyboard/releases/latest";

static bool isRemoteVersionNewer(const QString &remoteVer, const QString &localVer) {
  QString r = remoteVer.trimmed();
  QString l = localVer.trimmed();
  if (r.startsWith('v') || r.startsWith('V')) r = r.mid(1);
  if (l.startsWith('v') || l.startsWith('V')) l = l.mid(1);
  QStringList rParts = r.split('.');
  QStringList lParts = l.split('.');
  int count = qMax(rParts.size(), lParts.size());
  for (int i = 0; i < count; ++i) {
    int rv = (i < rParts.size()) ? rParts.at(i).toInt() : 0;
    int lv = (i < lParts.size()) ? lParts.at(i).toInt() : 0;
    if (rv > lv) return true;
    if (rv < lv) return false;
  }
  return false;
}

TopBar::TopBar(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TopBar) {
  ui->setupUi(this);

  gLayout = new Layout();
  gSettings = new Settings();
  updater = QSimpleUpdater::getInstance();

  /* Dialogs */
  aboutDialog = new AboutDialog(Q_NULLPTR);
  layoutViewer = new LayoutViewer(Q_NULLPTR);
  settingsDialog = new SettingsDialog(Q_NULLPTR);
  harperDialog = new HarperDialog(Q_NULLPTR);

  harperToggle = new MatraKiToggle(this);
  harperToggle->setChecked(true);
  connect(harperToggle, &MatraKiToggle::toggled, this, [this](bool on) {
    harperDialog->setHarperEnabled(on);
  });

  ui->buttonIcon->installEventFilter(this);

  SetupTopBar();
  SetupPopupMenus();
  SetupTrayIcon();
  DataMigration();

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &TopBar::applyTheme);
  applyTheme();

  // Automatic background update check linked with GitHub Releases & GitHub Pages
  QTimer::singleShot(3500, this, [=]() {
    if (gSettings->getUpdateCheck()) {
      checkForUpdate(true);
    }
  });
  QTimer *periodicUpdateTimer = new QTimer(this);
  connect(periodicUpdateTimer, &QTimer::timeout, this, [=]() {
    if (gSettings->getUpdateCheck()) {
      checkForUpdate(true);
    }
  });
  periodicUpdateTimer->start(4 * 60 * 60 * 1000); // Check every 4 hours
}

TopBar::~TopBar() {
  /* Dialogs */
  delete harperDialog;
  delete layoutViewer;
  delete settingsDialog;
  delete aboutDialog;

  delete gLayout;
  delete gSettings;

  delete ui;
}

void TopBar::SetupTopBar() {
  this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  this->setAttribute(Qt::WA_TranslucentBackground);
  this->setFixedSize(QSize(this->width(), this->height()));

  if (gSettings->getTopBarWindowPosition() == QPoint(0, 0)) {
    int width = this->frameGeometry().width();
    int height = this->frameGeometry().height();
    QDesktopWidget wid;

    int screenWidth = wid.screen()->width();
    int screenHeight = wid.screen()->height();

    this->setGeometry((screenWidth / 2) - (width / 2), 40, width, height);
  } else {
    QDesktopWidget wid;
    int screenWidth = wid.screen()->width();
    int screenHeight = wid.screen()->height();
    QPoint pos = gSettings->getTopBarWindowPosition();
    int x = qBound(10, pos.x(), qMax(10, screenWidth - this->width() - 10));
    int y = qBound(10, pos.y(), qMax(10, screenHeight - this->height() - 10));
    move(x, y);
  }
}

/* ------------------------------------------------------------------ */
/*  Update dialog                                                     */
/* ------------------------------------------------------------------ */

void TopBar::checkForUpdate(bool silent) {
  if (updater) {
    updater->setModuleVersion(QStringLiteral("https://sojibcreates-maker.github.io/MatraKi-Keyboard/updates.json"),
                              PROJECT_VERSION);
  }
  MatraKiUpdater::check(this, silent);
}

/* ------------------------------------------------------------------ */
/*  Theme                                                             */
/* ------------------------------------------------------------------ */

void TopBar::applyTheme() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  this->setStyleSheet(ThemeManager::instance().topBarStyle());

  /* Brand mark on the leading button. */
  ui->buttonIcon->setIcon(QIcon(MatraKiLogo::render(36, p.brand, p.danger)));

  /* Toolbar icons. */
  const QColor iconColor = p.icon;
  ui->buttonSettings->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Settings, iconColor, 22)));
  ui->buttonViewLayout->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::KeyboardShow, iconColor, 22)));
  ui->buttonSetLayout->setIcon(QIcon(MatraKiIcons::combined(
      {{MatraKiIcons::Keyboard, 20}, {MatraKiIcons::ChevronDown, 12}}, iconColor, 2)));
  ui->buttonShutdown->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Close, iconColor, 18)));

  /* Hairlines between the toolbar groups. */
  const QString line = QStringLiteral("background-color: %1; border: none;").arg(p.border);
  ui->separator1Line->setStyleSheet(line);
  ui->separator2Line->setStyleSheet(line);

  if (layoutMenu) {
    layoutMenu->setStyleSheet(ThemeManager::instance().popupMenuStyle());
  }
  if (iconMenu) {
    iconMenu->setStyleSheet(ThemeManager::instance().iconMenuStyle());
  }
  if (harperToggle) {
    harperToggle->setOnColor(p.accent);
    harperToggle->setOffColor(p.dark ? p.border : p.keyBorder);
    harperToggle->setFocusColor(p.accent);
  }

  refreshMenuIcons();
}

void TopBar::refreshMenuIcons() {
  const MatraKiPalette p = ThemeManager::instance().palette();
  Q_UNUSED(p);
  /* The menu rows paint their own icons, so only the toggle needs a
     re-tint when the theme flips. */
}

/* ------------------------------------------------------------------ */
/*  Popup menus                                                       */
/* ------------------------------------------------------------------ */

void TopBar::SetupPopupMenus() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  /* ---- Layout picker ---- */
  layoutMenu = new MatraKiMenu(QStringLiteral("কীবোর্ড লেআউট নির্বাচন করুন"), this);
  layoutMenu->setRowWidth(268);
  layoutMenu->setStyleSheet(ThemeManager::instance().popupMenuStyle());

  /* ---- Logo menu ---- */
  iconMenu = new MatraKiMenu(QStringLiteral("মাত্রা-কী"), this);
  iconMenu->setRowWidth(288);
  iconMenu->setStyleSheet(ThemeManager::instance().iconMenuStyle());

  iconMenuLayout = iconMenu->addRow(
      QStringLiteral("বর্তমান কীবোর্ড লেআউট সম্পর্কে"),
      MatraKiIcons::pixmap(MatraKiIcons::Keyboard, p.icon, 18));
  connect(iconMenuLayout, &QAction::triggered, this, &TopBar::iconMenuLayout_clicked);

  iconMenuAbout = iconMenu->addRow(
      QStringLiteral("মাত্রা-কী সম্পর্কে"),
      MatraKiIcons::pixmap(MatraKiIcons::Info, p.icon, 18));
  connect(iconMenuAbout, &QAction::triggered, this, &TopBar::iconMenuAbout_clicked);

  iconMenu->addSeparatorRow();

  iconMenuWebsite = iconMenu->addRow(
      QStringLiteral("অফিসিয়াল ওয়েবসাইট"),
      MatraKiIcons::pixmap(MatraKiIcons::World, p.icon, 18),
      QStringLiteral("লাইভ কীবোর্ড"));
  connect(iconMenuWebsite, &QAction::triggered, this, [=]() {
    QDesktopServices::openUrl(QUrl(WEBSITE_URL));
  });

  iconMenuHarper = iconMenu->addToggleRow(
      QStringLiteral("হার্পার ব্যাকরণ ও বানান যাচাই"),
      MatraKiIcons::pixmap(MatraKiIcons::SpellCheck, p.icon, 18), harperToggle);
  connect(harperToggle, &MatraKiToggle::toggled, this, [this](bool on) {
    harperDialog->setHarperEnabled(on);
  });

  iconMenu->addSeparatorRow();

  iconMenuDonate = iconMenu->addRow(
      QStringLiteral("সহায়তা ও ডোনেশন"),
      MatraKiIcons::pixmap(MatraKiIcons::Heart, p.danger, 18));
  connect(iconMenuDonate, &QAction::triggered, this, &TopBar::iconMenuDonate_clicked);

  iconMenuUpdate = iconMenu->addRow(
      QStringLiteral("আপডেট চেক করুন"),
      MatraKiIcons::pixmap(MatraKiIcons::Refresh, p.icon, 18));
  connect(iconMenuUpdate, &QAction::triggered, this, [=]() {
    checkForUpdate(false);
  });

  RefreshLayouts();
}

void TopBar::SetupTrayIcon() {
#if 0
  /* TODO: Fix Crash... */
  tray = new QSystemTrayIcon(QIcon(":/images/keyboard_layout_viewer.png"), this);
  tray->setToolTip("MatraKi");

  /* Tray Menu */
  trayMenuRestore = new QAction("Restore TopBar", this);
  connect(trayMenuRestore, SIGNAL(triggered()), this, SLOT(trayMenuRestore_clicked()));

  trayMenu = new QMenu(this);
  trayMenu->addAction(trayMenuRestore);
  trayMenu->addMenu(layoutMenu); // Layout Menu
  trayMenu->addSeparator();
  trayMenu->addAction(iconMenuQuit);

  tray->setContextMenu(trayMenu);
#endif
}

void TopBar::RefreshLayouts() {
  LayoutList list = gLayout->searchLayouts();

  QString selectedLayout = gSettings->getLayoutName();
  const MatraKiPalette p = ThemeManager::instance().palette();

  /* Rebuild the layout picker from scratch so repeated refreshes never
     duplicate entries. */
  layoutMenu->clearRows();

  QAction *install = layoutMenu->addRow(
      QStringLiteral("নতুন লেআউট ইনস্টল করুন"),
      MatraKiIcons::pixmap(MatraKiIcons::Download, p.icon, 18));
  connect(install, &QAction::triggered, this, &TopBar::layoutMenuInstall_clicked);

  layoutMenu->addSeparatorRow();

  for (int k = 0; k < list.count(); ++k) {
    const QString name = list[k];
    QAction *action = layoutMenu->addCheckableRow(
        name, MatraKiIcons::pixmap(MatraKiIcons::Keyboard, p.icon, 18));
    action->setChecked(name == selectedLayout);
    if (name == selectedLayout) {
      gLayout->setLayout(name);
    }
    connect(action, &QAction::triggered, this, [this, action]() {
      /**
       * From Qt version 5.10, Qt automatically adds shortcuts to menu items.
       * For that Qt includes a `&` character, so the raw text has to be
       * cleaned before it is handed to the layout engine.
       */
      QString layoutName = action->text();
      if (layoutName.contains("&")) layoutName.replace("&", "");

      gLayout->setLayout(layoutName);
      action->setChecked(true);
      layoutViewer->refreshLayoutViewer();
    });
  }
}

void TopBar::layoutMenuInstall_clicked() {
  QString fileName = QFileDialog::getOpenFileName(Q_NULLPTR, "Select Keyboard Layout", QDir::homePath(),
                                                  "Avro Keyboard 5 Keyboard Layout (*.avrolayout)");
  LayoutConverter conv;
  if (fileName.contains(".avrolayout") && fileName != "") {
    ConversionResult res = conv.convertAvroLayout(fileName);
    switch (res) {
    case Ok:
      QMessageBox::information(Q_NULLPTR, "MatraKi", "লেআউট সফলভাবে ইনস্টল হয়েছে।",
                               QMessageBox::Ok);
      break;
    case UnsupportedLayout:
      QMessageBox::critical(Q_NULLPTR,
                            "MatraKi",
                            "অসমর্থিত লেআউট ফাইল!\nমাত্রা-কী শুধুমাত্র Avro Keyboard 5 লেআউট সমর্থন করে।",
                            QMessageBox::Ok);
      break;
    case OpenError:
      QMessageBox::critical(Q_NULLPTR, "MatraKi",
                            "লেআউট ফাইল খোলার সময় একটি ত্রুটি হয়েছে!", QMessageBox::Ok);
      break;
    case SaveError:
      QMessageBox::critical(Q_NULLPTR, "MatraKi", "ফাইল সংরক্ষণের সময় ত্রুটি হয়েছে!",
                            QMessageBox::Ok);
      break;
    }
  }
  RefreshLayouts();
}

void TopBar::iconMenuLayout_clicked() {
  layoutViewer->showLayoutInfoDialog();
}

void TopBar::iconMenuHarper_clicked() {
  harperDialog->show();
  harperDialog->raise();
  harperDialog->activateWindow();
}

void TopBar::iconMenuAbout_clicked() {
  aboutDialog->show();
  aboutDialog->raise();
  aboutDialog->activateWindow();
}

void TopBar::iconMenuDonate_clicked() {
  QDesktopServices::openUrl(QUrl("https://www.supportkori.com/sojib"));
}

void TopBar::iconMenuOnTray_clicked() {
  this->setVisible(false);
  tray->setVisible(true);
  tray->showMessage("MatraKi", "MatraKi is now running on system tray");
}

void TopBar::trayMenuRestore_clicked() {
  tray->setVisible(false);
  this->setVisible(true);
}

void TopBar::on_buttonIcon_clicked() {
  // Check if this is not a position change event. If it is, then ignore it.
  if (!positionChanged) {
    QPoint point = ui->buttonIcon->mapToGlobal(QPoint(0, ui->buttonIcon->height() + 4));
    iconMenu->exec(point);
  }
}

void TopBar::closeEvent(QCloseEvent *event) {
  gSettings->setTopBarWindowPosition(this->pos());
  event->accept();
}

void TopBar::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    canMoveTopbar = true;
    positionChanged = false;
    pressedMouseX = event->x();
    pressedMouseY = event->y();
    event->accept();
  }
}

void TopBar::mouseMoveEvent(QMouseEvent *event) {
  if (canMoveTopbar && (event->buttons() & Qt::LeftButton)) {
    this->setCursor(Qt::ClosedHandCursor);
    move(event->globalX() - pressedMouseX, event->globalY() - pressedMouseY);
    positionChanged = true;
    event->accept();
  }
}

void TopBar::mouseReleaseEvent(QMouseEvent *event) {
  canMoveTopbar = false;
  this->setCursor(Qt::ArrowCursor);
  event->accept();
}

bool TopBar::eventFilter(QObject *object, QEvent *event) {
  if (object == ui->buttonIcon) {
    if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent *e = static_cast<QMouseEvent *>(event);
      if (e->button() == Qt::LeftButton) {
        canMoveTopbar = true;
        positionChanged = false; // reset
        dragStartGlobal = e->globalPos();
        dragWindowStart = pos();
        return true;
      }
    } else if (event->type() == QEvent::MouseMove) {
      if (canMoveTopbar) {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        const QPoint delta = e->globalPos() - dragStartGlobal;
        if (!positionChanged && delta.manhattanLength() > 3) {
          positionChanged = true;
          ui->buttonIcon->setCursor(Qt::ClosedHandCursor);
        }
        if (positionChanged) {
          move(dragWindowStart + delta);
        }
        return true;
      }
    } else if (event->type() == QEvent::MouseButtonRelease) {
      QMouseEvent *e = static_cast<QMouseEvent *>(event);
      if (e->button() == Qt::LeftButton) {
        canMoveTopbar = false;
        ui->buttonIcon->setCursor(Qt::ArrowCursor);
        /* A click (as opposed to a drag) opens the logo menu. */
        if (!positionChanged) {
          on_buttonIcon_clicked();
        }
        return true;
      }
    }
  }

  return QObject::eventFilter(object, event);
}

void TopBar::on_buttonSetLayout_clicked() {
  QPoint point = ui->buttonSetLayout->mapToGlobal(QPoint(0, ui->buttonSetLayout->height() + 4));
  layoutMenu->exec(point);
}

void TopBar::on_buttonShutdown_clicked() {
  gSettings->setTopBarWindowPosition(this->pos());
  QApplication::exit();
}

void TopBar::on_buttonViewLayout_clicked() {
  layoutViewer->refreshLayoutViewer();
  layoutViewer->show();
  layoutViewer->raise();
  layoutViewer->activateWindow();
}

void TopBar::on_buttonSettings_clicked() {
  settingsDialog->updateSettings();
  settingsDialog->show();
  settingsDialog->raise();
  settingsDialog->activateWindow();
}

/**
 * OBK from version 2 onwards reads and stores user data in a different directory
 * the following XDG specification.
 * This function checks and migrates data files into the new user data directory.
 **/
void TopBar::DataMigration() {
  UserFolders usr;
  LayoutConverter converter;
  if(gSettings->getPreviousUserDataRemains()) {
    QDir previousUserDataPath = QDir(environmentVariable("HOME", "") + "/.MatraKi-Keyboard");
    if(previousUserDataPath.exists()) {
      // Handle the data files.
      migrateFile("phonetic-candidate-selection.json", previousUserDataPath, usr.dataPath());
      migrateFile("autocorrect.json", previousUserDataPath, usr.dataPath());
      // Convert old layout files if present.
      previousUserDataPath.cd("Layouts");
      QStringList list = previousUserDataPath.entryList(QStringList("*.json"));
      if(!list.empty()) {
        for(auto& file : list) {
          QString path = previousUserDataPath.path() + "/" + file;

          if(converter.convertLayoutFormat(path) != Ok) {
            QMessageBox::critical(Q_NULLPTR, "MatraKi",
                            QString("An error occurred while converting %1 layout!").arg(file), QMessageBox::Ok);
            return;
          }
        }
      }
      QMessageBox::information(Q_NULLPTR, "MatraKi", "User data files has been migrated successfully.",
                               QMessageBox::Ok);
      // Delete the previous user directory.
      previousUserDataPath.cdUp();
      previousUserDataPath.removeRecursively();
      gSettings->setPreviousUserDataRemains(false);
      RefreshLayouts();
    }
  }
}
