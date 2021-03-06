#include "Controls.h"

#include <GitBase.h>
#include <GitStashes.h>
#include <GitQlientStyles.h>
#include <GitRemote.h>
#include <GitConfig.h>
#include <BranchDlg.h>
#include <RepoConfigDlg.h>
#include <ServerConfigDlg.h>
#include <CreateIssueDlg.h>
#include <CreatePullRequestDlg.h>
#include <QLogger.h>

#include <QApplication>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProgressBar>
#include <QFile>
#include <QStandardPaths>

using namespace QLogger;

Controls::Controls(const QSharedPointer<RevisionsCache> &cache, const QSharedPointer<GitBase> &git, QWidget *parent)
   : QFrame(parent)
   , mCache(cache)
   , mGit(git)
   , mHistory(new QToolButton())
   , mDiff(new QToolButton())
   , mBlame(new QToolButton())
   , mPullBtn(new QToolButton())
   , mPushBtn(new QToolButton())
   , mStashBtn(new QToolButton())
   , mRefreshBtn(new QToolButton())
   , mConfigBtn(new QToolButton())
   , mGitPlatform(new QToolButton())
   , mVersionCheck(new QToolButton())
   , mDownloadLog(new QProgressBar())
   , mMergeWarning(new QPushButton(tr("WARNING: There is a merge pending to be commited! Click here to solve it.")))
   , mManager(new QNetworkAccessManager())
{
   setAttribute(Qt::WA_DeleteOnClose);

   mHistory->setCheckable(true);
   mHistory->setIcon(QIcon(":/icons/git_orange"));
   mHistory->setIconSize(QSize(22, 22));
   mHistory->setText("View");
   mHistory->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

   mDiff->setCheckable(true);
   mDiff->setIcon(QIcon(":/icons/diff"));
   mDiff->setIconSize(QSize(22, 22));
   mDiff->setText("Diff");
   mDiff->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
   mDiff->setEnabled(false);

   mBlame->setCheckable(true);
   mBlame->setIcon(QIcon(":/icons/blame"));
   mBlame->setIconSize(QSize(22, 22));
   mBlame->setText("Blame");
   mBlame->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

   const auto menu = new QMenu(mPullBtn);

   auto action = menu->addAction(tr("Fetch all"));
   connect(action, &QAction::triggered, this, &Controls::fetchAll);

   action = menu->addAction(tr("Pull"));
   connect(action, &QAction::triggered, this, &Controls::pullCurrentBranch);
   mPullBtn->setDefaultAction(action);

   action = menu->addAction(tr("Prune"));
   connect(action, &QAction::triggered, this, &Controls::pruneBranches);
   menu->addSeparator();

   mPullBtn->setMenu(menu);
   mPullBtn->setIconSize(QSize(22, 22));
   mPullBtn->setText(tr("Pull"));
   mPullBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
   mPullBtn->setPopupMode(QToolButton::MenuButtonPopup);
   mPullBtn->setIcon(QIcon(":/icons/git_pull"));

   mPushBtn->setIcon(QIcon(":/icons/git_push"));
   mPushBtn->setIconSize(QSize(22, 22));
   mPushBtn->setText(tr("Push"));
   mPushBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

   const auto stashMenu = new QMenu(mStashBtn);

   action = stashMenu->addAction(tr("Stash push"));
   connect(action, &QAction::triggered, this, &Controls::stashCurrentWork);
   mStashBtn->setDefaultAction(action);

   action = stashMenu->addAction(tr("Stash pop"));
   connect(action, &QAction::triggered, this, &Controls::popStashedWork);

   mStashBtn->setMenu(stashMenu);
   mStashBtn->setIcon(QIcon(":/icons/git_stash"));
   mStashBtn->setIconSize(QSize(22, 22));
   mStashBtn->setText(tr("Stash"));
   mStashBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
   mStashBtn->setPopupMode(QToolButton::MenuButtonPopup);

   mRefreshBtn->setIcon(QIcon(":/icons/refresh"));
   mRefreshBtn->setIconSize(QSize(22, 22));
   mRefreshBtn->setText(tr("Refresh"));
   mRefreshBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

   mConfigBtn->setIcon(QIcon(":/icons/config"));
   mConfigBtn->setIconSize(QSize(22, 22));
   mConfigBtn->setText(tr("Config"));
   mConfigBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

   const auto verticalFrame = new QFrame();
   verticalFrame->setObjectName("orangeSeparator");

   const auto verticalFrame2 = new QFrame();
   verticalFrame2->setObjectName("orangeSeparator");

   const auto hLayout = new QHBoxLayout();
   hLayout->setContentsMargins(QMargins());
   hLayout->addStretch();
   hLayout->addWidget(mHistory);
   hLayout->addWidget(mDiff);
   hLayout->addWidget(mBlame);
   hLayout->addWidget(verticalFrame);
   hLayout->addWidget(mPullBtn);
   hLayout->addWidget(mPushBtn);
   hLayout->addWidget(mStashBtn);

   QScopedPointer<GitConfig> gitConfig(new GitConfig(mGit));
   const auto remoteUrl = gitConfig->getServerUrl();
   QIcon gitPlatformIcon;
   QString name;
   QString prName;
   auto add = true;

   if (remoteUrl.contains("github", Qt::CaseInsensitive))
   {
      gitPlatformIcon = QIcon(":/icons/github");
      name = "GitHub";
      prName = tr("Pull Request");
   }
   else if (remoteUrl.contains("gitlab", Qt::CaseInsensitive))
   {
      gitPlatformIcon = QIcon(":/icons/gitlab");
      name = "GitLab";
      prName = tr("Merge Request");
   }
   else
      add = false;

   if (add)
   {
      const auto gitMenu = new QMenu(mStashBtn);

      action = gitMenu->addAction(tr("New Issue"));
      connect(action, &QAction::triggered, this, &Controls::createNewIssue);

      action = gitMenu->addAction(tr("New %1").arg(prName));
      connect(action, &QAction::triggered, this, &Controls::createNewPullRequest);

      gitMenu->addSeparator();
      action = gitMenu->addAction(tr("Config server"));
      mGitPlatform->setDefaultAction(action);
      connect(action, &QAction::triggered, this, &Controls::configServer);

      mGitPlatform->setMenu(gitMenu);
      mGitPlatform->setIcon(gitPlatformIcon);
      mGitPlatform->setIconSize(QSize(22, 22));
      mGitPlatform->setText(name);
      mGitPlatform->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
      mGitPlatform->setPopupMode(QToolButton::MenuButtonPopup);

      const auto verticalFrame3 = new QFrame();
      verticalFrame3->setObjectName("orangeSeparator");
      hLayout->addWidget(verticalFrame3);
      hLayout->addWidget(mGitPlatform);

      connect(mGitPlatform, &QToolButton::clicked, this, &Controls::signalGoServer);
      connect(mGitPlatform, &QToolButton::toggled, this, [this](bool checked) {
         mDiff->blockSignals(true);
         mDiff->setChecked(!checked);
         mDiff->blockSignals(false);
         mBlame->blockSignals(true);
         mBlame->setChecked(!checked);
         mBlame->blockSignals(false);
         mHistory->blockSignals(true);
         mHistory->setChecked(!checked);
         mHistory->blockSignals(false);
      });
   }
   else
      mGitPlatform->setVisible(false);

   mVersionCheck->setIcon(QIcon(":/icons/get_gitqlient"));
   mVersionCheck->setIconSize(QSize(22, 22));
   mVersionCheck->setText(tr("New version"));
   mVersionCheck->setObjectName("longToolButton");
   mVersionCheck->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
   mVersionCheck->setVisible(false);

   mDownloadLog->setVisible(false);

   checkNewGitQlientVersion();

   hLayout->addWidget(verticalFrame2);
   hLayout->addWidget(mRefreshBtn);
   hLayout->addWidget(mVersionCheck);
   hLayout->addWidget(mDownloadLog);
   hLayout->addStretch();
   hLayout->addWidget(mConfigBtn);

   mMergeWarning->setObjectName("MergeWarningButton");
   mMergeWarning->setVisible(false);

   const auto vLayout = new QVBoxLayout(this);
   vLayout->setContentsMargins(0, 10, 0, 10);
   vLayout->setSpacing(10);
   vLayout->addLayout(hLayout);
   vLayout->addWidget(mMergeWarning);

   connect(mHistory, &QToolButton::clicked, this, &Controls::signalGoRepo);
   connect(mHistory, &QToolButton::toggled, this, [this](bool checked) {
      mDiff->blockSignals(true);
      mDiff->setChecked(!checked);
      mDiff->blockSignals(false);
      mBlame->blockSignals(true);
      mBlame->setChecked(!checked);
      mBlame->blockSignals(false);
      mGitPlatform->blockSignals(true);
      mGitPlatform->setChecked(!checked);
      mGitPlatform->blockSignals(false);
   });
   connect(mDiff, &QToolButton::clicked, this, &Controls::signalGoDiff);
   connect(mDiff, &QToolButton::toggled, this, [this](bool checked) {
      mHistory->blockSignals(true);
      mHistory->setChecked(!checked);
      mHistory->blockSignals(false);
      mBlame->blockSignals(true);
      mBlame->setChecked(!checked);
      mBlame->blockSignals(false);
      mGitPlatform->blockSignals(true);
      mGitPlatform->setChecked(!checked);
      mGitPlatform->blockSignals(false);
   });
   connect(mBlame, &QToolButton::clicked, this, &Controls::signalGoBlame);
   connect(mBlame, &QToolButton::toggled, this, [this](bool checked) {
      mHistory->blockSignals(true);
      mHistory->setChecked(!checked);
      mHistory->blockSignals(false);
      mDiff->blockSignals(true);
      mDiff->setChecked(!checked);
      mDiff->blockSignals(false);
      mGitPlatform->blockSignals(true);
      mGitPlatform->setChecked(!checked);
      mGitPlatform->blockSignals(false);
   });
   connect(mPushBtn, &QToolButton::clicked, this, &Controls::pushCurrentBranch);
   connect(mRefreshBtn, &QToolButton::clicked, this, &Controls::signalRepositoryUpdated);
   connect(mConfigBtn, &QToolButton::clicked, this, &Controls::showConfigDlg);
   connect(mMergeWarning, &QPushButton::clicked, this, &Controls::signalGoMerge);
   connect(mVersionCheck, &QToolButton::clicked, this, &Controls::showInfoMessage);

   enableButtons(false);
}

