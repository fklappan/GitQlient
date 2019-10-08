/*
        Author: Marco Costalba (C) 2005-2007

Copyright: See COPYING file that comes with this distribution

                */
#include "domain.h"
#include "RepositoryModel.h"
#include "git.h"
#include "MainWindow.h"
#include <QApplication>
#include <QStatusBar>
#include <QTimer>

using namespace QGit;

// ************************* Domain ****************************

Domain::Domain(bool isMain)
   : QObject()
{
   fileHistory = new RepositoryModel(this);

   if (isMain)
      Git::getInstance()->setDefaultModel(fileHistory);

   st.clear();
   busy = linked = false;
   popupType = 0;
}

Domain::~Domain()
{
   if (!parent())
      return;
}

void Domain::clear(bool complete)
{
   if (complete)
      st.clear();

   fileHistory->clear();
}

void Domain::on_closeAllTabs()
{

   delete this; // must be sync, deleteLater() does not work
}

void Domain::deleteWhenDone()
{
   emit cancelDomainProcesses();

   on_deleteWhenDone();
}

void Domain::on_deleteWhenDone()
{
   deleteLater();
}

void Domain::unlinkDomain(Domain *d)
{

   d->linked = false;
   while (d->disconnect(SIGNAL(updateRequested(StateInfo)), this))
      ; // a signal is emitted for every connection you make,
        // so if you duplicate a connection, two signals will be emitted.
}

void Domain::linkDomain(Domain *d)
{

   unlinkDomain(d); // be sure only one connection is active
   connect(d, &Domain::updateRequested, this, &Domain::on_updateRequested);
   d->linked = true;
}

void Domain::on_updateRequested(StateInfo newSt)
{

   st = newSt;
   update(false, false);
}

bool Domain::flushQueue()
{
   // during dragging any state update is queued, so try to flush pending now

   if (!busy && st.flushQueue())
   {
      update(false, false);
      return true;
   }
   return false;
}

void Domain::populateState()
{
   const auto r = Git::getInstance()->revLookup(st.sha());

   if (r)
      st.setIsMerge(r->parentsCount() > 1);
}

void Domain::update(bool fromMaster, bool force)
{

   if (busy && st.requestPending())
   {
      emit cancelDomainProcesses();
   }
   if (busy)
      return;

   if (linked && !fromMaster)
   {
      // in this case let the update to fall down from master domain
      StateInfo tmp(st);
      st.rollBack(); // we don't want to filter out next update sent from master
      emit updateRequested(tmp);
      return;
   }

   const auto git = Git::getInstance();
   git->setCurContext(this);
   busy = true;
   populateState(); // complete any missing state information
   st.setLock(true); // any state change will be queued now

   if (doUpdate(force))
      st.commit();
   else
      st.rollBack();

   st.setLock(false);
   busy = false;
   if (git->curContext() != this)
      qDebug("ASSERT in Domain::update, context is %p "
             "instead of %p",
             (void *)git->curContext(), (void *)this);

   git->setCurContext(nullptr);

   bool nextRequestPending = flushQueue();

   if (!nextRequestPending && !statusBarRequest.isEmpty())
   {
      // update status bar when we are sure no more work is pending
      // TODO: Update the status bar through the Singleton (Future)
      // QApplication::postEvent(m(), new MessageEvent(statusBarRequest));
      statusBarRequest = "";
   }
   if (!nextRequestPending && popupType)
      sendPopupEvent();
}

void Domain::sendPopupEvent()
{
   // call an async context popup, must be executed
   // after returning to event loop
   // TODO: Popup the error in ApplicationModality context
   /*
   DeferredPopupEvent* e = new DeferredPopupEvent(popupData, popupType);
   QApplication::postEvent(m(), e);
   popupType = 0;
*/
}

void Domain::on_contextMenu(const QString &data, int type)
{

   popupType = type;
   popupData = data;

   if (busy)
      return; // we are in the middle of an update

   sendPopupEvent();
}