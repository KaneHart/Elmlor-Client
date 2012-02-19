/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2012  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/widgets/setupitem.h"

#include "configuration.h"
#include "main.h"
#include "logger.h"

#include "gui/editdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/dropdown.h"
#include "gui/widgets/horizontcontainer.h"
#include "gui/widgets/inttextfield.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/tabbedarea.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/vertcontainer.h"

#include "utils/dtor.h"
#include "utils/gettext.h"


SetupItem::SetupItem(std::string text, std::string description,
                     std::string keyName, SetupTabScroll *parent,
                     std::string eventName, bool mainConfig) :
    mText(text),
    mDescription(description),
    mKeyName(keyName),
    mParent(parent),
    mEventName(eventName),
    mMainConfig(mainConfig),
    mUseDefault(false),
    mValue(""),
    mDefault(""),
    mWidget(nullptr),
    mValueType(VBOOL)
{
}

SetupItem::SetupItem(std::string text, std::string description,
                     std::string keyName, SetupTabScroll *parent,
                     std::string eventName, std::string def, bool mainConfig) :
    mText(text),
    mDescription(description),
    mKeyName(keyName),
    mParent(parent),
    mEventName(eventName),
    mMainConfig(mainConfig),
    mUseDefault(true),
    mValue(""),
    mDefault(def),
    mWidget(nullptr),
    mValueType(VBOOL)
{
}

SetupItem::~SetupItem()
{

}

Configuration *SetupItem::getConfig()
{
    if (mMainConfig)
        return &config;
    else
        return &serverConfig;
}

void SetupItem::load()
{
    Configuration *cfg = getConfig();
    if (mUseDefault)
    {
        mValue = cfg->getValue(mKeyName, mDefault);
    }
    else
    {
        switch (mValueType)
        {
            case VBOOL:
                if (cfg->getBoolValue(mKeyName))
                    mValue = "1";
                else
                    mValue = "0";
                break;
            case VSTR:
            default:
                mValue = cfg->getStringValue(mKeyName);
                break;
            case VINT:
                mValue = toString(cfg->getIntValue(mKeyName));
                break;
            case VNONE:
                break;
        }
    }
}

void SetupItem::save()
{
    Configuration *cfg = getConfig();
    cfg->setValue(mKeyName, mValue);
}

std::string SetupItem::getActionEventId()
{
    if (!mWidget)
        return "";

    return mWidget->getActionEventId();
}

void SetupItem::action(const gcn::ActionEvent &event)
{
    if (!mWidget)
        return;

    if (event.getId() == mWidget->getActionEventId())
        action();
}

void SetupItem::action()
{
    fromWidget();
}

void SetupItem::apply(std::string eventName A_UNUSED)
{
    save();
}

void SetupItem::cancel(std::string eventName A_UNUSED)
{
    load();
    toWidget();
}

void SetupItem::externalUpdated(std::string eventName A_UNUSED)
{
    load();
    toWidget();
}

SetupItemCheckBox::SetupItemCheckBox(std::string text, std::string description,
                                     std::string keyName,
                                     SetupTabScroll *parent,
                                     std::string eventName, bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, mainConfig)
{
    createControls();
}

SetupItemCheckBox::SetupItemCheckBox(std::string text, std::string description,
                                     std::string keyName,
                                     SetupTabScroll *parent,
                                     std::string eventName, std::string def,
                                     bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, def, mainConfig)
{
    createControls();
}

SetupItemCheckBox::~SetupItemCheckBox()
{
    mWidget = nullptr;
}

void SetupItemCheckBox::createControls()
{
    load();
    mCheckBox = new CheckBox(mText, mValue != "0", mParent, mEventName);
    mWidget = mCheckBox;
    mParent->getContainer()->add1(mWidget);
    mParent->addControl(this);
    mParent->addActionListener(this);
    mWidget->addActionListener(this);
}

