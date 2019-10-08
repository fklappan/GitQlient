#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2019  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 3 of the License, or (at your option) any later version.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

#include <QStyledItemDelegate>

class RepositoryView;
class RepositoryModel;
class Rev;

const int ROW_HEIGHT = 25;
const int LANE_WIDTH = 3 * ROW_HEIGHT / 4;

class RepositoryViewDelegate : public QStyledItemDelegate
{
   friend class RepositoryView;
   Q_OBJECT
public:
   RepositoryViewDelegate(RepositoryView *view, QObject *parent);

   virtual void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const;
   virtual QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
   {
      return QSize(LANE_WIDTH, ROW_HEIGHT);
   }

signals:
   void updateView();

public slots:
   void diffTargetChanged(int);

private:
   const Rev *revLookup(int row, RepositoryModel **fhPtr = nullptr) const;
   void paintLog(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const;
   void paintGraph(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const;
   void paintGraphLane(QPainter *p, int type, int x1, int x2, const QColor &col, const QColor &activeCol,
                       const QBrush &back) const;
   void paintWip(QPainter *painter, QStyleOptionViewItem opt) const;
   void paintTagBranch(QPainter *painter, QStyleOptionViewItem opt, int &startPoint, const QString &sha) const;

   RepositoryView *mRepoView = nullptr;
   int diffTargetRow = -1;
};