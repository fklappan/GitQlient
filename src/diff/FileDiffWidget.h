#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2020  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This program is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2 of the License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

#include <IDiffWidget.h>

#include <QFrame>
#include <DiffInfo.h>

class FileDiffView;
class QPushButton;
class CheckBox;
class FileEditor;
class QStackedWidget;

/*!
 \brief The FileDiffWidget creates the layout that contains all the widgets related with the creation of the diff of a
 specific file.

 \class FileDiffWidget FileDiffWidget.h "FileDiffWidget.h"
*/
class FileDiffWidget : public IDiffWidget
{
   Q_OBJECT

signals:
   /**
    * @brief exitRequested Signal triggered when the user clicks the back button to return to the graph view.
    */
   void exitRequested();

   /**
    * @brief fileStaged
    * @param fileName
    */
   void fileStaged(const QString &fileName);

   /**
    * @brief fileReverted Signal triggered when the user reverts all the changes of the current file.
    * @param fileName The filename
    */
   void fileReverted(const QString &fileName);

public:
   /*!
    \brief Default constructor.

    \param git The git object to perform Git operations.
    \param parent The parent widget if needed.
   */
   explicit FileDiffWidget(const QSharedPointer<GitBase> &git, QSharedPointer<RevisionsCache> cache,
                           QWidget *parent = nullptr);

   /*!
    \brief Clears the current information on the diff view.
   */
   void clear();
   /*!
    \brief Reloads the information currently displayed in the diff view. The relaod only is applied if the current file
    could change, that is if the user is watching the work in progress state. \return bool Returns true if the reload
    was done, otherwise false.
   */
   bool reload() override;
   /*!
    \brief Configures the diff view with the two commits that will be compared and the file that will be applied.

    \param currentSha The base SHA.
    \param previousSha The SHA to compare to.
    \param file The file that will show the diff.
    \return bool Returns true if the configuration was applied, otherwise false.
   */
   bool configure(const QString &currentSha, const QString &previousSha, const QString &file, bool editMode = false);

   /**
    * @brief setFileVsFileEnable Enables the widget to show file vs file view.
    * @param enable If true, enables the file vs file view.
    */
   void setSplitViewEnabled(bool enable);

   /**
    * @brief setFullViewEnabled Sets the full file view enabled.
    * @param enable True to enable, otherwise false.
    */
   void setFullViewEnabled(bool enable);

   /**
    * @brief editMode Enters in edit mode for the give file.
    * @param fileName The full path of the file.
    */
   void editMode(const QString &fileName);

private:
   QString mCurrentFile;
   QPushButton *mBack = nullptr;
   QPushButton *mGoPrevious = nullptr;
   QPushButton *mGoNext = nullptr;
   QPushButton *mEdition = nullptr;
   QPushButton *mFullView = nullptr;
   QPushButton *mSplitView = nullptr;
   QPushButton *mSave = nullptr;
   QPushButton *mStage = nullptr;
   QPushButton *mRevert = nullptr;
   FileDiffView *mNewFile = nullptr;
   FileDiffView *mOldFile = nullptr;
   QVector<int> mModifications;
   bool mFileVsFile = false;
   QVector<DiffInfo::ChunkInfo> mChunks;
   int mCurrentChunkLine = 0;
   FileEditor *mFileEditor = nullptr;
   QStackedWidget *mViewStackedWidget = nullptr;

   /**
    * @brief processDiff Process the diff to configure the navigation parameters.
    * @param text
    * @param newFileData
    * @param oldFileData
    */
   void processDiff(const QString &text, QPair<QStringList, QVector<DiffInfo::ChunkInfo>> &newFileData,
                    QPair<QStringList, QVector<DiffInfo::ChunkInfo>> &oldFileData);
   /**
    * @brief moveTop Moves to the first diff chunk.
    */
   void moveTop();
   /**
    * @brief moveChunkUp Moves to the previous diff chunk.
    */
   void moveChunkUp();
   /**
    * @brief moveChunkDown Moves to the following diff chunk.
    */
   void moveChunkDown();
   /**
    * @brief moveBottomChunk Moves to the last diff chunk.
    */
   void moveBottomChunk();

   /**
    * @brief enterEditionMode Enters edition mode
    * @param enter
    */
   void enterEditionMode(bool enter);

   /**
    * @brief endEditFile Closes the file editor.
    */
   void endEditFile();
   /**
    * @brief stageFile Stages the file.
    */
   void stageFile();
   /**
    * @brief revertFile Revert all the changes to the file.
    */
   void revertFile();
};
