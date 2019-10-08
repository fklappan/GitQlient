/*
        Author: Marco Costalba (C) 2005-2007

        Copyright: See COPYING file that comes with this distribution

*/
#ifndef DOMAIN_H
#define DOMAIN_H

#include <QObject>
#include <StateInfo.h>

class Domain;
class RepositoryModel;
class Git;
class MainWindow;

class Domain : public QObject
{
   Q_OBJECT
public:
   Domain(bool isMain);
   virtual ~Domain();
   void deleteWhenDone(); // will delete when no more run() are pending
   RepositoryModel *model() const { return fileHistory; }
   bool isLinked() const { return linked; }
   virtual bool isMatch(const QString &) { return false; }
   void update(bool fromMaster, bool force);

   StateInfo st;

signals:
   void updateRequested(StateInfo newSt);
   void cancelDomainProcesses();
   void setTabText(QWidget *w, const QString &label);

public slots:
   void on_closeAllTabs();
   virtual void on_contextMenu(const QString &, int);

protected slots:
   void on_updateRequested(StateInfo newSt);
   void on_deleteWhenDone();

protected:
   virtual void clear(bool complete = true);
   virtual bool doUpdate(bool force) = 0;
   void linkDomain(Domain *d);
   void unlinkDomain(Domain *d);

   bool busy;

private:
   void populateState();
   bool flushQueue();
   void sendPopupEvent();

   int exDeleteRequest;
   int exCancelRequest;

   RepositoryModel *fileHistory;
   bool linked;
   int popupType;
   QString popupData;
   QString statusBarRequest;
};

#endif