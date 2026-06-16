/*
    Copyright 2025 - Present MENDELU

    This file is part of ForDil.

    ForDil is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ForDil is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ForDil. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef C_MAP_PAINTER_H
#define C_MAP_PAINTER_H

#include <QPainter>
#include "c_data.h"

class MapPainter
{
public:
    MapPainter();

    static void draw(QPainter &painter, Data &data);

    static void click(SelectedTree &selectedTree,
                      SelectedHotspot &selectedHotspot,
                      SelectedSpot &selectedSpot,
                      SelectedStop &selectedStop,
                      SelectedPoint &selectedPoint,
                      Data &data,
                      double rx,
                      double ry);
};

#endif // C_MAP_PAINTER_H
