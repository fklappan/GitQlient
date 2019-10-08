/*
        Description: patch viewer window

        Author: Marco Costalba (C) 2005-2007

        Copyright: See COPYING file that comes with this distribution

*/
#include "DiffWidget.h"
#include "ui_DiffWidget.h"

#include "common.h"
#include "git.h"
#include "MainWindow.h"
#include "FullDiffWidget.h"
#include <QButtonGroup>
#include <QScrollBar>

DiffWidget::DiffWidget(QWidget *parent)
   : QWidget(parent)
   , ui(new Ui::DiffWidget)
   , mDomain(new PatchViewDomain())
{
   ui->setupUi(this);

   mDomain->setOwner(this);
}

void DiffWidget::clear(bool complete)
{
   if (complete)
      mSt.clear();

   ui->textEditDiff->clear();
}

void DiffWidget::setStateInfo(const StateInfo &st)
{
   mSt = st;
   mDomain->update(true, false);
}

bool DiffWidget::doUpdate(bool force)
{
   if ((mSt.isChanged(StateInfo::SHA) || force) && !mDomain->isLinked())
   {
      QString caption(Git::getInstance()->getShortLog(mSt.sha()));

      if (caption.length() > 30)
         caption = caption.left(30 - 3).trimmed().append("...");
   }

   if (mSt.isChanged(StateInfo::ANY & ~StateInfo::FILE_NAME) || force)
   {
      ui->textEditDiff->clear();

      if (normalizedSha != mSt.diffToSha() && !normalizedSha.isEmpty())
         normalizedSha = "";

      ui->textEditDiff->update(mSt); // non blocking
   }

   if (mSt.isChanged() || force)
      ui->textEditDiff->centerOnFileHeader(mSt);

   return true;
}

PatchViewDomain::PatchViewDomain()
   : Domain(false)
{
}

void PatchViewDomain::setOwner(DiffWidget *owner)
{
   mOwner = owner;
}

bool PatchViewDomain::doUpdate(bool force)
{
   return mOwner->doUpdate(force);
}