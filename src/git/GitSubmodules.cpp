#include "GitSubmodules.h"

#include <GitBase.h>

#include <QLogger.h>
#include <BenchmarkTool.h>

using namespace QLogger;
using namespace Benchmarker;

GitSubmodules::GitSubmodules(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

QVector<QString> GitSubmodules::getSubmodules()
{
   BenchmarkStart();

   QLog_Debug("Git", QString("Executing getSubmodules"));

   QVector<QString> submodulesList;
   const auto ret = mGitBase->run("git config --file .gitmodules --name-only --get-regexp path");
   if (ret.success)
   {
      const auto submodules = ret.output.toString().split('\n');
      for (const auto &submodule : submodules)
         if (!submodule.isEmpty() && submodule != "\n")
            submodulesList.append(submodule.split('.').at(1));
   }

   BenchmarkEnd();

   return submodulesList;
}

bool GitSubmodules::submoduleAdd(const QString &url, const QString &name)
{
   BenchmarkStart();

   QLog_Debug("Git", QString("Executing submoduleAdd: {%1} {%2}").arg(url, name));

   const auto ret = mGitBase->run(QString("git submodule add %1 %2").arg(url, name)).success;

   BenchmarkStart();

   return ret;
}

bool GitSubmodules::submoduleUpdate(const QString &)
{
   BenchmarkStart();

   QLog_Debug("Git", QString("Executing submoduleUpdate"));

   const auto ret = mGitBase->run("git submodule update --init --recursive").success;

   BenchmarkEnd();

   return ret;
}

bool GitSubmodules::submoduleRemove(const QString &)
{
   return false;
}
