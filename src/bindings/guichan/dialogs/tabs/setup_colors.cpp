/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *
 *  This file is part of Aethyra.
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

#include <string>
#include <cmath>

#include "setup_colors.h"

#include "../../gui.h"
#include "../../layouthelper.h"

#include "../../widgets/label.h"
#include "../../widgets/listbox.h"
#include "../../widgets/richtextbox.h"
#include "../../widgets/scrollarea.h"
#include "../../widgets/slider.h"
#include "../../widgets/textfield.h"
#include "../../widgets/textpreview.h"

#include "../../../../core/configuration.h"

#include "../../../../core/utils/dtor.h"
#include "../../../../core/utils/gettext.h"
#include "../../../../core/utils/stringutils.h"

const std::string Setup_Colors::rawmsg = _("This is what the color looks like");

Setup_Colors::Setup_Colors() :
    mSelected(-1)
{
    setName(_("Colors"));

    mColorBox = new ListBox(guiPalette);
    mColorBox->addSelectionListener(this);

    mScroll = new ScrollArea(mColorBox);
    mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTextPreview = new TextPreview(&rawmsg);

    mPreview = new RichTextBox(RichTextBox::AUTO_WRAP);
    mPreview->setOpaque(false);

    // don't do anything with links
    mPreview->setLinkHandler(NULL);

    mPreviewBox = new ScrollArea(mPreview);
    mPreviewBox->setHeight(20);
    mPreviewBox->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER,
                                 gcn::ScrollArea::SHOW_NEVER);

    mGradTypeLabel = new Label(_("Type: "));

    mGradTypeSlider = new Slider(0, 3);
    mGradTypeSlider->setWidth(180);
    mGradTypeSlider->setStepLength(1.0);
    mGradTypeSlider->setActionEventId("slider_grad");
    mGradTypeSlider->setValue(0);
    mGradTypeSlider->addActionListener(this);
    mGradTypeSlider->setEnabled(false);

    mGradTypeText = new Label();

    std::string longText = _("Static");

    if (getFont()->getWidth(_("Pulse")) > getFont()->getWidth(longText))
        longText = _("Pulse");
    if (getFont()->getWidth(_("Rainbow")) > getFont()->getWidth(longText))
        longText = _("Rainbow");
    if (getFont()->getWidth(_("Spectrum")) > getFont()->getWidth(longText))
        longText = _("Spectrum");

    mGradTypeText->setCaption(longText);

    mGradDelayLabel = new Label(_("Delay: "));

    mGradDelayText = new TextField();
    mGradDelayText->setWidth(40);
    mGradDelayText->setRange(20, 100);
    mGradDelayText->setNumeric(true);
    mGradDelayText->setEnabled(false);
    mGradDelayText->setFocusable(false);

    mGradDelaySlider = new Slider(20, 100);
    mGradDelaySlider->setWidth(180);
    mGradDelaySlider->setStepLength(1.0);
    mGradDelaySlider->setValue(mGradDelayText->getValue());
    mGradDelaySlider->setActionEventId("slider_graddelay");
    mGradDelaySlider->addActionListener(this);
    mGradDelaySlider->setEnabled(false);

    mRedLabel = new Label(_("Red: "));

    mRedText = new TextField();
    mRedText->setWidth(40);
    mRedText->setRange(0, 255);
    mRedText->setNumeric(true);
    mRedText->setEnabled(false);
    mRedText->setFocusable(false);

    mRedSlider = new Slider(0, 255);
    mRedSlider->setWidth(180);
    mRedSlider->setStepLength(8.0);
    mRedSlider->setValue(mRedText->getValue());
    mRedSlider->setActionEventId("slider_red");
    mRedSlider->addActionListener(this);
    mRedSlider->setEnabled(false);

    mGreenLabel = new Label(_("Green: "));

    mGreenText = new TextField();
    mGreenText->setWidth(40);
    mGreenText->setRange(0, 255);
    mGreenText->setNumeric(true);
    mGreenText->setEnabled(false);
    mGreenText->setFocusable(false);

    mGreenSlider = new Slider(0, 255);
    mGreenSlider->setWidth(180);
    mGreenSlider->setStepLength(8.0);
    mGreenSlider->setValue(mGreenText->getValue());
    mGreenSlider->setActionEventId("slider_green");
    mGreenSlider->addActionListener(this);
    mGreenSlider->setEnabled(false);

    mBlueLabel = new Label(_("Blue: "));

    mBlueText = new TextField();
    mBlueText->setWidth(40);
    mBlueText->setRange(0, 255);
    mBlueText->setNumeric(true);
    mBlueText->setEnabled(false);
    mBlueText->setFocusable(false);

    mBlueSlider = new Slider(0, 255);
    mBlueSlider->setWidth(180);
    mBlueSlider->setStepLength(8.0);
    mBlueSlider->setValue(mBlueText->getValue());
    mBlueSlider->setActionEventId("slider_blue");
    mBlueSlider->addActionListener(this);
    mBlueSlider->setEnabled(false);

    setOpaque(false);

    fontChanged();

    setDimension(gcn::Rectangle(0, 0, 325, 280));

    mGradTypeText->setCaption("");
}