void Controls::toggleButton(ControlsMainViews view)
{
   switch (view)
   {
      case ControlsMainViews::HISTORY:
         mHistory->setChecked(true);
         break;
      case ControlsMainViews::DIFF:
         mDiff->setChecked(true);
         break;
      case ControlsMainViews::BLAME:
         mBlame->setChecked(true);
         break;
      case ControlsMainViews::MERGE:
         mHistory->setChecked(true);
         mHistory->blockSignals(true);
         mHistory->setChecked(false);
         mHistory->blockSignals(false);
         break;
      case ControlsMainViews::SERVER:
         mGitPlatform->setChecked(true);
         break;
      default:
         break;
   }
}

void Controls::enableButtons(bool enabled)
{
   mHistory->setEnabled(enabled);
   mBlame->setEnabled(enabled);
   mPullBtn->setEnabled(enabled);
   mPushBtn->setEnabled(enabled);
   mStashBtn->setEnabled(enabled);
   mRefreshBtn->setEnabled(enabled);
   mGitPlatform->setEnabled(enabled);
}

void Controls::pullCurrentBranch()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->pull();
   QApplication::restoreOverrideCursor();

   if (ret.success)
      emit signalRepositoryUpdated();
   else
   {
      const auto errorMsg = ret.output.toString();

      if (errorMsg.contains("error: could not apply", Qt::CaseInsensitive)
          && errorMsg.contains("causing a conflict", Qt::CaseInsensitive))
      {
         emit signalPullConflict();
      }
      else
      {
         QMessageBox msgBox(QMessageBox::Critical, tr("Error while pulling"),
                            QString("There were problems during the pull operation. Please, see the detailed "
                                    "description for more information."),
                            QMessageBox::Ok, this);
         msgBox.setDetailedText(errorMsg);
         msgBox.setStyleSheet(GitQlientStyles::getStyles());
         msgBox.exec();
      }
   }
}

