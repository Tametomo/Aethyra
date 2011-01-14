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

#ifndef XML_H
#define XML_H

#include <string>

#include <libxml/tree.h>

/**
 * XML helper functions.
 */
namespace XML
{
    /**
     * A helper class for parsing an XML document, which also cleans it up
     * again (RAII).
     */
    class Document
    {
        public:
            /**
             * Constructor that attempts to load the given file through the
             * resource manager. Logs errors.
             */
            Document(const std::string &filename);

            /**
             * Constructor that attempts to load an XML document from memory.
             * Does not log errors.
             *
             * @param data the string to parse as XML
             * @param size the length of the string in bytes
             */
            Document(const char *data, int size);

            /**
             * Destructor. Frees the loaded XML file.
             */
            ~Document();

            /**
             * Returns the root node of the document (or NULL if there was a
             * load error).
             */
            xmlNodePtr rootNode() const;

        private:
            xmlDocPtr mDoc;
    };

    /**
     * Gets an integer property from an xmlNodePtr.
     */
    int getProperty(xmlNodePtr node, const char *name, const int def);

    /**
     * Gets an floating point property from an xmlNodePtr.
     */
    double getFloatProperty(xmlNodePtr node, const char *name, const double def);

    /**
     * Gets a boolean property from an xmlNodePtr.
     */
    bool getBoolProperty(xmlNodePtr node, const char *name, const bool def);

    /**
     * Gets a string property from an xmlNodePtr.
     */
    std::string getProperty(xmlNodePtr node, const char *name, const std::string &def);

    /**
     * Finds the first child node with the given name
     */
    xmlNodePtr findFirstChildByName(xmlNodePtr parent, const char *name);
}

#define for_each_xml_child_node(var, parent) \
    for (xmlNodePtr var = parent->xmlChildrenNode; var; var = var->next)

#endif