Setup_Colors::~Setup_Colors()
{
    if (mPreviewBox->getContent() == mPreview)
        destroy(mTextPreview);
    else
        destroy(mPreview);
}

void Setup_Colors::fontChanged()
{
    SetupTabContainer::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mScroll, 6, 6).setPadding(2);
    place(0, 6, mPreviewBox, 6).setPadding(2);
    place(0, 7, mGradTypeLabel, 3);
    place(3, 7, mGradTypeSlider);
    place(4, 7, mGradTypeText, 2).setPadding(1);
    place(0, 8, mRedLabel, 3);
    place(3, 8, mRedSlider);
    place(5, 8, mRedText).setPadding(1);
    place(0, 9, mGreenLabel, 3);
    place(3, 9, mGreenSlider);
    place(5, 9, mGreenText).setPadding(1);
    place(0, 10, mBlueLabel, 3);
    place(3, 10, mBlueSlider);
    place(5, 10, mBlueText).setPadding(1);
    place(0, 11, mGradDelayLabel, 3);
    place(3, 11, mGradDelaySlider);
    place(5, 11, mGradDelayText).setPadding(1);

    h.reflowLayout(325, 280);

    restoreFocus();
}

void Setup_Colors::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "slider_grad")
        updateGradType();
    else if (event.getId() == "slider_graddelay")
        mGradDelayText->setText(toString(std::floor(mGradDelaySlider->getValue())));
    else if (event.getId() == "slider_red")
        mRedText->setText(toString(std::floor(mRedSlider->getValue())));
    else if (event.getId() == "slider_green")
        mGreenText->setText(toString(std::floor(mGreenSlider->getValue())));
    else if (event.getId() == "slider_blue")
        mBlueText->setText(toString(std::floor(mBlueSlider->getValue())));
    else
        return;

    updateColor();
}

void Setup_Colors::valueChanged(const gcn::SelectionEvent &event)
{
    mSelected = mColorBox->getSelected();
    Palette::ColorType type = guiPalette->getColorTypeAt(mSelected);
    const gcn::Color *col = &guiPalette->getColor(type);
    Palette::GradientType grad = guiPalette->getGradientType(type);
    const int delay = guiPalette->getGradientDelay(type);

    std::string msg;
    const std::string markup = guiPalette->getColorMarkup(type);

    mPreview->clearRows();
    mPreviewBox->setContent(mTextPreview);
    mTextPreview->setFont(gui->getFont());
    mTextPreview->setTextColor(&guiPalette->getColor(Palette::TEXT));
    mTextPreview->setTextBGColor(NULL);
    mTextPreview->setOpaque(false);
    mTextPreview->setShadow(true);
    mTextPreview->setOutline(true);
    mTextPreview->useTextAlpha(false);

    switch (type)
    {
        case Palette::TEXT:
        case Palette::SHADOW:
        case Palette::OUTLINE:
            mTextPreview->setFont(gui->getFont());
            mTextPreview->setShadow(type == Palette::SHADOW);
            mTextPreview->setOutline(type == Palette::OUTLINE);
            break;
        case Palette::PROGRESS_BAR:
            mTextPreview->useTextAlpha(true);
            mTextPreview->setFont(gui->getBoldFont());
            mTextPreview->setTextColor(col);
            mTextPreview->setOutline(true);
            mTextPreview->setShadow(false);
            break;
        case Palette::TAB_HIGHLIGHT:
        case Palette::ITEM_EQUIPPED:
            mTextPreview->setFont(gui->getFont());
            mTextPreview->setTextColor(col);
            mTextPreview->setOutline(false);
            mTextPreview->setShadow(false);
            break;
        case Palette::BACKGROUND:
        case Palette::SHOP_WARNING:
            mTextPreview->setBGColor(col);
            mTextPreview->setOpaque(true);
            mTextPreview->setOutline(false);
            mTextPreview->setShadow(false);
            break;
        case Palette::HIGHLIGHT:
            mTextPreview->setTextBGColor(col);
            mTextPreview->setOutline(false);
            mTextPreview->setShadow(false);
            mPreview->addRow(rawmsg);
            break;
        case Palette::CHAT:
        case Palette::GM:
        case Palette::PLAYER:
        case Palette::WHISPER:
        case Palette::IS:
        case Palette::PARTY:
        case Palette::SERVER:
        case Palette::LOGGER:
        case Palette::HYPERLINK:
            mPreviewBox->setContent(mPreview);
            mPreview->clearRows();

            if (markup.compare("##<") == 0)
                msg = toString("@@|") + rawmsg + "@@";
            else
                msg = markup + rawmsg;

            mPreview->addRow(msg);
            break;
        case Palette::UNKNOWN_ITEM:
        case Palette::GENERIC:
        case Palette::HEAD:
        case Palette::USABLE:
        case Palette::TORSO:
        case Palette::ONEHAND:
        case Palette::LEGS:
        case Palette::FEET:
        case Palette::TWOHAND:
        case Palette::SHIELD:
        case Palette::RING:
        case Palette::ARMS:
        case Palette::AMMO:
            mTextPreview->setFont(gui->getBoldFont());
            mTextPreview->setTextColor(col);
            mTextPreview->setOutline(false);
            mTextPreview->setShadow(false);
            break;
        case Palette::PARTICLE:
        case Palette::EXP_INFO:
        case Palette::PICKUP_INFO:
        case Palette::HIT_PLAYER_MONSTER:
        case Palette::HIT_MONSTER_PLAYER:
        case Palette::HIT_CRITICAL:
        case Palette::MISS:
            mTextPreview->setShadow(false);
        case Palette::BEING:
        case Palette::PC:
        case Palette::SELF:
        case Palette::GM_NAME:
        case Palette::NPC:
        case Palette::MONSTER:
            mTextPreview->setFont(gui->getBoldFont());
            mTextPreview->setTextColor(col);
        case Palette::TYPE_COUNT:
        default:
            break;
    }

    // If a gradient color, don't display the current, but the committed
    // color at the sliders
    if (grad != Palette::STATIC && grad != Palette::PULSE)
        col = &guiPalette->getCommittedColor(type);
    else if (grad == Palette::PULSE)
        col = &guiPalette->getTestColor(type);

    setEntry(mGradDelaySlider, mGradDelayText, delay);
    setEntry(mRedSlider, mRedText, col->r);
    setEntry(mGreenSlider, mGreenText, col->g);
    setEntry(mBlueSlider, mBlueText, col->b);

    mGradTypeSlider->setValue(grad);
    updateGradType();
    mGradTypeSlider->setEnabled(true);
}

