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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QTemporaryFile>
#include <QProcess>
#include <QTextStream>
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

  ui->buttonIcon->installEventFilter(this);

  SetupTopBar();
  SetupPopupMenus();
  SetupTrayIcon();
  DataMigration();

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

void TopBar::showUpdateDialog(bool updateAvailable, const QString &latestVer, const QString &releaseTitle, const QString &changelog, const QString &releaseUrl, bool silent) {
  if (!updateAvailable && silent) {
    return;
  }

  QMessageBox msgBox(this);
  msgBox.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
  msgBox.setWindowTitle(updateAvailable ? "MatraKi — New Update Available!" : "MatraKi — Software Update");
  msgBox.setWindowIcon(QIcon(":/images/MatraKi.png"));
  msgBox.setIcon(QMessageBox::Information);
  msgBox.setTextFormat(Qt::RichText);

  if (updateAvailable) {
    msgBox.setText(QString(
      "<span style='font-size:14pt; font-weight:800; color:#38bdf8;'>🎉 নতুন আপডেট পাওয়া গেছে! (Update Available: v%1)</span><br/>"
      "<span style='font-size:10pt; color:#cbd5e1;'>Current Installed: <b>v%2</b>  •  Latest Release: <b style='color:#34d399;'>v%1 (%3)</b></span>"
    ).arg(latestVer, QStringLiteral(PROJECT_VERSION), releaseTitle));

    QString summary = changelog.trimmed();
    if (summary.length() > 360) summary = summary.left(360) + "...";
    msgBox.setInformativeText(QString(
      "<b>Release Highlights:</b><br/>%1<br/><br/>"
      "Click below to download the latest installer from the official MatraKi web page or GitHub Releases."
    ).arg(summary.toHtmlEscaped().replace("\n", "<br/>")));
  } else {
    msgBox.setText(QString(
      "<span style='font-size:13.5pt; font-weight:800; color:#34d399;'>✅ MatraKi v%1 — AI Smart Edition is Up to Date!</span>"
    ).arg(QStringLiteral(PROJECT_VERSION)));
    msgBox.setInformativeText(
      "You are running the latest version of MatraKi Keyboard.<br/>"
      "• <b>20,535+</b> Pre-Trained Words &amp; Auto-Correct Rules<br/>"
      "• <b>Smart Suffix Stemmer</b>, Next-Word AI, Live Math (<code>=</code>) &amp; 42+ Snippets (<code>!</code>)<br/>"
      "• Connected to Official Update Page: <a style='color:#38bdf8;' href='https://sojibcreates-maker.github.io/MatraKi-Keyboard/'>sojibcreates-maker.github.io/MatraKi-Keyboard</a>"
    );
  }

  QPushButton *btnWeb = msgBox.addButton(
    updateAvailable ? "🌐 Download from Web Page" : "🌐 Official Web Page",
    QMessageBox::AcceptRole
  );
  QPushButton *btnGitHub = msgBox.addButton("📦 GitHub Releases", QMessageBox::ActionRole);
  msgBox.addButton(updateAvailable ? "Later" : "Close", QMessageBox::RejectRole);

  msgBox.setStyleSheet(
    "QMessageBox { background-color: #0b1120; color: #f8fafc; }"
    "QLabel { color: #e2e8f0; font-size: 13px; }"
    "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1d4ed8, stop:1 #0ea5e9); color: #ffffff; border: 1px solid #38bdf8; border-radius: 7px; padding: 7px 16px; font-weight: bold; }"
    "QPushButton:hover { background-color: #1e40af; }"
  );

  msgBox.exec();
  if (msgBox.clickedButton() == btnWeb) {
    QDesktopServices::openUrl(QUrl(UPDATE_PAGE_URL));
  } else if (msgBox.clickedButton() == btnGitHub) {
    QDesktopServices::openUrl(QUrl(releaseUrl.isEmpty() ? GITHUB_RELEASES_URL : releaseUrl));
  }
}

