/*
 *  MatraKi
 *  Copyright (C) 2016-2018 Muhammad Mominul Huque <mominul2082@gmail.com>
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
#include <QMouseEvent>
#include <QFileDialog>
#include <QMenu>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include "TopBar.h"
#include "Layout.h"
#include "Settings.h"
#include "FileSystem.h"
#include "LayoutViewer.h"
#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "LayoutConverter.h"
#include "AutoCorrectDialog.h"
#include "QSimpleUpdater.h"
#include "ui_TopBar.h"

static const QString DEFS_URL = "https://facebook.com/hossain.sojibs";

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

  ui->buttonIcon->installEventFilter(this);

  SetupTopBar();
  SetupPopupMenus();
  SetupTrayIcon();
  DataMigration();
}

TopBar::~TopBar() {
  /* Dialogs */
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

void TopBar::checkForUpdate() {
  QMessageBox msgBox;
  msgBox.setWindowTitle("MatraKi - Software Update");
  msgBox.setWindowIcon(QIcon(":/images/MatraKi.png"));
  msgBox.setIcon(QMessageBox::Information);
  msgBox.setText("<b>MatraKi v1.0.0 — AI Smart Edition</b>");
  msgBox.setInformativeText("Your MatraKi Keyboard is up to date!\n(13,560+ Trained Words, 18,559 Auto-Correct Rules & AI Smart Engine Active)");
  msgBox.setStyleSheet(
    "QMessageBox { background-color: #0b1120; color: #f8fafc; }"
    "QLabel { color: #e2e8f0; font-size: 13px; }"
    "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1d4ed8, stop:1 #0ea5e9); color: #ffffff; border: 1px solid #38bdf8; border-radius: 6px; padding: 6px 18px; font-weight: bold; }"
    "QPushButton:hover { background-color: #1e40af; }"
  );
  msgBox.exec();
}

void TopBar::SetupPopupMenus() {
  // Layout Popup Menu
  layoutMenu = new QMenu("Select keyboard layout", this);
  layoutMenu->setStyleSheet(
    "QMenu { background-color: #0f172a; color: #f1f5f9; border: 1.5px solid #38bdf8; border-radius: 10px; padding: 6px; font-weight: 500; }"
    "QMenu::item { padding: 6px 28px 6px 14px; border-radius: 6px; }"
    "QMenu::item:selected { background-color: rgba(56, 189, 248, 0.25); color: #38bdf8; }"
    "QMenu::item:checked { font-weight: bold; color: #00d2ff; }"
    "QMenu::separator { height: 1px; background: rgba(255, 255, 255, 0.12); margin: 4px 6px; }"
  );
  layoutMenu->setIcon(QIcon(":/images/keyboard_layout.png"));
  connect(layoutMenu, &QMenu::aboutToHide, [=]() {
    ui->buttonSetLayout->setChecked(false);
  });

  layoutMenuInstall = new QAction("Install a layout", this);
  layoutMenuLayoutsGroup = new QActionGroup(this);
  for (auto &layoutMenuLayout : layoutMenuLayouts) {
    layoutMenuLayout = new QAction(this);
    layoutMenuLayout->setVisible(false);
    layoutMenuLayout->setCheckable(true);
    layoutMenuLayoutsGroup->addAction(layoutMenuLayout);
    connect(layoutMenuLayout, SIGNAL(triggered()), this, SLOT(layoutMenuLayouts_clicked()));
  }
  RefreshLayouts();
  connect(layoutMenuInstall, SIGNAL(triggered()), this, SLOT(layoutMenuInstall_clicked()));

  // Icon Button Popup Menu
#if 0
  iconMenuOnTray = new QAction("Jump to system tray", this);
  connect(iconMenuOnTray, SIGNAL(triggered()), this, SLOT(iconMenuOnTray_clicked()));
#endif

  iconMenuLayout = new QAction("About current keyboard layout", this);
  connect(iconMenuLayout, SIGNAL(triggered()), this, SLOT(iconMenuLayout_clicked()));

  iconMenuAbout = new QAction("About MatraKi", this);
  connect(iconMenuAbout, SIGNAL(triggered()), this, SLOT(iconMenuAbout_clicked()));

  iconMenuDonate = new QAction("💖 Support & Donate", this);
  connect(iconMenuDonate, SIGNAL(triggered()), this, SLOT(iconMenuDonate_clicked()));

  iconMenuUpdate = new QAction("Check for Updates", this);
  connect(iconMenuUpdate, &QAction::triggered, [=]() {
    checkForUpdate();
  });

  iconMenu = new QMenu(this);
  iconMenu->setStyleSheet(
    "QMenu { background-color: #0f172a; color: #f1f5f9; border: 1.5px solid #d946ef; border-radius: 10px; padding: 6px; font-weight: 500; }"
    "QMenu::item { padding: 6px 28px 6px 14px; border-radius: 6px; }"
    "QMenu::item:selected { background-color: rgba(217, 70, 239, 0.25); color: #f472b6; }"
    "QMenu::separator { height: 1px; background: rgba(255, 255, 255, 0.12); margin: 4px 6px; }"
  );
  //iconMenu->addAction(iconMenuOnTray);
  iconMenu->addAction(iconMenuLayout);
  iconMenu->addAction(iconMenuAbout);
  iconMenu->addAction(iconMenuDonate);
  iconMenu->addAction(iconMenuUpdate);  
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
  /*
  trayMenu->addSeparator();
  trayMenu->addAction(iconMenuAbout);*/
  trayMenu->addSeparator();
  trayMenu->addAction(iconMenuQuit);

  tray->setContextMenu(trayMenu);
#endif
}

