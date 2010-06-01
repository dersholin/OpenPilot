/**
 ******************************************************************************
 *
 * @file       workspacesettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
 * @{
 * 
 *****************************************************************************/
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "workspacesettings.h"
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uavgadgetmode.h>
#include <QtCore/QSettings>

#include "ui_workspacesettings.h"


using namespace Core;
using namespace Core::Internal;

const int WorkspaceSettings::MAX_WORKSPACES = 10;

WorkspaceSettings::WorkspaceSettings(QObject *parent) :
    IOptionsPage(parent)
{
}

WorkspaceSettings::~WorkspaceSettings()
{
}

// IOptionsPage

QString WorkspaceSettings::id() const
{
    return QLatin1String("Workspaces");
}

QString WorkspaceSettings::trName() const
{
    return tr("Workspaces");
}

QString WorkspaceSettings::category() const
{
    return QLatin1String("GCS");
}

QString WorkspaceSettings::trCategory() const
{
    return tr("GCS");
}

QWidget *WorkspaceSettings::createPage(QWidget *parent)
{
    m_page = new Ui::WorkspaceSettings();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);

    // Read workspaces from settings
    m_page->numberOfWorkspacesSpinBox->setMaximum(MAX_WORKSPACES);
    m_modeManager = Core::ICore::instance()->modeManager();
    m_settings = Core::ICore::instance()->settings();
    m_settings->beginGroup(QLatin1String("Workspace"));
    const int numberOfWorkspaces = m_settings->value(QLatin1String("NumberOfWorkspaces"), 2).toInt();
    m_page->numberOfWorkspacesSpinBox->setValue(numberOfWorkspaces);
    for (int i = 1; i <= MAX_WORKSPACES; ++i) {
        QString numberString = QString::number(i);
        QString defaultName = "Workspace" + numberString;
        QString defaultIconName = "Icon" + numberString;
        const QString name = m_settings->value(defaultName, defaultName).toString();
        const QString iconName = m_settings->value(defaultIconName, ":/core/images/openpilot_logo_64.png").toString();
        m_iconNames.append(iconName);
        m_names.append(name);
        if (i <= numberOfWorkspaces)
            m_page->workspaceComboBox->addItem(QIcon(iconName), name);
    }
    m_page->iconPathChooser->setExpectedKind(Utils::PathChooser::File);
    m_page->iconPathChooser->setPromptDialogFilter(tr("Images (*.png *.jpg *.bmp *.xpm)"));
    m_page->iconPathChooser->setPromptDialogTitle(tr("Choose icon"));

    m_settings->endGroup();

    connect(m_page->workspaceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectWorkspace(int)));
    connect(m_page->numberOfWorkspacesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(numberOfWorkspacesChanged(int)));
    connect(m_page->nameEdit, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));
    connect(m_page->iconPathChooser, SIGNAL(browsingFinished()), this, SLOT(iconChanged()));

    m_currentIndex = 0;
    selectWorkspace(m_currentIndex);

    return w;
}

void WorkspaceSettings::apply()
{
    selectWorkspace(m_currentIndex, true);

    m_settings->beginGroup(QLatin1String("Workspace"));
    m_settings->setValue(QLatin1String("NumberOfWorkspaces"), m_page->numberOfWorkspacesSpinBox->value());
    for (int i = 1; i <= MAX_WORKSPACES; ++i) {
        QString numberString = QString::number(i);
        QString defaultName = "Workspace" + numberString;
        QString defaultIconName = "Icon" + numberString;
        m_settings->setValue(defaultName, m_names.at(i-1));
        m_settings->setValue(defaultIconName, m_iconNames.at(i-1));
        QString modeName = "Mode" + numberString;
        Core::Internal::UAVGadgetMode *mode = qobject_cast<Core::Internal::UAVGadgetMode*>(m_modeManager->mode(modeName));
        if (mode) {
            m_modeManager->updateModeNameIcon(mode, QIcon(m_iconNames.at(i-1)), m_names.at(i-1));
        }
    }
    m_settings->endGroup();

}

void WorkspaceSettings::finish()
{
    delete m_page;
}

void WorkspaceSettings::textEdited(QString name)
{
    m_page->workspaceComboBox->setItemText(m_currentIndex, m_page->nameEdit->text());
}

void WorkspaceSettings::iconChanged()
{
    QString iconName = m_page->iconPathChooser->path();
    m_page->workspaceComboBox->setItemIcon(m_currentIndex, QIcon(iconName));
}

void WorkspaceSettings::numberOfWorkspacesChanged(int value)
{
    int count = m_page->workspaceComboBox->count();
    if (value > count) {
        for (int i = count; i < value; ++i) {
            m_page->workspaceComboBox->addItem(QIcon(m_iconNames.at(i)), m_names.at(i));
        }
    } else if (value < count){
        for (int i = count-1; i >= value; --i) {
            m_page->workspaceComboBox->removeItem(i);
        }
    }
}

void WorkspaceSettings::selectWorkspace(int index, bool store)
{
    if (store || (index != m_currentIndex)) {
        // write old values of workspace not shown anymore
        m_iconNames.replace(m_currentIndex, m_page->iconPathChooser->path());
        m_names.replace(m_currentIndex, m_page->nameEdit->text());
        m_page->workspaceComboBox->setItemIcon(m_currentIndex, QIcon(m_page->iconPathChooser->path()));
        m_page->workspaceComboBox->setItemText(m_currentIndex, m_page->nameEdit->text());
    }

    // display current workspace   
    QString iconName = m_iconNames.at(index);
    m_page->iconPathChooser->setPath(iconName);
    m_page->nameEdit->setText(m_names.at(index));
    m_currentIndex = index;
}
