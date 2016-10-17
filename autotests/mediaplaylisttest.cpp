/*
 * Copyright 2016 Matthieu Gallien <matthieu_gallien@yahoo.fr>
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

#include "mediaplaylisttest.h"

#include "mediaplaylist.h"
#include "databaseinterface.h"
#include "musicalbum.h"
#include "musicaudiotrack.h"

#include <QtTest/QtTest>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QUrl>

MediaPlayListTest::MediaPlayListTest(QObject *parent) : QObject(parent)
{
}

void MediaPlayListTest::initTestCase()
{
    qRegisterMetaType<QHash<QString,QUrl>>("QHash<QString,QUrl>");
    qRegisterMetaType<QHash<QString,QVector<MusicAudioTrack>>>("QHash<QString,QVector<MusicAudioTrack>>");
    qRegisterMetaType<QVector<qlonglong>>("QVector<qlonglong>");
    qRegisterMetaType<QHash<qlonglong,int>>("QHash<qlonglong,int>");
}

void MediaPlayListTest::simpleInitialCase()
{
    MediaPlayList myPlayList;
    DatabaseInterface myDatabaseContent;
    DatabaseInterface myDatabaseView;

    QSignalSpy rowsAboutToBeMovedSpy(&myPlayList, &MediaPlayList::rowsAboutToBeMoved);
    QSignalSpy rowsAboutToBeRemovedSpy(&myPlayList, &MediaPlayList::rowsAboutToBeRemoved);
    QSignalSpy rowsAboutToBeInsertedSpy(&myPlayList, &MediaPlayList::rowsAboutToBeInserted);
    QSignalSpy rowsMovedSpy(&myPlayList, &MediaPlayList::rowsMoved);
    QSignalSpy rowsRemovedSpy(&myPlayList, &MediaPlayList::rowsRemoved);
    QSignalSpy rowsInsertedSpy(&myPlayList, &MediaPlayList::rowsInserted);
    QSignalSpy trackHasBeenAddedSpy(&myPlayList, &MediaPlayList::trackHasBeenAdded);
    QSignalSpy databaseInterfaceChangedSpy(&myPlayList, &MediaPlayList::databaseInterfaceChanged);
    QSignalSpy persistentStateChangedSpy(&myPlayList, &MediaPlayList::persistentStateChanged);
    QSignalSpy dataChangedSpy(&myPlayList, &MediaPlayList::dataChanged);

    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeMovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(rowsMovedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(trackHasBeenAddedSpy.count(), 0);
    QCOMPARE(databaseInterfaceChangedSpy.count(), 0);
    QCOMPARE(persistentStateChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    myDatabaseContent.init(QStringLiteral("testDbDirectContent"));
    myDatabaseContent.initDatabase();
    myDatabaseContent.initRequest();

    myDatabaseView.init(QStringLiteral("testDbDirectView"));
    myDatabaseView.initDatabase();
    myDatabaseView.initRequest();

    connect(&myDatabaseContent, &DatabaseInterface::databaseChanged,
            &myDatabaseView, &DatabaseInterface::databaseHasChanged);

    myPlayList.setDatabaseInterface(&myDatabaseView);

    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeMovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(rowsMovedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(trackHasBeenAddedSpy.count(), 0);
    QCOMPARE(databaseInterfaceChangedSpy.count(), 1);
    QCOMPARE(persistentStateChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    QCOMPARE(myPlayList.databaseInterface(), &myDatabaseView);

    auto newTracks = QHash<QString, QVector<MusicAudioTrack>>();
    auto newCovers = QHash<QString, QUrl>();

    newTracks[QStringLiteral("album1")] = {
        {true, QStringLiteral("$1"), QStringLiteral("0"), QStringLiteral("track1"),
            QStringLiteral("artist1"), QStringLiteral("album1"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$1"))}},
        {true, QStringLiteral("$2"), QStringLiteral("0"), QStringLiteral("track2"),
            QStringLiteral("artist1"), QStringLiteral("album1"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$2"))}},
        {true, QStringLiteral("$3"), QStringLiteral("0"), QStringLiteral("track3"),
            QStringLiteral("artist1"), QStringLiteral("album1"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$3"))}},
        {true, QStringLiteral("$4"), QStringLiteral("0"), QStringLiteral("track4"),
            QStringLiteral("artist1"), QStringLiteral("album1"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$4"))}},
    };

    newTracks[QStringLiteral("album2")] = {
        {true, QStringLiteral("$5"), QStringLiteral("0"), QStringLiteral("track1"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$5"))}},
        {true, QStringLiteral("$6"), QStringLiteral("0"), QStringLiteral("track2"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$6"))}},
        {true, QStringLiteral("$7"), QStringLiteral("0"), QStringLiteral("track3"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$7"))}},
        {true, QStringLiteral("$8"), QStringLiteral("0"), QStringLiteral("track4"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$8"))}},
        {true, QStringLiteral("$9"), QStringLiteral("0"), QStringLiteral("track5"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$9"))}},
        {true, QStringLiteral("$10"), QStringLiteral("0"), QStringLiteral("track6"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$10"))}}
    };

    newCovers[QStringLiteral("album1")] = QUrl::fromLocalFile(QStringLiteral("album1"));
    newCovers[QStringLiteral("album2")] = QUrl::fromLocalFile(QStringLiteral("album2"));

    myDatabaseContent.insertTracksList(newTracks, newCovers);

    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeMovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(rowsMovedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(trackHasBeenAddedSpy.count(), 0);
    QCOMPARE(databaseInterfaceChangedSpy.count(), 1);
    QCOMPARE(persistentStateChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    auto newTrackID = myDatabaseView.trackIdFromTitleAlbumArtist(QStringLiteral("track6"), QStringLiteral("album2"), QStringLiteral("artist1"));
    myPlayList.enqueue(newTrackID);

    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeMovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(rowsMovedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(trackHasBeenAddedSpy.count(), 1);
    QCOMPARE(databaseInterfaceChangedSpy.count(), 1);
    QCOMPARE(persistentStateChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 0);
}

void MediaPlayListTest::enqueueAlbumCase()
{
    MediaPlayList myPlayList;
    DatabaseInterface myDatabaseContent;
    DatabaseInterface myDatabaseView;

    QSignalSpy rowsAboutToBeMovedSpy(&myPlayList, &MediaPlayList::rowsAboutToBeMoved);
    QSignalSpy rowsAboutToBeRemovedSpy(&myPlayList, &MediaPlayList::rowsAboutToBeRemoved);
    QSignalSpy rowsAboutToBeInsertedSpy(&myPlayList, &MediaPlayList::rowsAboutToBeInserted);
    QSignalSpy rowsMovedSpy(&myPlayList, &MediaPlayList::rowsMoved);
    QSignalSpy rowsRemovedSpy(&myPlayList, &MediaPlayList::rowsRemoved);
    QSignalSpy rowsInsertedSpy(&myPlayList, &MediaPlayList::rowsInserted);
    QSignalSpy trackHasBeenAddedSpy(&myPlayList, &MediaPlayList::trackHasBeenAdded);
    QSignalSpy databaseInterfaceChangedSpy(&myPlayList, &MediaPlayList::databaseInterfaceChanged);
    QSignalSpy persistentStateChangedSpy(&myPlayList, &MediaPlayList::persistentStateChanged);
    QSignalSpy dataChangedSpy(&myPlayList, &MediaPlayList::dataChanged);

    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeMovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(rowsMovedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(trackHasBeenAddedSpy.count(), 0);
    QCOMPARE(databaseInterfaceChangedSpy.count(), 0);
    QCOMPARE(persistentStateChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    myDatabaseContent.init(QStringLiteral("testDbDirectContent"));
    myDatabaseContent.initDatabase();
    myDatabaseContent.initRequest();

    myDatabaseView.init(QStringLiteral("testDbDirectView"));
    myDatabaseView.initDatabase();
    myDatabaseView.initRequest();

    connect(&myDatabaseContent, &DatabaseInterface::databaseChanged,
            &myDatabaseView, &DatabaseInterface::databaseHasChanged);

    myPlayList.setDatabaseInterface(&myDatabaseView);

    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeMovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(rowsMovedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(trackHasBeenAddedSpy.count(), 0);
    QCOMPARE(databaseInterfaceChangedSpy.count(), 1);
    QCOMPARE(persistentStateChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    QCOMPARE(myPlayList.databaseInterface(), &myDatabaseView);

    auto newTracks = QHash<QString, QVector<MusicAudioTrack>>();
    auto newCovers = QHash<QString, QUrl>();

    newTracks[QStringLiteral("album1")] = {
        {true, QStringLiteral("$1"), QStringLiteral("0"), QStringLiteral("track1"),
            QStringLiteral("artist1"), QStringLiteral("album1"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$1"))}},
        {true, QStringLiteral("$2"), QStringLiteral("0"), QStringLiteral("track2"),
            QStringLiteral("artist1"), QStringLiteral("album1"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$2"))}},
        {true, QStringLiteral("$3"), QStringLiteral("0"), QStringLiteral("track3"),
            QStringLiteral("artist1"), QStringLiteral("album1"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$3"))}},
        {true, QStringLiteral("$4"), QStringLiteral("0"), QStringLiteral("track4"),
            QStringLiteral("artist1"), QStringLiteral("album1"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$4"))}},
    };

    newTracks[QStringLiteral("album2")] = {
        {true, QStringLiteral("$5"), QStringLiteral("0"), QStringLiteral("track1"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$5"))}},
        {true, QStringLiteral("$6"), QStringLiteral("0"), QStringLiteral("track2"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$6"))}},
        {true, QStringLiteral("$7"), QStringLiteral("0"), QStringLiteral("track3"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$7"))}},
        {true, QStringLiteral("$8"), QStringLiteral("0"), QStringLiteral("track4"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$8"))}},
        {true, QStringLiteral("$9"), QStringLiteral("0"), QStringLiteral("track5"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$9"))}},
        {true, QStringLiteral("$10"), QStringLiteral("0"), QStringLiteral("track6"),
            QStringLiteral("artist1"), QStringLiteral("album2"), 1, {}, {QUrl::fromLocalFile(QStringLiteral("$10"))}}
    };

    newCovers[QStringLiteral("album1")] = QUrl::fromLocalFile(QStringLiteral("album1"));
    newCovers[QStringLiteral("album2")] = QUrl::fromLocalFile(QStringLiteral("album2"));

    myDatabaseContent.insertTracksList(newTracks, newCovers);

    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeMovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(rowsMovedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(trackHasBeenAddedSpy.count(), 0);
    QCOMPARE(databaseInterfaceChangedSpy.count(), 1);
    QCOMPARE(persistentStateChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    myPlayList.enqueue(QStringLiteral("album2"), QStringLiteral("artist1"));

    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeMovedSpy.count(), 0);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 6);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(rowsMovedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 6);
    QCOMPARE(trackHasBeenAddedSpy.count(), 6);
    QCOMPARE(databaseInterfaceChangedSpy.count(), 1);
    QCOMPARE(persistentStateChangedSpy.count(), 6);
    QCOMPARE(dataChangedSpy.count(), 0);
}

QTEST_MAIN(MediaPlayListTest)


#include "moc_mediaplaylisttest.cpp"