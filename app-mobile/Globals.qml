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

pragma Singleton
import QtQuick

QtObject {
    id: globals

    property string appName: "ForDil"

    property int languageRefresh: 0

    property var stackView: null

    property var pageMain: null

    property var pageDiscoveryPart: null
    property var pageMarteloscope: null
    property var pageGlobalResults: null
    property var pageHotspots: null
    property var pageAdvancedResults: null

    property var pageTravailloscope: null
    property var pageValuation: null

    readonly property real bigScreenMultiplier : {
        if (Screen.width > 500 && Screen.height > 800)
        {
            return 1.5;
        }
        return 1;
    }

    function resultComponentAutoWidth(max = 600)
    {
        let width = Screen.width - 20;
        if (width > max)
        {
            width = max;
        }
        return width;
    }

    Component.onCompleted: {
        console.debug("Globals completed");
        console.debug("Screen.width", Screen.width);
        console.debug("Screen.height", Screen.height);
    }
}
