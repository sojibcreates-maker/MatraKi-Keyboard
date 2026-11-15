/*
 *  MatraKi Keyboard
 *  Copyright (C) 2018 Muhammad Mominul Huque <mominul2082@gmail.com>
 *  Copyright (C) 2026 Md Sojib Hossain
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

#include <QTreeWidget>
#include <QPushButton>
#include <QJsonDocument>
#include "AutoCorrectDialog.h"
#include "ui_AutoCorrectDialog.h"
#include "ThemeManager.h"

AutoCorrectDialog::AutoCorrectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AutoCorrectDialog) {
  ui->setupUi(this);
  this->setWindowIcon(QIcon(":/images/MatraKi.png"));
  this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &AutoCorrectDialog::applyTheme);
  applyTheme();

  if (QPushButton *saveBtn = ui->buttonBox->button(QDialogButtonBox::Save)) {
    saveBtn->setText("✓ Save Changes");
    saveBtn->setCursor(Qt::PointingHandCursor);
  }
  if (QPushButton *cancelBtn = ui->buttonBox->button(QDialogButtonBox::Cancel)) {
    cancelBtn->setText("✕ Cancel");
    cancelBtn->setCursor(Qt::PointingHandCursor);
  }

  ui->autoCorrect->setColumnCount(2);
  ui->autoCorrect->setHeaderLabels({"Replace (Phonetic Pattern)", "With (Bangla Output)"});

  ui->btnClear->setEnabled(false);
  ui->btnUpdate->setEnabled(false);
}

AutoCorrectDialog::~AutoCorrectDialog() {
  delete ui;
}

void AutoCorrectDialog::applyTheme() {
  setStyleSheet(ThemeManager::instance().autoCorrectDialogStyle());
}

void AutoCorrectDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  if (!entriesLoaded_) {
    entriesLoaded_ = true;
    loadData();
    loadEntries();
  }
}

void AutoCorrectDialog::loadData() {
  QFile dictFile(AutoCorrectFilePath());
  if (!dictFile.open(QIODevice::ReadOnly)) {
    LOG_ERROR("[AutoCorrect]: Error: Couldn't open autocorrect dictionary file!\n");
  }
  QByteArray data = dictFile.readAll();
  dictFile.close();

  QJsonDocument json(QJsonDocument::fromJson(data));
  dict = json.object();
  
  dictFile.setFileName(folders.getUserAutoCorrectFile());
  if (!dictFile.open(QIODevice::ReadOnly)) {
    LOG_ERROR("[AutoCorrect]: Error: Couldn't open user specific AutoCorrect file!\n");
  }
  data = dictFile.readAll();
  dictFile.close();

  QJsonDocument usrJson(QJsonDocument::fromJson(data));
  usrDict = usrJson.object();
}

QVariantMap AutoCorrectDialog::getEntries() {
  QVariantMap dct = dict.toVariantMap();

  QJsonObject::const_iterator iter = usrDict.constBegin();
  while (iter != usrDict.constEnd()) {
    dct.insert(iter.key(), iter.value());
    ++iter;
  }

  return dct;
}

void AutoCorrectDialog::loadEntries() {
  int items = 0;
  const QSignalBlocker blocker(ui->autoCorrect);
  ui->autoCorrect->setUpdatesEnabled(false);
  ui->autoCorrect->clearSelection();
  ui->autoCorrect->clear();

  QVariantMap acList = getEntries();
  QList<QTreeWidgetItem *> treeItems;
  treeItems.reserve(acList.size());
  QVariantMap::const_iterator iter = acList.constBegin();
  while (iter != acList.constEnd()) {
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, iter.key());
    item->setText(1, iter.value().toString());
    treeItems.append(item);
    ++iter;
    ++items;
  }
  ui->autoCorrect->addTopLevelItems(treeItems);

  ui->autoCorrect->sortItems(0, Qt::SortOrder::AscendingOrder);
  ui->autoCorrect->setUpdatesEnabled(true);
  ui->lblEntries->setText("Total Trained Dictionary Entries: " + QString::number(items));
}

void AutoCorrectDialog::addEntryInViewer(QString replace, QString with) {
  QTreeWidgetItem *item = new QTreeWidgetItem(ui->autoCorrect);
  item->setText(0, replace);
  item->setText(1, with);
}

void AutoCorrectDialog::on_buttonBox_accepted() {
  QFile saveFile(folders.getUserAutoCorrectFile());
  if (!saveFile.open(QIODevice::WriteOnly)) {
    LOG_ERROR("[AutoCorrect:Save]: Error couldn't open save file.\n");
    return;
  }

  QJsonDocument json(usrDict);
  saveFile.write(json.toJson());
  saveFile.close();
}

void AutoCorrectDialog::on_buttonBox_rejected() {
  AutoCorrectDialog::close();
}

void AutoCorrectDialog::on_btnUpdate_clicked() {
  QString key = ui->txtReplace->text().trimmed();
  QString val = ui->txtWith->text().trimmed();
  if (key.isEmpty() || val.isEmpty()) return;

  usrDict.insert(key, val);
  const QSignalBlocker blocker(ui->autoCorrect);
  ui->autoCorrect->setUpdatesEnabled(false);
  QList<QTreeWidgetItem *> matches = ui->autoCorrect->findItems(key, Qt::MatchExactly, 0);
  if (!matches.isEmpty()) {
    matches.first()->setText(1, val);
    ui->autoCorrect->setCurrentItem(matches.first());
  } else {
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, key);
    item->setText(1, val);
    ui->autoCorrect->addTopLevelItem(item);
    ui->autoCorrect->sortItems(0, Qt::SortOrder::AscendingOrder);
    ui->autoCorrect->setCurrentItem(item);
  }
  ui->autoCorrect->setUpdatesEnabled(true);
  ui->lblEntries->setText("Total Trained Dictionary Entries: " + QString::number(ui->autoCorrect->topLevelItemCount()));
}

void AutoCorrectDialog::on_txtSearch_textChanged(const QString &arg1) {
  QString q = arg1.trimmed();
  const QSignalBlocker blocker(ui->autoCorrect);
  ui->autoCorrect->setUpdatesEnabled(false);
  int total = ui->autoCorrect->topLevelItemCount();
  int visible = 0;
  for (int i = 0; i < total; ++i) {
    QTreeWidgetItem *item = ui->autoCorrect->topLevelItem(i);
    bool match = q.isEmpty() ||
                 item->text(0).contains(q, Qt::CaseInsensitive) ||
                 item->text(1).contains(q, Qt::CaseInsensitive);
    item->setHidden(!match);
    if (match) ++visible;
  }
  ui->autoCorrect->setUpdatesEnabled(true);
  if (q.isEmpty()) {
    ui->lblEntries->setText("Total Trained Dictionary Entries: " + QString::number(total));
  } else {
    ui->lblEntries->setText("Matching Entries: " + QString::number(visible) + " / " + QString::number(total));
  }
}

void AutoCorrectDialog::on_btnClear_clicked() {
  ui->txtReplace->setText("");
  ui->txtWith->setText("");
}

void AutoCorrectDialog::on_txtReplace_textChanged(const QString &arg1) {
  if (arg1 != "") {
    ui->lblPreviewR->setText(phonetic.convert(arg1));
    if (!ui->btnClear->isEnabled()) {
      ui->btnClear->setEnabled(true);
    }
    ui->btnUpdate->setEnabled(ui->txtWith->text() != "");
  } else {
    ui->lblPreviewR->setText("");
    ui->btnClear->setEnabled(false);
    ui->btnUpdate->setEnabled(false);
  }
}

void AutoCorrectDialog::on_txtWith_textChanged(const QString &arg1) {
  if (arg1 != "") {
    if (ui->txtReplace->text() == arg1) {
      ui->lblPreviewW->setText(arg1);
    } else {
      ui->lblPreviewW->setText(phonetic.convert(arg1));
    }
    if (!ui->btnClear->isEnabled())
      ui->btnClear->setEnabled(true);
    ui->btnUpdate->setEnabled(ui->txtReplace->text() != "");
  } else {
    ui->lblPreviewW->setText("");
    ui->btnClear->setEnabled(false);
    ui->btnUpdate->setEnabled(false);
  }
}

void AutoCorrectDialog::on_autoCorrect_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
  Q_UNUSED(previous);
  if (!current) return;
  ui->txtReplace->setText(current->text(0));
  ui->txtWith->setText(current->text(1));
}