void Setup_Colors::setEntry(gcn::Slider *s, TextField *t, int value)
{
    s->setValue(value);
    char buffer[100];
    sprintf(buffer, "%d", value);
    t->setText(buffer);
}

void Setup_Colors::apply()
{
    guiPalette->commit();
}

void Setup_Colors::cancel()
{
    guiPalette->rollback();
    Palette::ColorType type = guiPalette->getColorTypeAt(mSelected);
    const gcn::Color *col = &guiPalette->getColor(type);
    mGradTypeSlider->setValue(guiPalette->getGradientType(type));
    const int delay = guiPalette->getGradientDelay(type);
    setEntry(mGradDelaySlider, mGradDelayText, delay);
    setEntry(mRedSlider, mRedText, col->r);
    setEntry(mGreenSlider, mGreenText, col->g);
    setEntry(mBlueSlider, mBlueText, col->b);
}

void Setup_Colors::updateGradType()
{
    if (mSelected == -1)
        return;

    mSelected = mColorBox->getSelected();
    Palette::ColorType type = guiPalette->getColorTypeAt(mSelected);
    Palette::GradientType grad = guiPalette->getGradientType(type);

    setGradTypeText(grad);

    const bool enable = (grad == Palette::STATIC || grad == Palette::PULSE);
    const bool delayEnable = (grad != Palette::STATIC);

    mGradDelayText->setEnabled(delayEnable);
    mGradDelaySlider->setEnabled(delayEnable);

    mRedText->setEnabled(enable);
    mRedSlider->setEnabled(enable);
    mGreenText->setEnabled(enable);
    mGreenSlider->setEnabled(enable);
    mBlueText->setEnabled(enable);
    mBlueSlider->setEnabled(enable);
}

void Setup_Colors::setGradTypeText(const Palette::GradientType &value)
{
    switch (value)
    {              
        case 0:
            mGradTypeText->setCaption(_("Static"));
            break;
        case 1:
            mGradTypeText->setCaption(_("Pulse"));
            break;
        case 2:
            mGradTypeText->setCaption(_("Rainbow"));
            break;
        case 3:
            mGradTypeText->setCaption( _("Spectrum"));
            break;
    }

    mGradTypeText->adjustSize();
}

void Setup_Colors::updateColor()
{
    if (mSelected == -1)
        return;

    Palette::ColorType type = guiPalette->getColorTypeAt(mSelected);
    const Palette::GradientType grad = (Palette::GradientType) mGradTypeSlider->getValue();
    const int delay = (int) mGradDelaySlider->getValue();
    guiPalette->setGradient(type, grad);
    guiPalette->setGradientDelay(type, delay);
    updateGradType();

    if (grad == Palette::STATIC)
    {
        guiPalette->setColor(type,
                static_cast<int>(mRedSlider->getValue()),
                static_cast<int>(mGreenSlider->getValue()),
                static_cast<int>(mBlueSlider->getValue()));
    }
    else if (grad == Palette::PULSE)
    {
        guiPalette->setTestColor(type, gcn::Color(
                static_cast<int>(mRedSlider->getValue()),
                static_cast<int>(mGreenSlider->getValue()),
                static_cast<int>(mBlueSlider->getValue())));
    }
}
