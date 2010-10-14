/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  The Aethyra Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iostream>
#include <SDL.h>
#include <SDL_thread.h>

#include <guichan/font.hpp>

#include "updatewindow.h"

#include "../statemanager.h"

#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/widgets/button.h"
#include "../../bindings/guichan/widgets/label.h"
#include "../../bindings/guichan/widgets/progressbar.h"
#include "../../bindings/guichan/widgets/richtextbox.h"
#include "../../bindings/guichan/widgets/scrollarea.h"

#include "../../core/configuration.h"
#include "../../core/log.h"
#include "../../core/resourcemanager.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

UpdaterWindow::UpdaterWindow(const std::string &updateHost) :
    Window(_("Updating...")),
    labelState(IDLE_LABEL)
{
    mRichTextBox = new RichTextBox();
    mRichTextBox->setOpaque(false);
    mScrollArea = new ScrollArea(mRichTextBox);

    mLabel = new Label(_("Connecting..."));
    mNewLabelCaption = mLabel->getCaption();

    mProgressBar = new ProgressBar(0.0, 310, 20, gcn::Color(168, 116, 31));
    mProgressBar->setSmoothProgress(false);
    mProgressBar->toggleThrobbing(true);

    mStateButton = new Button(_("Cancel"), "cancel", this);

    fontChanged();

    mLogic = new DownloadUpdates(updateHost, this);
    mLogic->download();
}

UpdaterWindow::~UpdaterWindow()
{
    destroy(mLogic);
}

void UpdaterWindow::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mScrollArea, 5, 3).setPadding(3);
    place(0, 3, mLabel, 5);
    place(0, 4, mProgressBar, 5);
    place(4, 5, mStateButton);

    reflowLayout(320, 240);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    restoreFocus();
}

void UpdaterWindow::downloadComplete()
{
    MutexLocker lock(&mLabelMutex);
    labelState = PLAY_LABEL;

    mNewLabelCaption = _("Update Complete");
    // leave mNewProgress as it is, in case the user cancelled
    // (this will leave the download bar reflecting the situation)
}

void UpdaterWindow::downloadFailed()
{
    MutexLocker lock(&mLabelMutex);
    labelState = FAIL_LABEL;

    mNewLabelCaption = _("Update Failed");
    // leave mNewProgress as it is, in case the user cancelled
    // (this will leave the download bar reflecting the situation)
}

void UpdaterWindow::verifyingFiles()
{
    MutexLocker lock(&mLabelMutex);
    labelState = PLAY_LABEL;

    mNewLabelCaption = _("Verifying Files");
    // leave mNewProgress as it is, in case the user cancelled
    // (this will leave the download bar reflecting the situation)
}

void UpdaterWindow::requestFocus()
{
    Window::requestFocus();

    if (mStateButton->isEnabled())
        mStateButton->requestFocus();
}

void UpdaterWindow::enable()
{
    mStateButton->setCaption(_("Play"));
    mStateButton->setActionEventId("play");
    fontChanged();
}

void UpdaterWindow::downloadTextUpdate(const std::vector<std::string>& news)
{
    {
        MutexLocker lock(&mLabelMutex);
        mNewNews = news;
    }
}

void UpdaterWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel" || event.getId() == "close")
    {
        mLogic->userCancelled();
    }
    else if (event.getId() == "play")
    {
        mStateButton->setEnabled(false);
        stateManager->setState(LOADDATA_STATE);
    }
}

void UpdaterWindow::downloadProgress(float totalProgress,
        const std::string& currentFile, float fileProgress)
{
    // Do delayed label text update, since Guichan isn't thread-safe
    {
        MutexLocker lock(&mLabelMutex);

        mNewLabelCaption = currentFile +" (" + toString((int) (fileProgress *
                           100)) + "%) " + _("Total Progress") + " (" +
                           toString((int) (totalProgress * 100)) + "%)";

        mNewProgress = totalProgress;
        labelState = DOWNLOAD_LABEL;
    }
}

void UpdaterWindow::logic()
{
    Window::logic();

    // Synchronize label caption when necessary
    {
        MutexLocker lock(&mLabelMutex);

        if (mLabel->getCaption() != mNewLabelCaption)
        {
            mLabel->setCaption(mNewLabelCaption);
            mLabel->adjustSize();

            mProgressBar->setProgress(mNewProgress);

            switch (labelState)
            {
                case PLAY_LABEL:
                    mProgressBar->setProgress(1.0f);
                    enable();
                    break;
                default:
                    break;
            }

            labelState = IDLE_LABEL;
        }

        if (! mNewNews.empty())
        {
            mRichTextBox->clearRows();

            for (std::vector<std::string>::const_iterator itr = mNewNews.begin();
                 itr != mNewNews.end(); itr++)
            {
                mRichTextBox->addRow(*itr);
            }

            mScrollArea->setVerticalScrollAmount(0);
            mNewNews.clear();
        }
    }
}
