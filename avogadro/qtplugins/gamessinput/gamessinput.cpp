/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "gamessinput.h"

#include "gamessinputdialog.h"

#include <avogadro/io/fileformat.h>
#include <avogadro/qtgui/fileformatdialog.h>
#include <avogadro/qtgui/molecule.h>

#include <avogadro/molequeue/client/jobobject.h>

#include <QtCore/QDebug>

#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>

namespace Avogadro {
namespace Core {
class Molecule;
}
namespace QtPlugins {

using MoleQueue::JobObject;

GamessInput::GamessInput(QObject* parent_)
  : ExtensionPlugin(parent_), m_action(new QAction(this)), m_molecule(nullptr),
    m_dialog(nullptr), m_outputFormat(nullptr)
{
  m_action->setEnabled(true);
  m_action->setText(tr("&GAMESS…"));
  connect(m_action, SIGNAL(triggered()), SLOT(menuActivated()));
}

GamessInput::~GamessInput()
{
}

QList<QAction*> GamessInput::actions() const
{
  QList<QAction*> actions_;
  actions_.append(m_action);
  return actions_;
}

QStringList GamessInput::menuPath(QAction*) const
{
  QStringList path;
  path << tr("&Input");
  return path;
}

void GamessInput::setMolecule(QtGui::Molecule* mol)
{
  if (m_dialog)
    m_dialog->setMolecule(mol);
  m_molecule = mol;
}

void GamessInput::openJobOutput(const JobObject& job)
{
  m_outputFormat = nullptr;
  m_outputFileName.clear();

  QString outputPath(job.value("outputDirectory").toString());

  using QtGui::FileFormatDialog;
  FileFormatDialog::FormatFilePair result = FileFormatDialog::fileToRead(
    qobject_cast<QWidget*>(parent()), tr("Open Output File"), outputPath);

  if (result.first == nullptr) // User canceled
    return;

  m_outputFormat = result.first;
  m_outputFileName = result.second;

  emit moleculeReady(1);
}

bool GamessInput::readMolecule(QtGui::Molecule& mol)
{
  Io::FileFormat* reader = m_outputFormat->newInstance();
  bool success = reader->readFile(m_outputFileName.toStdString(), mol);
  if (!success) {
    QMessageBox::information(qobject_cast<QWidget*>(parent()), tr("Error"),
                             tr("Error reading output file '%1':\n%2")
                               .arg(m_outputFileName)
                               .arg(QString::fromStdString(reader->error())));
  }

  m_outputFormat = nullptr;
  m_outputFileName.clear();

  return success;
}

void GamessInput::menuActivated()
{
  if (!m_dialog) {
    m_dialog = new GamessInputDialog(qobject_cast<QWidget*>(parent()));
    connect(m_dialog, SIGNAL(openJobOutput(Avogadro::MoleQueue::JobObject)), this,
            SLOT(openJobOutput(Avogadro::MoleQueue::JobObject)));
  }
  m_dialog->setMolecule(m_molecule);
  m_dialog->show();
}
}
}
