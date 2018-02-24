/*
 * Copyright 2016-2017 Matthieu Gallien <matthieu_gallien@yahoo.fr>
 * Copyright 2017 Alexander Stippich <a.stippich@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.2
import QtQuick.Controls 1.4 as Controls1
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import org.kde.elisa 1.0

FocusScope {
    id: mediaTrack

    property string title
    property var coverImage
    property string artist
    property string albumName
    property string albumArtist
    property string trackResource
    property string duration
    property int trackNumber
    property int discNumber
    property int rating
    property bool isFirstTrackOfDisc
    property bool isSingleDiscAlbum
    property var trackData
    property bool isAlternateColor
    property bool detailedView: true

    signal clicked()
    signal enqueue(var data)
    signal replaceAndPlay(var data)

    Controls1.Action {
        id: replaceAndPlayAction
        text: i18nc("Clear play list and enqueue current track", "Play Now and Replace Play List")
        iconName: "media-playback-start"
        onTriggered: replaceAndPlay(trackData)
    }

    Controls1.Action {
        id: enqueueAction
        text: i18nc("Enqueue current track", "Enqueue")
        iconName: "media-track-add-amarok"
        onTriggered: enqueue(trackData)
    }

    Controls1.Action {
        id: viewDetailsAction
        text: i18nc("Show track metadata", "View Details")
        iconName: "help-about"
        onTriggered: {
            if (metadataLoader.active === false) {
                metadataLoader.active = true
            }
            else {
                metadataLoader.item.close();
                metadataLoader.active = false
            }
        }
    }

    Keys.onReturnPressed: enqueueToPlaylist(trackData)
    Keys.onEnterPressed: enqueueToPlaylist(trackData)

    Loader {
        id: metadataLoader
        active: false
        onLoaded: item.show()

        sourceComponent:  MediaTrackMetadataView {
            trackTitle: mediaTrack.title
            artist: mediaTrack.artist
            albumArtist: mediaTrack.albumArtist
            albumName: mediaTrack.albumName
            duration: mediaTrack.duration
            resource: mediaTrack.trackResource
            rating: mediaTrack.rating
            trackNumber: mediaTrack.trackNumber
            discNumber: mediaTrack.discNumber

            onRejected: metadataLoader.active = false;
        }
    }

    Rectangle {
        id: rowRoot

        anchors.fill: parent

        color: (isAlternateColor ? myPalette.alternateBase : myPalette.base)

        MouseArea {
            id: hoverArea

            anchors.fill: parent

            hoverEnabled: true
            focus: true
            acceptedButtons: Qt.LeftButton

            onClicked: {
                hoverArea.forceActiveFocus()
                mediaTrack.clicked()
            }

            onDoubleClicked: enqueue(trackData)

            RowLayout {
                anchors.fill: parent
                spacing: 0

                LabelWithToolTip {
                    id: mainLabel

                    visible: !detailedView

                    text: {
                        var text = "<b>";

                        text += (trackNumber > 0) ? trackNumber + "-" : "";
                        text += title + "</b>";

                        text += (artist !== albumArtist) ? " - <i>" + artist + "</i>" : "";

                        return text;
                    }

                    horizontalAlignment: Text.AlignLeft

                    color: myPalette.text

                    Layout.leftMargin: {
                        if (!LayoutMirroring.enabled)
                            return (!isSingleDiscAlbum ? elisaTheme.layoutHorizontalMargin * 4 : elisaTheme.layoutHorizontalMargin)
                        else
                            return 0
                    }
                    Layout.rightMargin: {
                        if (LayoutMirroring.enabled)
                            return (!isSingleDiscAlbum ? elisaTheme.layoutHorizontalMargin * 4 : elisaTheme.layoutHorizontalMargin)
                        else
                            return 0
                    }
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    Layout.fillWidth: true

                    elide: Text.ElideRight
                }

                Item {
                    Layout.preferredHeight: mediaTrack.height * 0.9
                    Layout.preferredWidth: mediaTrack.height * 0.9

                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                    visible: detailedView

                    Image {
                        id: coverImageElement

                        anchors.fill: parent

                        sourceSize.width: mediaTrack.height * 0.9
                        sourceSize.height: mediaTrack.height * 0.9
                        fillMode: Image.PreserveAspectFit
                        smooth: true

                        source: (coverImage ? coverImage : Qt.resolvedUrl(elisaTheme.defaultAlbumImage))

                        asynchronous: true

                        layer.enabled: coverImage === '' ? false : true

                        layer.effect: DropShadow {
                            source: coverImageElement

                            radius: 10
                            spread: 0.1
                            samples: 21

                            color: myPalette.shadow
                        }
                    }
                }

                ColumnLayout {
                    visible: detailedView

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignLeft

                    spacing: 0

                    LabelWithToolTip {
                        id: mainLabelDetailed

                        text: (trackNumber > -1) ? trackNumber + ' - ' + title : title
                        horizontalAlignment: Text.AlignLeft

                        font.weight: Font.Bold
                        color: myPalette.text

                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.leftMargin: !LayoutMirroring.enabled ? elisaTheme.layoutHorizontalMargin : 0
                        Layout.rightMargin: LayoutMirroring.enabled ? elisaTheme.layoutHorizontalMargin : 0
                        Layout.fillWidth: true
                        Layout.topMargin: elisaTheme.layoutVerticalMargin / 2

                        elide: Text.ElideRight
                    }

                    Item {
                        Layout.fillHeight: true
                    }

                    LabelWithToolTip {
                        id: artistLabel

                        text: (isSingleDiscAlbum ? artist + ' - ' + albumName : artist + ' - ' + albumName + ' - CD ' + discNumber)
                        horizontalAlignment: Text.AlignLeft

                        font.weight: Font.Light
                        font.italic: true
                        color: myPalette.text

                        Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                        Layout.leftMargin: !LayoutMirroring.enabled ? elisaTheme.layoutHorizontalMargin : 0
                        Layout.rightMargin: LayoutMirroring.enabled ? elisaTheme.layoutHorizontalMargin : 0
                        Layout.fillWidth: true
                        Layout.bottomMargin: elisaTheme.layoutVerticalMargin / 2

                        elide: Text.ElideRight
                    }
                }

                Loader {
                    id: hoverLoader
                    active: false

                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                    z: 1
                    opacity: 0

                    sourceComponent: Row {
                        anchors.centerIn: parent

                        Controls1.ToolButton {
                            id: detailsButton

                            height: elisaTheme.delegateHeight
                            width: elisaTheme.delegateHeight

                            action: viewDetailsAction
                        }

                        Controls1.ToolButton {
                            id: enqueueButton

                            height: elisaTheme.delegateHeight
                            width: elisaTheme.delegateHeight

                            action: enqueueAction
                        }

                        Controls1.ToolButton {
                            id: clearAndEnqueueButton

                            height: elisaTheme.delegateHeight
                            width: elisaTheme.delegateHeight

                            action: replaceAndPlayAction
                        }
                    }
                }

                RatingStar {
                    id: ratingWidget

                    starSize: elisaTheme.ratingStarSize

                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.leftMargin: elisaTheme.layoutHorizontalMargin
                    Layout.rightMargin: elisaTheme.layoutHorizontalMargin
                }

                LabelWithToolTip {
                    id: durationLabel

                    text: duration

                    font.weight: Font.Light
                    color: myPalette.text

                    elide: Text.ElideRight

                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.rightMargin: !LayoutMirroring.enabled ? elisaTheme.layoutHorizontalMargin : 0
                    Layout.leftMargin: LayoutMirroring.enabled ? elisaTheme.layoutHorizontalMargin : 0
                }
            }
        }
    }

    states: [
        State {
            name: 'notSelected'
            when: !hoverArea.containsMouse && !mediaTrack.activeFocus
            PropertyChanges {
                target: hoverLoader
                active: false
            }
            PropertyChanges {
                target: hoverLoader
                opacity: 0.0
            }
            PropertyChanges {
                target: rowRoot
                color: (isAlternateColor ? myPalette.alternateBase : myPalette.base)
            }
        },
        State {
            name: 'hoveredOrSelected'
            when: hoverArea.containsMouse || mediaTrack.activeFocus
            PropertyChanges {
                target: hoverLoader
                active: true
            }
            PropertyChanges {
                target: hoverLoader
                opacity: 1.0
            }
            PropertyChanges {
                target: rowRoot
                color: myPalette.mid
            }
        }
    ]

    transitions: [
        Transition {
            to: 'hoveredOrSelected'
            SequentialAnimation {
                PropertyAction {
                    properties: "active"
                }
                ParallelAnimation {
                    NumberAnimation {
                        properties: "opacity"
                        easing.type: Easing.InOutQuad
                        duration: 250
                    }
                    ColorAnimation {
                        properties: "color"
                        duration: 250
                    }
                }
            }
        },
        Transition {
            to: 'notSelected'
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation {
                        properties: "opacity"
                        easing.type: Easing.InOutQuad
                        duration: 250
                    }
                    ColorAnimation {
                        properties: "color"
                        duration: 250
                    }
                }
                PropertyAction {
                    properties: "active"
                }
            }
        }
    ]
}
