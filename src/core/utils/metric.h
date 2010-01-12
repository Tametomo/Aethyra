/*
 *  Aethyra
 *  Copyright (C) 2010  Aethyra Development Team
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

#ifndef METRIC_H
#define METRIC_H

#include <cmath>

#include "gettext.h"
#include "stringutils.h"

namespace Metric
{
    const int NUMBER_OF_PREFIXES = 21;

    struct Unit
    {
        int powerOfTen;
        std::string prefix;
        std::string abbreviation;
    };

    /// TRANSLATORS: The following strings are metric prefixes. Translate them
    ///              to the appropriate equivalent for your language.
    static Unit conversionTable[NUMBER_OF_PREFIXES] = {
        {-24, _("yocto"),  "y"},
        {-21, _("zepto"),  "z"},
        {-18, _("atto"),   "a"},
        {-15, _("fempto"), "f"},
        {-12, _("pico"),   "p"},
        {-9,  _("nano"),   "n"},
        {-6,  _("micro"),  "µ"},
        {-3,  _("milli"),  "m"},
        {-2,  _("centi"),  "c"},
        {-1,  _("deci"),   "d"},
        {0,     "",        ""},
        {1,   _("deca"),   "da"},
        {2,   _("hecto"),  "h"},
        {3,   _("kilo"),   "k"},
        {6,   _("mega"),   "M"},
        {9,   _("giga"),   "G"},
        {12,  _("tera"),   "T"},
        {15,  _("peta"),   "P"},
        {18,  _("exa"),    "E"},
        {21,  _("zetta"),  "Z"},
        {24,  _("yotta"),  "Y"},
    };

    /**
     * Simple, generic metric conversion of double values. This function will
     * take a double value, calculate its closest unit, then return a string
     * with the converted value with its metric prefix. The user of this
     * function needs to provide their own units to append to the results.
     */
    static std::string convert(const double &value, const int &precision = 2,
                               const bool &abbreviated = true)
    {
        if (value == 0.0)
            return strprintf("%.*f ", precision, value);

        int index;

        for (index = 0; index < NUMBER_OF_PREFIXES; index++)
        {
            if (pow(10.0, conversionTable[index].powerOfTen) > value)
                break;
        }

        index = (index == 0) ? 0 : index - 1;
        const double convertedValue = value / pow(10.0, conversionTable[index].powerOfTen);
        const std::string &units = abbreviated ? conversionTable[index].abbreviation :
                                                 conversionTable[index].prefix;
        std::string metric = strprintf("%.*f %s", precision, convertedValue,
                                       units.c_str());
        return metric;
    }
};

#endif // METRIC_H
