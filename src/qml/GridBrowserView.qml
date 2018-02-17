/*
 * Copyright 2016-2017 Matthieu Gallien <matthieu_gallien@yahoo.fr>
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
import QtQuick.Controls 2.2
import QtQuick.Window 2.2
import QtQml.Models 2.1
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0

import org.kde.elisa 1.0

FocusScope {
    id: gridView

    property bool isSubPage: false
    property string mainTitle
    property string secondaryTitle
    property url image
    property alias model: contentDirectoryView.model
    property bool showRating: true
    property bool delegateDisplaySecondaryText: true

    property var tempMediaPlayList
    property var tempMediaControl

    signal enqueue(var data)
    signal replaceAndPlay(var data)
    signal open(var innerMainTitle, var innerSecondaryTitle, var innerImage, var databaseId)
    signal goBack()

    SystemPalette {
        id: myPalette
        colorGroup: SystemPalette.Active
    }

    Theme {
        id: elisaTheme
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        NavigationActionBar {
            id: navigationBar

            mainTitle: gridView.mainTitle
            secondaryTitle: gridView.secondaryTitle
            image: gridView.image
            enableGoBack: isSubPage

            height: elisaTheme.navigationBarHeight
            Layout.preferredHeight: height
            Layout.minimumHeight: height
            Layout.maximumHeight: height
            Layout.fillWidth: true

            Binding {
                target: model
                property: 'filterText'
                value: navigationBar.filterText
            }

            Binding {
                target: model
                property: 'filterRating'
                value: navigationBar.filterRating
            }

            onEnqueue: model.enqueueToPlayList(tempMediaPlayList)

            onReplaceAndPlay: {
                tempMediaPlayList.clearPlayList()
                model.enqueueToPlayList(tempMediaPlayList)
                tempMediaControl.ensurePlay()
            }

            onGoBack: gridView.goBack()
        }

        Rectangle {
            color: myPalette.base

            Layout.fillHeight: true
            Layout.fillWidth: true

            GridView {
                id: contentDirectoryView
                anchors.topMargin: 20

                focus: true
                anchors.fill: parent

                ScrollBar.vertical: ScrollBar {
                    id: scrollBar
                }
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                TextMetrics {
                    id: secondaryLabelSize
                    text: 'example'
                }

                cellWidth: elisaTheme.gridDelegateWidth
                cellHeight: (delegateDisplaySecondaryText ? elisaTheme.gridDelegateHeight : elisaTheme.gridDelegateHeight - secondaryLabelSize.height)

                delegate: GridBrowserDelegate {
                    width: contentDirectoryView.cellWidth
                    height: contentDirectoryView.cellHeight

                    focus: true

                    mainText: model.display
                    secondaryText: model.secondaryText
                    imageUrl: model.imageUrl
                    shadowForImage: model.shadowForImage
                    containerData: model.containerData
                    delegateDisplaySecondaryText: gridView.delegateDisplaySecondaryText

                    onEnqueue: gridView.enqueue(data)
                    onReplaceAndPlay: gridView.replaceAndPlay(data)
                    onOpen: gridView.open(model.display, model.secondaryText, model.imageUrl, model.databaseId)
                    onSelected: {
                        forceActiveFocus()
                        contentDirectoryView.currentIndex = model.index
                    }
                }
            }
        }
    }
}