void SetupItemCheckBox::fromWidget()
{
    if (!mCheckBox)
        return;

    if (mCheckBox->isSelected())
        mValue = "1";
    else
        mValue = "0";
}

void SetupItemCheckBox::toWidget()
{
    if (!mCheckBox)
        return;

    mCheckBox->setSelected(mValue != "0");
}


SetupItemTextField::SetupItemTextField(std::string text,
                                       std::string description,
                                       std::string keyName,
                                       SetupTabScroll *parent,
                                       std::string eventName,
                                       bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, mainConfig),
    mHorizont(nullptr),
    mLabel(nullptr),
    mTextField(nullptr),
    mButton(nullptr),
    mEditDialog(nullptr)
{
    mValueType = VSTR;
    createControls();
}

SetupItemTextField::SetupItemTextField(std::string text,
                                       std::string description,
                                       std::string keyName,
                                       SetupTabScroll *parent,
                                       std::string eventName, std::string def,
                                       bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, def, mainConfig),
    mHorizont(nullptr),
    mLabel(nullptr),
    mTextField(nullptr),
    mButton(nullptr),
    mEditDialog(nullptr)
{
    mValueType = VSTR;
    createControls();
}

SetupItemTextField::~SetupItemTextField()
{
    mHorizont = nullptr;
    mWidget = nullptr;
    mTextField = nullptr;
    mLabel = nullptr;
    mButton = nullptr;
}

void SetupItemTextField::createControls()
{
    load();
    mHorizont = new HorizontContainer(32, 2);

    mLabel = new Label(mText);
    mTextField = new TextField(mValue, true, mParent, mEventName);
    mButton = new Button(_("Edit"), mEventName + "_EDIT", mParent);
    mWidget = mTextField;
    mTextField->setWidth(200);
    mHorizont->add(mLabel);
    mHorizont->add(mTextField);
    mHorizont->add(mButton);

    mParent->getContainer()->add2(mHorizont, true, 4);
    mParent->addControl(this);
    mParent->addControl(this, mEventName + "_EDIT");
    mParent->addControl(this, mEventName + "_EDIT_OK");
    mParent->addActionListener(this);
    mWidget->addActionListener(this);
    mButton->addActionListener(this);
}

void SetupItemTextField::fromWidget()
{
    if (!mTextField)
        return;

    mValue = mTextField->getText();
}

void SetupItemTextField::toWidget()
{
    if (!mTextField)
        return;

    mTextField->setText(mValue);
}

void SetupItemTextField::action(const gcn::ActionEvent &event)
{
    if (!mTextField)
        return;

    if (event.getId() == mWidget->getActionEventId())
    {
        fromWidget();
    }
    else if (event.getId() == mEventName + "_EDIT")
    {
        mEditDialog =  new EditDialog(mText, mTextField->getText(),
            mEventName + "_EDIT_OK");
        mEditDialog->addActionListener(this);
    }
    else if (event.getId() == mEventName + "_EDIT_OK")
    {
        mTextField->setText(mEditDialog->getMsg());
        mEditDialog = nullptr;
    }
}

void SetupItemTextField::apply(std::string eventName)
{
    if (eventName != mEventName)
        return;

    fromWidget();
    save();
}

SetupItemIntTextField::SetupItemIntTextField(std::string text,
                                             std::string description,
                                             std::string keyName,
                                             SetupTabScroll *parent,
                                             std::string eventName,
                                             int min, int max,
                                             bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, mainConfig),
    mHorizont(nullptr),
    mLabel(nullptr),
    mTextField(nullptr),
    mButton(nullptr),
    mMin(min),
    mMax(max),
    mEditDialog(nullptr)
{
    mValueType = VSTR;
    createControls();
}

SetupItemIntTextField::SetupItemIntTextField(std::string text,
                                             std::string description,
                                             std::string keyName,
                                             SetupTabScroll *parent,
                                             std::string eventName,
                                             int min, int max,
                                             std::string def,
                                             bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, def, mainConfig),
    mHorizont(nullptr),
    mLabel(nullptr),
    mTextField(nullptr),
    mButton(nullptr),
    mMin(min),
    mMax(max),
    mEditDialog(nullptr)
{
    mValueType = VSTR;
    createControls();
}

