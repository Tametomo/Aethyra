/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "shoplistmodel.h"

#include "../../core/utils/dtor.h"

ShopListModel::ShopListModel(bool mergeDuplicates) :
    mMergeDuplicates(mergeDuplicates)
{
}

ShopListModel::~ShopListModel()
{
    clear();
}

int ShopListModel::getNumberOfElements()
{
    return mShopListModel.size();
}

std::string ShopListModel::getElementAt(int i)
{
    return mShopListModel.at(i)->getDisplayName();
}

void ShopListModel::addItem(int inventoryIndex, int id, int quantity, int price)
{
    ShopItem* item = NULL;

    if (mMergeDuplicates)
        item = findItem(id);

    if (item)
        item->addDuplicate (inventoryIndex, quantity);
    else
    {
        item = new ShopItem(inventoryIndex, id, quantity, price);
        mShopListModel.push_back(item);
    }
}

void ShopListModel::addItem(int id, int price)
{
    addItem(-1, id, 0, price);
}

ShopItem* ShopListModel::at(int i) const
{
    return mShopListModel.at(i);
}

void ShopListModel::erase(int i)
{
    mShopListModel.erase(mShopListModel.begin() + i);
}

void ShopListModel::clear()
{
    delete_all(mShopListModel);
    mShopListModel.clear();
}

ShopItem* ShopListModel::findItem(int id)
{
    ShopItem *item;

    std::vector<ShopItem*>::iterator it;
    for(it = mShopListModel.begin(); it != mShopListModel.end(); it++)
    {
        item = *(it);

        if (item->getId() == id)
            return item;
    }

    return NULL;
}
