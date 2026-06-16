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

import QtCore
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtPositioning
import QtSensors
import ForDil

// The main application window
ApplicationWindow {
    id: pageMain
    width: 480
    height: 640
    flags: Qt.Window | Qt.ExpandedClientAreaHint

    visible: true

    palette {
        window: "white"
        text: "black"
    }

    function refreshTr() {
        marteloscopeTabBarSelection.text = qsTr("Selection");
        marteloscopeTabBarResults.text = qsTr("Results");
        marteloscopeTabBarHotspots.text = qsTr("Hotspots");
        marteloscopeTabBarAdvancedResults.text = qsTr("Advanced Results");

        travailloscopeTabBarWorks.text = qsTr("Works");
        travailloscopeTabBarValuation.text = qsTr("Valuation");
    }

    // Ensure Globals is initialized by accessing it early
    Component.onCompleted: {
        Globals.pageMain = pageMain
        console.debug("Application \"" + Globals.appName + "\" initialized");
    }

    // Application title
    title: "ForDil"

    // Position
    property bool pageWantsPosition: false

    LocationPermission {
        id: locationPermission
        accuracy: LocationPermission.Precise
        availability: LocationPermission.WhenInUse
        Component.onCompleted: request();
    }

    PositionSource {
        id: positionSource
        updateInterval: 250  // Update every second 1000
        active: Backend.isGpsEnabled && pageMain.pageWantsPosition && locationPermission.status === Qt.Granted

        onPositionChanged: {
            var coord = positionSource.position.coordinate

            if (stackView.currentItem &&
                (stackView.currentItem.objectName === "PageMarteloscope" ||
                 stackView.currentItem.objectName === "PageHotspots" ||
                 stackView.currentItem.objectName === "PageTravailloscope" ||
                 stackView.currentItem.objectName === "PageValuation" ||
                 stackView.currentItem.objectName === "PageDiscoveryPart") &&
                stackView.currentItem.mapView &&
                stackView.currentItem.mapView.updateLocation)
            {
                stackView.currentItem.mapView.updateLocation(coord.latitude, coord.longitude);
            }
        }
    }

    // Compass
    function orientationToDegrees(o) {
        switch (o) {
        case Qt.PortraitOrientation: return 0
        case Qt.LandscapeOrientation: return 90
        case Qt.InvertedPortraitOrientation: return 180
        case Qt.InvertedLandscapeOrientation: return 270
        default: return 0
        }
    }

    function correctedAzimuth(rawAzimuth) {
        const uiDeg = pageMain.orientationToDegrees(Screen.orientation)
        const nativeDeg = pageMain.orientationToDegrees(Screen.nativeOrientation)
        const delta = (uiDeg - nativeDeg + 360) % 360
        return (rawAzimuth + delta) % 360
    }

    Compass {
        id: compass
        active: pageMain.pageWantsPosition
        axesOrientationMode: Sensor.FixedOrientation

        onReadingChanged: {
            if (!reading) {
                return;
            }

            const rawAzimuth = (reading as CompassReading).azimuth;
            const newAzimuth = pageMain.correctedAzimuth(rawAzimuth);

            if (stackView.currentItem &&
                (stackView.currentItem.objectName === "PageMarteloscope" ||
                 stackView.currentItem.objectName === "PageHotspots" ||
                 stackView.currentItem.objectName === "PageTravailloscope" ||
                 stackView.currentItem.objectName === "PageValuation" ||
                 stackView.currentItem.objectName === "PageDiscoveryPart") &&
                stackView.currentItem.mapView &&
                stackView.currentItem.mapView.updateAzimuth)
            {
                stackView.currentItem.mapView.updateAzimuth(newAzimuth);
            }
        }
    }

    // Application focus Rectangle
    Rectangle {
        id: applicationRectangle
        anchors.fill: parent

        // anchors.topMargin: pageMain.SafeArea.margins.top
        // anchors.bottomMargin: pageMain.SafeArea.margins.bottom
        // anchors.leftMargin: pageMain.SafeArea.margins.left
        // anchors.rightMargin: pageMain.SafeArea.margins.right

        // Keys
        focus: true
        Keys.onReleased: (event) => {
            // Navigate back
            if (event.key === Qt.Key_Back) {
                event.accepted = true
                pageMain.popPage()
            }
        }

        // Application tool bar
        ToolBar {
            id: toolBar
            width: parent.width
            height: 40

            anchors.top: parent.top

            property int iconHeight: 30
            property int logoHeight: 25

            visible: true

            background: Rectangle {
                // color: "lightgreen"
                color: "#41B28E"
            }

            RowLayout {
                anchors.fill: parent
                spacing: 0  // We'll use spacers instead

                // Fixed-width spacer at the front
                Rectangle {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: toolBar.height
                    color: "transparent"
                }

                // Home
                Image {
                    source: "qrc:/images/ui_home.png"
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredWidth: toolBar.iconHeight
                    Layout.preferredHeight: toolBar.iconHeight
                    Layout.fillHeight: true

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            pageMain.pushPage("PageHome.qml")
                        }
                    }
                }

                // Fill the space to center the image
                Item {
                    Layout.fillWidth: true
                }

                // Logo
                Image {
                    source: "qrc:/images/ui_logo_fordil_toolbar.png"
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredWidth: 1380 / (200 / toolBar.logoHeight)
                    Layout.preferredHeight: toolBar.logoHeight
                    Layout.fillHeight: true
                }

                // Fill the space to center the image
                Item {
                    Layout.fillWidth: true
                }

                // Settings
                Image {
                    source: "qrc:/images/ui_settings.png"
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredWidth: toolBar.iconHeight
                    Layout.preferredHeight: toolBar.iconHeight
                    Layout.fillHeight: true

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            pageMain.pushPage("PageSettings.qml")
                        }
                    }
                }

                // Fixed-width spacer at the end
                Rectangle {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: toolBar.height
                    color: "transparent"
                }
            }
        }

        // Marteloscope Tab
        TabBar {
            id: marteloscopeTabBar
            width: parent.width
            height: visible ? implicitHeight : 0
            anchors.top: toolBar.bottom
            visible: false

            property int tabCurrentIndex: 0

            TabButton {
                id: marteloscopeTabBarSelection
                text: qsTr("Selection")
                checked: marteloscopeTabBar.tabCurrentIndex === 0
                contentItem: Text {
                    text: marteloscopeTabBarSelection.text
                    color: marteloscopeTabBarSelection.checked ? "#41B28E" : "#26787A"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: {
                    marteloscopeTabBar.tabCurrentIndex = 0
                    if (Globals.stackView.currentItem !== Globals.pageMarteloscope) {
                        Backend.resetInteraction()
                        onClicked: {
                            Backend.setHotspotsReasonText("None")
                            Globals.pageMain.pushPage("PageMarteloscope.qml")
                        }
                    }
                }
            }

            TabButton {
                id: marteloscopeTabBarResults
                text: qsTr("Results")
                checked: marteloscopeTabBar.tabCurrentIndex === 1
                contentItem: Text {
                    text: marteloscopeTabBarResults.text
                    color: marteloscopeTabBarResults.checked ? "#41B28E" : "#26787A"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: {
                    marteloscopeTabBar.tabCurrentIndex = 1
                    if (Globals.stackView.currentItem !== Globals.pageGlobalResults) {
                        onClicked: {
                            Globals.pageMain.pushPage("PageGlobalResults.qml")
                        }
                    }
                }
            }

            TabButton {
                id: marteloscopeTabBarHotspots
                text: qsTr("Hotspots")
                checked: marteloscopeTabBar.tabCurrentIndex === 2
                contentItem: Text {
                    text: marteloscopeTabBarHotspots.text
                    color: marteloscopeTabBarHotspots.checked ? "#41B28E" : "#26787A"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: {
                    marteloscopeTabBar.tabCurrentIndex = 2
                    if (Globals.stackView.currentItem !== Globals.pageHotspots) {
                        onClicked: {
                            Globals.pageMain.pushPage("PageHotspots.qml")
                        }
                    }
                }
            }

            TabButton {
                id: marteloscopeTabBarAdvancedResults
                text: qsTr("Advanced Results")
                checked: marteloscopeTabBar.tabCurrentIndex === 3
                contentItem: Text {
                    text: marteloscopeTabBarAdvancedResults.text
                    color: marteloscopeTabBarAdvancedResults.checked ? "#41B28E" : "#26787A"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: {
                    marteloscopeTabBar.tabCurrentIndex = 3
                    if (Globals.stackView.currentItem !== Globals.pageAdvancedResults) {
                        onClicked: {
                            Globals.pageMain.pushPage("PageAdvancedResults.qml")
                        }
                    }
                }
            }
        }

        // Travailloscope Tab
        TabBar {
            id: travailloscopeTabBar
            width: parent.width
            height: visible ? implicitHeight : 0
            anchors.top: marteloscopeTabBar.bottom
            visible: false

            property int tabCurrentIndex: 0

            TabButton {
                id: travailloscopeTabBarWorks
                text: qsTr("Works")
                checked: travailloscopeTabBar.tabCurrentIndex === 0
                contentItem: Text {
                    text: travailloscopeTabBarWorks.text
                    color: travailloscopeTabBarWorks.checked ? "#41B28E" : "#26787A"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: {
                    travailloscopeTabBar.tabCurrentIndex = 0
                    if (Globals.stackView.currentItem !== Globals.pageTravailloscope) {
                        onClicked: {
                            Backend.setValuationReasonText("None")
                            Globals.pageMain.pushPage("PageTravailloscope.qml")
                        }
                    }
                }
            }

            TabButton {
                id: travailloscopeTabBarValuation
                text: qsTr("Valuation")
                checked: travailloscopeTabBar.tabCurrentIndex === 1
                contentItem: Text {
                    text: travailloscopeTabBarValuation.text
                    color: travailloscopeTabBarValuation.checked ? "#41B28E" : "#26787A"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: {
                    travailloscopeTabBar.tabCurrentIndex = 1
                    if (Globals.stackView.currentItem !== Globals.pageValuation) {
                        onClicked: {
                            Globals.pageMain.pushPage("PageValuation.qml")
                        }
                    }
                }
            }
        }

        // Application content
        StackView {
            id: stackView
            anchors.top: travailloscopeTabBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: logWindow.top

            visible: true

            // Disable all transitions
            pushEnter: null
            pushExit: null
            popEnter: null
            popExit: null
            replaceEnter: null
            replaceExit: null

            // Initial page
            Component.onCompleted: {
                console.debug("stackView completed");
                Globals.stackView = stackView;
                pageMain.pushPage(Qt.resolvedUrl("PageHome.qml"));
            }

            // Tab activation
            onCurrentItemChanged: {
                if (currentItem === null)
                {
                    console.debug("onCurrentItemChanged null page")
                    marteloscopeTabBar.visible = false
                    travailloscopeTabBar.visible = false
                    Globals.pageMain.pageWantsPosition = false
                } else if (currentItem === Globals.pageMarteloscope) {
                    console.debug("onCurrentItemChanged pageMarteloscope")
                    if (Backend.isExerciseFinished)
                    {
                        marteloscopeTabBar.visible = true
                        marteloscopeTabBar.currentIndex = 0
                    }
                    else
                    {
                        marteloscopeTabBar.visible = false
                        marteloscopeTabBar.currentIndex = 0
                    }
                    travailloscopeTabBar.visible = false
                    Globals.pageMain.pageWantsPosition = true
                    Backend.setHotspotsReasonText("None")
                } else if (currentItem === Globals.pageGlobalResults) {
                    console.debug("onCurrentItemChanged pageGlobalResults")
                    marteloscopeTabBar.visible = true
                    marteloscopeTabBar.currentIndex = 1
                    travailloscopeTabBar.visible = false
                    Globals.pageMain.pageWantsPosition = false
                } else if (currentItem === Globals.pageHotspots) {
                    console.debug("onCurrentItemChanged pageHotspots")
                    marteloscopeTabBar.visible = true
                    marteloscopeTabBar.currentIndex = 2
                    travailloscopeTabBar.visible = false
                    Globals.pageMain.pageWantsPosition = true
                } else if (currentItem === Globals.pageAdvancedResults) {
                    console.debug("onCurrentItemChanged pageAdvancedResults")
                    marteloscopeTabBar.visible = true
                    marteloscopeTabBar.currentIndex = 3
                    travailloscopeTabBar.visible = false
                    Globals.pageMain.pageWantsPosition = false
                } else if (currentItem === Globals.pageTravailloscope) {
                    console.debug("onCurrentItemChanged pageTravailloscope")
                    if (Backend.isExerciseFinished)
                    {
                        travailloscopeTabBar.visible = true
                        travailloscopeTabBar.currentIndex = 0
                    }
                    else
                    {
                        travailloscopeTabBar.visible = false
                        travailloscopeTabBar.currentIndex = 0
                    }

                    marteloscopeTabBar.visible = false
                    Globals.pageMain.pageWantsPosition = true
                    Backend.setValuationReasonText("None")
                } else if (currentItem === Globals.pageValuation) {
                    console.debug("onCurrentItemChanged pageValuation")
                    marteloscopeTabBar.visible = false
                    travailloscopeTabBar.visible = true
                    travailloscopeTabBar.currentIndex = 1
                    Globals.pageMain.pageWantsPosition = true
                } else if (currentItem === Globals.pageDiscoveryPart) {
                    console.debug("onCurrentItemChanged pageDiscoveryPart")
                    marteloscopeTabBar.visible = false
                    travailloscopeTabBar.visible = false
                    Globals.pageMain.pageWantsPosition = true
                }
                else
                {
                    console.debug("onCurrentItemChanged some page")
                    marteloscopeTabBar.visible = false
                    travailloscopeTabBar.visible = false
                    pageMain.pageWantsPosition = false
                }
            }
        }

        // Log
        LogWindow {
            id: logWindow
            visible: Backend.isDebugBuild && Backend.isDebugModeEnabled
            height: visible ? parent.height / 3 : 0
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }
    } // Application focus Rectangle

    // History
    property var pageHistory: []

    function canPushPage(pageUrl) {
        if ( (!Backend.isExerciseFinished &&
              (pageUrl === "PageGlobalResults.qml" ||
               pageUrl === "PageHotspots.qml" ||
               pageUrl === "PageAdvancedResults.qml" ||
               pageUrl === "PageValuation.qml"))
            ||
             (Backend.loadedSystem() !== "Marteloscope" &&
              (pageUrl === "PageMarteloscope.qml" ||
               pageUrl === "PageGlobalResults.qml" ||
               pageUrl === "PageHotspots.qml" ||
               pageUrl === "PageAdvancedResults.qml"))
            ||
             (Backend.loadedSystem() !== "Travailoscope" &&
              (pageUrl === "PageTravailloscope.qml" ||
               pageUrl === "PageValuation.qml"))
           )
        {
            return false
        }

        return true
    }

    function canPopPage() {
        if ( Backend.loadedSystem() === "Marteloscope" &&
             Backend.loadedDiscoveryPart()
           )
        {
            return false
        }

        return true
    }

    function pushPage(pageUrl) {
        console.debug("push page ", pageUrl)
        pageHistory.push(pageUrl)
        stackView.clear()
        stackView.push(pageUrl)
    }

    function popPage() {
        // if (!canPopPage()) {
        //     return
        // }

        while (pageHistory.length > 1) {
            console.debug("pop page")
            pageHistory.pop()  // remove current
            let previousPage = pageHistory[pageHistory.length - 1]
            if (canPushPage(previousPage)) {
                stackView.clear()
                stackView.push(previousPage)
                break
            }
        }
    }
}