void Controls::fetchAll()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->fetch();
   QApplication::restoreOverrideCursor();

   if (ret)
   {
      emit signalFetchPerformed();
      emit signalRepositoryUpdated();
   }
}

void Controls::activateMergeWarning()
{
   mMergeWarning->setVisible(true);
}

void Controls::disableMergeWarning()
{
   mMergeWarning->setVisible(false);
}

void Controls::disableDiff()
{
   mDiff->setDisabled(true);
}

void Controls::enableDiff()
{
   mDiff->setEnabled(true);
}

ControlsMainViews Controls::getCurrentSelectedButton() const
{
   return mBlame->isChecked() ? ControlsMainViews::BLAME : ControlsMainViews::HISTORY;
}

void Controls::pushCurrentBranch()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->push();
   QApplication::restoreOverrideCursor();

   if (ret.output.toString().contains("has no upstream branch"))
   {
      const auto currentBranch = mGit->getCurrentBranch();
      BranchDlg dlg({ currentBranch, BranchDlgMode::PUSH_UPSTREAM, mGit });
      const auto dlgRet = dlg.exec();

      if (dlgRet == QDialog::Accepted)
      {
         emit signalRefreshPRsCache();
         emit signalRepositoryUpdated();
      }
   }
   else if (ret.success)
   {
      emit signalRefreshPRsCache();
      emit signalRepositoryUpdated();
   }
   else
   {
      QMessageBox msgBox(QMessageBox::Critical, tr("Error while pushing"),
                         QString("There were problems during the push operation. Please, see the detailed description "
                                 "for more information."),
                         QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output.toString());
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}