void TopBar::checkForUpdate(bool silent) {
  // Also keep QSimpleUpdater synced with our GitHub Pages updates.json URL
  if (updater) {
    updater->setModuleVersion(DEFS_URL, PROJECT_VERSION);
  }

  QNetworkAccessManager *nam = new QNetworkAccessManager(this);

  // Step 1: Check GitHub Releases API (detects any newly published GitHub release immediately)
  QNetworkRequest apiReq{QUrl(GITHUB_API_LATEST_URL)};
  apiReq.setRawHeader("User-Agent", "MatraKi-Keyboard-Updater");
  apiReq.setRawHeader("Accept", "application/vnd.github+json");
  QNetworkReply *apiReply = nam->get(apiReq);

  connect(apiReply, &QNetworkReply::finished, this, [=]() {
    apiReply->deleteLater();
    if (apiReply->error() == QNetworkReply::NoError) {
      QJsonDocument doc = QJsonDocument::fromJson(apiReply->readAll());
      if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QString tag = obj.value("tag_name").toString().trimmed();
        if (tag.startsWith('v') || tag.startsWith('V')) tag = tag.mid(1);
        QString title = obj.value("name").toString("MatraKi Release").trimmed();
        QString body = obj.value("body").toString().trimmed();
        QString htmlUrl = obj.value("html_url").toString(GITHUB_RELEASES_URL).trimmed();
        if (!tag.isEmpty()) {
          bool newer = isRemoteVersionNewer(tag, QStringLiteral(PROJECT_VERSION));
          showUpdateDialog(newer, tag, title, body, htmlUrl, silent);
          nam->deleteLater();
          return;
        }
      }
    }

    // Step 2: Fallback to GitHub Pages updates.json feed
    QNetworkRequest pageReq{QUrl(DEFS_URL)};
    pageReq.setRawHeader("User-Agent", "MatraKi-Keyboard-Updater");
    QNetworkReply *pageReply = nam->get(pageReq);
    connect(pageReply, &QNetworkReply::finished, this, [=]() {
      pageReply->deleteLater();
      nam->deleteLater();
      if (pageReply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(pageReply->readAll());
        if (doc.isObject()) {
          QJsonObject obj = doc.object();
          QString latestVer = obj.value("latest_version").toString().trimmed();
          if (latestVer.isEmpty()) {
            latestVer = obj.value("updates").toObject().value("linux").toObject().value("latest-version").toString().trimmed();
          }
          if (latestVer.startsWith('v') || latestVer.startsWith('V')) latestVer = latestVer.mid(1);
          QString title = obj.value("release_title").toString("MatraKi Release");
          QString changelog = obj.value("changelog").toString();
          QString relUrl = obj.value("github_release_url").toString(GITHUB_RELEASES_URL);
          if (!latestVer.isEmpty()) {
            bool newer = isRemoteVersionNewer(latestVer, QStringLiteral(PROJECT_VERSION));
            showUpdateDialog(newer, latestVer, title, changelog, relUrl, silent);
            return;
          }
        }
      }
      // Offline or unreachable fallback (only show if user manually clicked Check for Updates)
      showUpdateDialog(false, QStringLiteral(PROJECT_VERSION), "MatraKi v1.0.0 — AI Smart Edition", "", GITHUB_RELEASES_URL, silent);
    });
  });
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

  iconMenuWebsite = new QAction("🌐 Official Website & Live Keyboard", this);
  connect(iconMenuWebsite, &QAction::triggered, [=]() {
    QDesktopServices::openUrl(QUrl(WEBSITE_URL));
  });

  iconMenuHarper = new QAction("🧠 Harper Grammar & Spell Checker (Automattic)", this);
  connect(iconMenuHarper, SIGNAL(triggered()), this, SLOT(iconMenuHarper_clicked()));

  iconMenuDonate = new QAction("💖 Support & Donate", this);
  connect(iconMenuDonate, SIGNAL(triggered()), this, SLOT(iconMenuDonate_clicked()));

  iconMenuUpdate = new QAction("🔄 Check for Updates", this);
  connect(iconMenuUpdate, &QAction::triggered, [=]() {
    checkForUpdate(false);
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
  iconMenu->addAction(iconMenuWebsite);
  iconMenu->addAction(iconMenuHarper);
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

void TopBar::iconMenuHarper_clicked() {
  QDialog *dlg = new QDialog(this);
  dlg->setWindowTitle("Automattic Harper™ — Grammar & Spell Checker");
  dlg->setWindowIcon(QIcon(":/images/MatraKi.png"));
  dlg->resize(680, 500);
  dlg->setStyleSheet(
    "QDialog { background-color: #0b1120; color: #f8fafc; font-family: sans-serif; }"
    "QLabel { color: #cbd5e1; font-size: 13px; }"
    "QTextEdit { background-color: #060b16; border: 1px solid rgba(56, 189, 248, 0.4); border-radius: 8px; color: #f8fafc; font-size: 13px; padding: 8px; selection-background-color: #1d4ed8; }"
    "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1d4ed8, stop:1 #0ea5e9); color: #ffffff; border: 1px solid #38bdf8; border-radius: 7px; padding: 7px 15px; font-weight: bold; font-size: 12px; }"
    "QPushButton:hover { background-color: #1e40af; }"
    "QPushButton:pressed { background-color: #0c4a6e; }"
  );

  QVBoxLayout *layout = new QVBoxLayout(dlg);
  layout->setContentsMargins(18, 18, 18, 18);
  layout->setSpacing(12);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *iconLabel = new QLabel("🧠", dlg);
  iconLabel->setStyleSheet("font-size: 26px;");
  QLabel *titleLabel = new QLabel(
    "<b><span style='font-size: 15px; color: #38bdf8;'>Automattic Harper™ Grammar &amp; Spell Checker</span></b><br/>"
    "<span style='font-size: 11px; color: #94a3b8;'>Official integration with Automattic's high-speed Rust grammar engine (v3.1.0)</span>",
    dlg
  );
  headerLayout->addWidget(iconLabel);
  headerLayout->addWidget(titleLabel, 1);
  layout->addLayout(headerLayout);

  QLabel *inputPrompt = new QLabel("Type or paste text to check for grammar, spelling, and style errors:", dlg);
  layout->addWidget(inputPrompt);

  QTextEdit *txtInput = new QTextEdit(dlg);
  txtInput->setPlaceholderText("e.g. This is an test with a apple and alot of issues in monday.");
  txtInput->setText("This is an test with a apple and alot of issues in monday. I want to to say that their is no problem.");
  txtInput->setFixedHeight(105);
  layout->addWidget(txtInput);

  // Buttons row
  QHBoxLayout *btnLayout = new QHBoxLayout();
  QPushButton *btnCheck = new QPushButton("🔍 Run Harper Grammar Check", dlg);
  QPushButton *btnSample = new QPushButton("⚡ Test Sample", dlg);
  QPushButton *btnWeb = new QPushButton("🌐 Open Web Studio", dlg);
  btnLayout->addWidget(btnCheck);
  btnLayout->addWidget(btnSample);
  btnLayout->addWidget(btnWeb);
  btnLayout->addStretch(1);
  layout->addLayout(btnLayout);

  QLabel *resultLabel = new QLabel("<b>Harper Inspection Output (CLI):</b>", dlg);
  layout->addWidget(resultLabel);

  QTextEdit *txtOutput = new QTextEdit(dlg);
  txtOutput->setReadOnly(true);
  txtOutput->setStyleSheet("background-color: #030712; border: 1px solid rgba(99, 102, 241, 0.4); border-radius: 8px; font-family: monospace; font-size: 12px; color: #cbd5e1;");
  txtOutput->setPlaceholderText("Click 'Run Harper Grammar Check' to inspect text using harper-cli...");
  layout->addWidget(txtOutput, 1);

  auto runCheck = [=]() {
    QString text = txtInput->toPlainText().trimmed();
    if (text.isEmpty()) {
      txtOutput->setPlainText("Please enter some text to check.");
      return;
    }
    QTemporaryFile tempFile;
    if (tempFile.open()) {
      QTextStream out(&tempFile);
      out << text << "\n";
      tempFile.close();

      QProcess proc;
      proc.start("harper-cli", QStringList() << "--no-color" << "lint" << tempFile.fileName());
      proc.waitForFinished(5000);
      QString output = QString::fromUtf8(proc.readAllStandardOutput());
      QString errOutput = QString::fromUtf8(proc.readAllStandardError());
      QString combined = (output + "\n" + errOutput).trimmed();

      if (proc.exitCode() == 0 && !combined.contains("Advice:")) {
        txtOutput->setHtml(
          "<p style='color:#34d399; font-weight:bold; font-size:13px;'>✅ No grammar or spelling issues detected!</p>"
          "<p style='color:#94a3b8;'>Your text looks completely clean and passes all Automattic Harper rules.</p>"
        );
      } else {
        txtOutput->setPlainText(combined);
      }
    }
  };

  connect(btnCheck, &QPushButton::clicked, runCheck);
  connect(btnSample, &QPushButton::clicked, [=]() {
    txtInput->setText("This is an test with a apple and alot of issues in monday. I want to to say that their is no problem.");
    runCheck();
  });
  connect(btnWeb, &QPushButton::clicked, [=]() {
    QDesktopServices::openUrl(QUrl("https://sojibcreates-maker.github.io/MatraKi-Keyboard/#demo"));
  });

  // Run automatically on first open
  runCheck();

  dlg->setAttribute(Qt::WA_DeleteOnClose);
  dlg->show();
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