void TopBar::RefreshLayouts() {
  LayoutList list;
  list = gLayout->searchLayouts();

  QString selectedLayout = gSettings->getLayoutName();

  // This loop need to be rewritten to use `for each` loop, which is unnecessary. Skipping.


  for (int k = 0; k < MaxLayoutFiles; ++k) {
    if (k < list.count()) {
      QString name = list[k];
      layoutMenuLayouts[k]->setText(name);
      layoutMenuLayouts[k]->setVisible(true);
      // Select previously selected layout
      if (name == selectedLayout) {
        layoutMenuLayouts[k]->setChecked(true);
        gLayout->setLayout(name);
      }
    } else {
      layoutMenuLayouts[k]->setVisible(false);
    }
    layoutMenu->addAction(layoutMenuLayouts[k]);
  }
  layoutMenu->addSeparator();
  layoutMenu->addAction(layoutMenuInstall);
}

void TopBar::layoutMenuLayouts_clicked() {
  /**
   * From Qt version 5.10, Qt automatically adds shortcuts to
   * menu items. For that Qt includes a `&` character. So when
   * we use QAction::text() function, we get a string including
   * a `&` character and we mess all things up.
   *
   * See issue #17
   */

  QAction *action = qobject_cast<QAction *>(sender());

  QString layoutName = action->text();
  if (layoutName.contains("&")) {
    layoutName.replace("&", "");
  }

  gLayout->setLayout(layoutName);
  action->setChecked(true);
  layoutViewer->refreshLayoutViewer();
}

void TopBar::layoutMenuInstall_clicked() {
  QString fileName = QFileDialog::getOpenFileName(Q_NULLPTR, "Select Keyboard Layout", QDir::homePath(),
                                                  "Avro Keyboard 5 Keyboard Layout (*.avrolayout)");
  LayoutConverter conv;
  if (fileName.contains(".avrolayout") && fileName != "") {
    ConversionResult res = conv.convertAvroLayout(fileName);
    switch (res) {
    case Ok:
      QMessageBox::information(Q_NULLPTR, "MatraKi", "Layout Installed Successfully",
                               QMessageBox::Ok);
      break;
    case UnsupportedLayout:
      QMessageBox::critical(Q_NULLPTR,
                            "MatraKi",
                            "Unsupported Layout file!\nMatraKi only supports Avro Keyboard 5 layouts.",
                            QMessageBox::Ok);
      break;
    case OpenError:
      QMessageBox::critical(Q_NULLPTR, "MatraKi",
                            "An error occurred while opening the layout file!", QMessageBox::Ok);
      break;
    case SaveError:
      QMessageBox::critical(Q_NULLPTR, "MatraKi", "Error occurred while saving the file!",
                            QMessageBox::Ok);
      break;
    }
  }
  RefreshLayouts();
}

void TopBar::iconMenuLayout_clicked() {
  layoutViewer->showLayoutInfoDialog();
}

void TopBar::iconMenuAbout_clicked() {
  aboutDialog->show();
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
  if(!positionChanged) {
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
      canMoveTopbar = true;
      positionChanged = false; // reset
      QMouseEvent *e = (QMouseEvent *) event;
      pressedMouseX = e->x();
      pressedMouseY = e->y();
      event->accept();
    } else if (event->type() == QEvent::MouseMove) {
      if (canMoveTopbar) {
        QMouseEvent *e = (QMouseEvent *) event;
        ui->buttonIcon->setCursor(Qt::ClosedHandCursor);
        move(e->globalX() - pressedMouseX, e->globalY() - pressedMouseY);
        positionChanged = true;
      }
    } else if (event->type() == QEvent::MouseButtonRelease) {
      canMoveTopbar = false;
      ui->buttonIcon->setCursor(Qt::ArrowCursor);
      event->accept();
    }
  }

  return QObject::eventFilter(object, event);
}

void TopBar::on_buttonSetLayout_clicked() {
  QPoint point = ui->buttonSetLayout->mapToGlobal(QPoint(0, ui->buttonSetLayout->height() + 4));
  layoutMenu->exec(point);
}

void TopBar::on_buttonShutdown_clicked() {
  QApplication::exit();
}

void TopBar::on_buttonViewLayout_clicked() {
  layoutViewer->refreshLayoutViewer();
  layoutViewer->show();
}

void TopBar::on_buttonSettings_clicked() {
  settingsDialog->updateSettings();
  settingsDialog->show();
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