void Controls::stashCurrentWork()
{
   QScopedPointer<GitStashes> git(new GitStashes(mGit));
   const auto ret = git->stash();

   if (ret.success)
      emit signalRepositoryUpdated();
}

void Controls::popStashedWork()
{
   QScopedPointer<GitStashes> git(new GitStashes(mGit));
   const auto ret = git->pop();

   if (ret.success)
      emit signalRepositoryUpdated();
   else
   {
      QMessageBox msgBox(QMessageBox::Critical, tr("Error while poping a stash"),
                         QString("There were problems during the stash pop operation. Please, see the detailed "
                                 "description for more information."),
                         QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output.toString());
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}

void Controls::pruneBranches()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->prune();
   QApplication::restoreOverrideCursor();

   if (ret.success)
      emit signalRepositoryUpdated();
}

void Controls::showConfigDlg()
{
   const auto configDlg = new RepoConfigDlg(mGit, this);
   configDlg->exec();
}

void Controls::createNewIssue()
{
   const auto createIssue = new CreateIssueDlg(mGit, this);
   createIssue->exec();
}

void Controls::createNewPullRequest()
{
   const auto prDlg = new CreatePullRequestDlg(mCache, mGit, this);
   connect(prDlg, &CreatePullRequestDlg::signalRefreshPRsCache, this, &Controls::signalRefreshPRsCache);

   prDlg->exec();
}

void Controls::configServer()
{
   const auto configDlg = new ServerConfigDlg(mGit, this);
   configDlg->exec();
}

void Controls::checkNewGitQlientVersion()
{
   QNetworkRequest request;
   request.setRawHeader("User-Agent", "GitQlient");
   request.setRawHeader("X-Custom-User-Agent", "GitQlient");
   request.setRawHeader("Content-Type", "application/json");
   request.setUrl(QUrl("https://github.com/francescmm/ci-utils/releases/download/gq_update/updates.json"));
   request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

   const auto reply = mManager->get(request);
   connect(reply, &QNetworkReply::finished, this, &Controls::processUpdateFile);
}