SetupItemIntTextField::~SetupItemIntTextField()
{
    mHorizont = nullptr;
    mWidget = nullptr;
    mTextField = nullptr;
    mLabel = nullptr;
    mButton = nullptr;
}

void SetupItemIntTextField::createControls()
{
    load();
    mHorizont = new HorizontContainer(32, 2);

    mLabel = new Label(mText);
    mTextField = new IntTextField(atoi(mValue.c_str()), mMin, mMax, true, 30);
    mTextField->setActionEventId(mEventName);
    mTextField->addActionListener(mParent);

    mButton = new Button(_("Edit"), mEventName + "_EDIT", mParent);
    mWidget = mTextField;
    mTextField->setWidth(50);
    mHorizont->add(mLabel);
    mHorizont->add(mTextField);
    mHorizont->add(mButton);

    mParent->getContainer()->add2(mHorizont, true, 4);
    mParent->addControl(this);
    mParent->addControl(this, mEventName + "_EDIT");
    mParent->addControl(this, mEventName + "_EDIT_OK");
    mParent->addActionListener(this);
    mWidget->addActionListener(this);
    mButton->addActionListener(this);
}

void SetupItemIntTextField::fromWidget()
{
    if (!mTextField)
        return;

    mValue = mTextField->getText();
}

void SetupItemIntTextField::toWidget()
{
    if (!mTextField)
        return;

    mTextField->setText(mValue);
}

void SetupItemIntTextField::action(const gcn::ActionEvent &event)
{
    if (!mTextField)
        return;

    if (event.getId() == mWidget->getActionEventId())
    {
        fromWidget();
    }
    else if (event.getId() == mEventName + "_EDIT")
    {
        mEditDialog =  new EditDialog(mText, mTextField->getText(),
            mEventName + "_EDIT_OK");
        mEditDialog->addActionListener(this);
    }
    else if (event.getId() == mEventName + "_EDIT_OK")
    {
        mTextField->setValue(atoi(mEditDialog->getMsg().c_str()));
        mEditDialog = nullptr;
    }
}

void SetupItemIntTextField::apply(std::string eventName)
{
    if (eventName != mEventName)
        return;

    fromWidget();
    save();
}


SetupItemLabel::SetupItemLabel(std::string text, std::string description,
                               SetupTabScroll *parent, bool separator) :
    SetupItem(text, description, "", parent, "", "", true),
    mLabel(nullptr),
    mIsSeparator(separator)
{
    mValueType = VNONE;
    createControls();
}

SetupItemLabel::~SetupItemLabel()
{
    mWidget = nullptr;
    mLabel = nullptr;
}

void SetupItemLabel::createControls()
{
    if (mIsSeparator)
    {
        const std::string str = " \342\200\225\342\200\225\342\200\225"
            "\342\200\225\342\200\225 ";
        mLabel = new Label(str + mText + str);
    }
    else
    {
        mLabel = new Label(mText);
    }

    mWidget = mLabel;
    mParent->getContainer()->add1(mWidget);
    mParent->addControl(this);
    mParent->addActionListener(this);
    mWidget->addActionListener(this);
}

void SetupItemLabel::fromWidget()
{
}

void SetupItemLabel::toWidget()
{
}

void SetupItemLabel::action(const gcn::ActionEvent &event A_UNUSED)
{
}

void SetupItemLabel::apply(std::string eventName A_UNUSED)
{
}


SetupItemDropDown::SetupItemDropDown(std::string text,
                                     std::string description,
                                     std::string keyName,
                                     SetupTabScroll *parent,
                                     std::string eventName,
                                     gcn::ListModel *model,
                                     bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, mainConfig),
    mHorizont(nullptr),
    mLabel(nullptr),
    mModel(model),
    mDropDown(nullptr)
{
    mValueType = VSTR;
    createControls();
}

