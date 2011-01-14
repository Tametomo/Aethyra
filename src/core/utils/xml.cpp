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

#include "stringutils.h"
#include "xml.h"

#include "../log.h"
#include "../resourcemanager.h"

namespace XML
{
    Document::Document(const std::string &filename):
        mDoc(NULL)
    {
        int size;
        ResourceManager *resman = ResourceManager::getInstance();
        char *data = (char*) resman->loadFile(filename.c_str(), size);

        if (data)
        {
            mDoc = xmlParseMemory(data, size);
            free(data);

            if (!mDoc)
                logger->log("Error parsing XML file %s", filename.c_str());
        }
        else
            logger->log("Error loading %s", filename.c_str());
    }

    Document::Document(const char *data, int size)
    {
        mDoc = xmlParseMemory(data, size);
    }

    Document::~Document()
    {
        if (mDoc)
            xmlFreeDoc(mDoc);
    }

    xmlNodePtr Document::rootNode() const
    {
        return mDoc ? xmlDocGetRootElement(mDoc) : 0;
    }

    int getProperty(xmlNodePtr node, const char* name, const int def)
    {
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            int val = atoi((char*)prop);
            xmlFree(prop);
            return val;
        }

        return def;
    }

    double getFloatProperty(xmlNodePtr node, const char* name, const double def)
    {
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            double val = atof((char*)prop);
            xmlFree(prop);
            return val;
        }

        return def;
    }

    bool getBoolProperty(xmlNodePtr node, const char* name, const bool def)
    {
        bool ret = def;

        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            ret = getBoolFromString((char*) prop, def);
            xmlFree(prop);
        }

        return ret;
    }

    std::string getProperty(xmlNodePtr node, const char *name, const std::string &def)
    {
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            std::string val = (char*)prop;
            xmlFree(prop);
            return val;
        }

        return def;
    }

    xmlNodePtr findFirstChildByName(xmlNodePtr parent, const char *name)
    {
        for_each_xml_child_node(child, parent)
            if (xmlStrEqual(child->name, BAD_CAST name))
                return child;

        return NULL;
    }
}