void Controls::processUpdateFile()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   const auto data = reply->readAll();
   const auto jsonDoc = QJsonDocument::fromJson(data);
   const auto url = reply->url().path();

   if (jsonDoc.isNull())
   {
      QLog_Error("Ui", QString("Error when parsing Json. Current data:\n%1").arg(QString::fromUtf8(data)));
      return;
   }

   QVector<ServerLabel> labels;
   const auto json = jsonDoc.object();

   mLatestGitQlient = json["latest-version"].toString();
   const auto changeLogUrl = json["changelog"].toString();

#if defined(Q_OS_WIN)
   const auto os = json["windows"].toObject();
#elif defined(Q_OS_LINUX)
   const auto os = json["linux"].toObject();
#elif defined(Q_OS_OSX)
   const auto os = json["osx"].toObject();
#endif

   mGitQlientDownloadUrl = os["download-url"].toString();

   const auto newVersion = mLatestGitQlient.split(".");
   const auto nv = newVersion.at(0).toInt() * 10000 + newVersion.at(1).toInt() * 100 + newVersion.at(2).toInt();
   const auto curVersion = QString("%1").arg(VER).split(".");
   const auto cv = curVersion.at(0).toInt() * 10000 + curVersion.at(1).toInt() * 100 + curVersion.at(2).toInt();

   if (nv > cv)
   {
      mVersionCheck->setVisible(true);

      QTimer::singleShot(200, this, [this, changeLogUrl] {
         QNetworkRequest request;
         request.setRawHeader("User-Agent", "GitQlient");
         request.setRawHeader("X-Custom-User-Agent", "GitQlient");
         request.setRawHeader("Content-Type", "application/json");
         request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
         request.setUrl(QUrl(changeLogUrl));

         const auto reply = mManager->get(request);

         connect(reply, &QNetworkReply::finished, this, &Controls::processChangeLog);
      });
   }
}

void Controls::processChangeLog()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   mChangeLog = QString::fromUtf8(reply->readAll());
}

void Controls::showInfoMessage()
{
   QMessageBox msgBox(QMessageBox::Information, tr("New version of GitQlient!"),
                      QString("There is a new version of GitQlient available. Your current vrsion is {%1} and the new "
                              "one is {%2}. You can read more about the new changes in the detailed description.")
                          .arg(VER, mLatestGitQlient),
                      QMessageBox::Ok | QMessageBox::Close, this);
   msgBox.setButtonText(QMessageBox::Ok, tr("Download"));
   msgBox.setDetailedText(mChangeLog);
   msgBox.setStyleSheet(GitQlientStyles::getStyles());

   if (msgBox.exec() == QMessageBox::Ok)
      downloadFile();
}

void Controls::downloadFile()
{
   QNetworkRequest request;
   request.setRawHeader("User-Agent", "GitQlient");
   request.setRawHeader("X-Custom-User-Agent", "GitQlient");
   request.setRawHeader("Content-Type", "application/octet-stream");
   request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
   request.setUrl(QUrl(mGitQlientDownloadUrl));

   const auto fileName = mGitQlientDownloadUrl.split("/").last();

   const auto reply = mManager->get(request);

   connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 read, qint64 total) {
      mDownloadLog->setVisible(true);
      mDownloadLog->setMaximum(total);
      mDownloadLog->setValue(read);
   });

   connect(reply, &QNetworkReply::finished, this, [this, reply, fileName]() {
      mDownloadLog->setVisible(false);

      const auto b = reply->readAll();
      const auto destination
          = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first() + "/" + fileName;
      QFile file(destination);
      if (file.open(QIODevice::WriteOnly))
      {
         QDataStream out(&file);
         out << b;

         file.close();
      }

      reply->deleteLater();
   });
}