SetupItemDropDown::SetupItemDropDown(std::string text,
                                     std::string description,
                                     std::string keyName,
                                     SetupTabScroll *parent,
                                     std::string eventName,
                                     gcn::ListModel *model,
                                     std::string def,
                                     bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, def, mainConfig),
    mHorizont(nullptr),
    mLabel(nullptr),
    mModel(model),
    mDropDown(nullptr)
{
    mValueType = VSTR;
    createControls();
}

SetupItemDropDown::~SetupItemDropDown()
{
    mHorizont = nullptr;
    mWidget = nullptr;
    mModel = nullptr;
    mDropDown = nullptr;
    mLabel = nullptr;
}

void SetupItemDropDown::createControls()
{
    load();
    mHorizont = new HorizontContainer(32, 2);

    mLabel = new Label(mText);
    mDropDown = new DropDown(mModel);
    mDropDown->setActionEventId(mEventName);
    mDropDown->addActionListener(mParent);

    mWidget = mDropDown;
//    mTextField->setWidth(50);
    mHorizont->add(mLabel);
    mHorizont->add(mDropDown);

    mParent->getContainer()->add2(mHorizont, true, 4);
    mParent->addControl(this);
    mParent->addActionListener(this);
    mWidget->addActionListener(this);
}

void SetupItemDropDown::fromWidget()
{
    if (!mDropDown)
        return;

    mValue = mDropDown->getSelectedString();
}

void SetupItemDropDown::toWidget()
{
    if (!mDropDown)
        return;

    mDropDown->setSelectedString(mValue);
}






SetupItemSlider::SetupItemSlider(std::string text, std::string description,
                                 std::string keyName, SetupTabScroll *parent,
                                 std::string eventName, double min, double max,
                                 bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, mainConfig),
    mHorizont(nullptr),
    mLabel(nullptr),
    mSlider(nullptr),
    mMin(min),
    mMax(max)
{
    mValueType = VSTR;
    createControls();
}

SetupItemSlider::SetupItemSlider(std::string text, std::string description,
                                 std::string keyName, SetupTabScroll *parent,
                                 std::string eventName, double min, double max,
                                 std::string def, bool mainConfig) :
    SetupItem(text, description, keyName, parent, eventName, def, mainConfig),
    mHorizont(nullptr),
    mLabel(nullptr),
    mSlider(nullptr),
    mMin(min),
    mMax(max)
{
    mValueType = VSTR;
    createControls();
}

SetupItemSlider::~SetupItemSlider()
{
    mHorizont = nullptr;
    mWidget = nullptr;
    mSlider = nullptr;
    mLabel = nullptr;
}

void SetupItemSlider::createControls()
{
    load();
    mHorizont = new HorizontContainer(32, 2);

    mLabel = new Label(mText);
    mSlider = new Slider(mMin, mMax);
    mSlider->setActionEventId(mEventName);
    mSlider->addActionListener(mParent);
    mSlider->setValue(atof(mValue.c_str()));
    mSlider->setHeight(30);

    mWidget = mSlider;
    mSlider->setWidth(150);
    mSlider->setHeight(40);
    mHorizont->add(mLabel);
    mHorizont->add(mSlider, -10);

    mParent->getContainer()->add2(mHorizont, true, 4);
    mParent->addControl(this);
    mParent->addActionListener(this);
    mWidget->addActionListener(this);
}

void SetupItemSlider::fromWidget()
{
    if (!mSlider)
        return;

    mValue = toString(mSlider->getValue());
}

void SetupItemSlider::toWidget()
{
    if (!mSlider)
        return;

    mSlider->setValue(atof(mValue.c_str()));
}

void SetupItemSlider::action(const gcn::ActionEvent &event A_UNUSED)
{
    fromWidget();
}

void SetupItemSlider::apply(std::string eventName)
{
    if (eventName != mEventName)
        return;

    fromWidget();
    save();
}
