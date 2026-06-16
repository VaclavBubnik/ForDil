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

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import ForDil
import "." 1.0

Item {
    id: root

    default property alias content: contentData.data
    property bool showScrollbar: true

    function resetView() {
        viewport.panX = 0
        viewport.panY = 0
        viewport.zoom = viewport.minZoom
    }

    // Custom always-visible vertical scrollbar
    Rectangle {
        id: scrollbarTrack
        width: 6
        color: "gray"
        radius: 3
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 2
        visible: root.showScrollbar && (viewport.contentHeight > viewport.height)
        opacity: 0.4

        Rectangle {
            id: scrollbarThumb
            width: parent.width
            color: "black"
            radius: 3

            height: Math.max(30, (viewport.height / viewport.contentHeight) * scrollbarTrack.height)

            y: {
                const scrollableContent = Math.max(0, viewport.contentHeight - viewport.height)
                const maxThumbY = Math.max(0, scrollbarTrack.height - height)

                if (scrollableContent <= 0)
                    return 0

                const scrollPos = Math.max(0, Math.min(-viewport.panY, scrollableContent))
                return (scrollPos / scrollableContent) * maxThumbY
            }
        }
    }

    // View
    Item {
        id: viewport
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: scrollbarTrack.left
        anchors.rightMargin: 4
        clip: true

        property real panX: 0
        property real panY: 0
        property real zoom: 1.0
        property real minZoom: 1.0
        property real maxZoom: 5.0

        property real contentWidth:  contentData.width  * zoom
        property real contentHeight: contentData.height * zoom

        property bool dragBlockedAfterPinch: false

        Timer {
            id: dragUnlockTimer
            interval: 120
            repeat: false
            onTriggered: viewport.dragBlockedAfterPinch = false
        }

        function clamp(v, lo, hi) {
            return Math.max(lo, Math.min(hi, v))
        }

        function clampPan() {
            const scaledWidth = contentBase.width * zoom
            const scaledHeight = contentBase.height * zoom

            panX = clamp(panX, Math.min(0, width - scaledWidth), 0)
            panY = clamp(panY, Math.min(0, height - scaledHeight), 0)
        }

        Item {
            id: contentSurface
            x: viewport.panX
            y: viewport.panY
            scale: viewport.zoom
            transformOrigin: Item.TopLeft

            Item {
                id: contentBase
                width: contentData.width
                height: contentData.height

                // Content
                Rectangle {
                    id: contentData
                    width: viewport.width
                    height: childrenRect.height
                }
            }
        }

        Item {
            id: inputLayer
            anchors.fill: parent

            DragHandler {
                id: drag
                target: null
                minimumPointCount: 1
                maximumPointCount: 1
                enabled: !pinch.active && !viewport.dragBlockedAfterPinch
                grabPermissions: PointerHandler.TakeOverForbidden

                property real startPanX: 0
                property real startPanY: 0

                onActiveChanged: {
                    if (active) {
                        startPanX = viewport.panX
                        startPanY = viewport.panY
                    }
                }

                onTranslationChanged: {
                    viewport.panX = startPanX + activeTranslation.x
                    viewport.panY = startPanY + activeTranslation.y
                    viewport.clampPan()
                }
            }

            PinchHandler {
                id: pinch
                target: null
                minimumPointCount: 2
                maximumPointCount: 2

                property real startZoom: 1.0
                property real startPanX: 0
                property real startPanY: 0
                property real anchorX: 0
                property real anchorY: 0

                onActiveChanged: {
                    if (active) {
                        viewport.dragBlockedAfterPinch = true
                        dragUnlockTimer.stop()

                        startZoom = viewport.zoom
                        startPanX = viewport.panX
                        startPanY = viewport.panY
                        anchorX = centroid.position.x
                        anchorY = centroid.position.y
                    } else {
                        dragUnlockTimer.restart()
                    }
                }

                onScaleChanged: {
                    if (!active)
                        return

                    const newZoom = viewport.clamp(startZoom * activeScale,
                                                viewport.minZoom,
                                                viewport.maxZoom)

                    const contentPx = (anchorX - startPanX) / startZoom
                    const contentPy = (anchorY - startPanY) / startZoom

                    viewport.zoom = newZoom
                    viewport.panX = anchorX - contentPx * newZoom
                    viewport.panY = anchorY - contentPy * newZoom
                    viewport.clampPan()
                }
            }
        }
    }
}
