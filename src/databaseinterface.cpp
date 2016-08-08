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

#include "databaseinterface.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>

#include <QtCore/QVariant>
#include <QtCore/QDebug>

#include <algorithm>

class DatabaseInterfacePrivate
{
public:

    DatabaseInterfacePrivate(QSqlDatabase tracksDatabase)
        : mTracksDatabase(tracksDatabase), mIndexByPosition(), mPositionByIndex(),
          mAlbumCache(), mSelectAlbumQuery(mTracksDatabase),
          mSelectTrackQuery(mTracksDatabase), mSelectAlbumIdOfTrackQuery(mTracksDatabase),
          mSelectCountAlbumsQuery(mTracksDatabase), mSelectAlbumIdFromTitleQuery(mTracksDatabase),
          mInsertAlbumQuery(mTracksDatabase), mSelectTrackIfFromTitleAlbumArtistQuery(mTracksDatabase),
          mInsertTrackQuery(mTracksDatabase), mSelectAlbumTrackCountQuery(mTracksDatabase),
          mUpdateAlbumQuery(mTracksDatabase), selectAllAlbumIdsQuery(mTracksDatabase)
    {
    }

    QSqlDatabase mTracksDatabase;

    QVector<qlonglong> mIndexByPosition;

    QHash<qlonglong, int> mPositionByIndex;

    QHash<qlonglong, MusicAlbum> mAlbumCache;

    QSqlQuery mSelectAlbumQuery;

    QSqlQuery mSelectTrackQuery;

    QSqlQuery mSelectAlbumIdOfTrackQuery;

    QSqlQuery mSelectCountAlbumsQuery;

    QSqlQuery mSelectAlbumIdFromTitleQuery;

    QSqlQuery mInsertAlbumQuery;

    QSqlQuery mSelectTrackIfFromTitleAlbumArtistQuery;

    QSqlQuery mInsertTrackQuery;

    QSqlQuery mSelectAlbumTrackCountQuery;

    QSqlQuery mUpdateAlbumQuery;

    QSqlQuery selectAllAlbumIdsQuery;

};

DatabaseInterface::DatabaseInterface(QObject *parent) : QObject(parent), d(nullptr)
{
    static int databaseCounter = 1;
    QSqlDatabase tracksDatabase = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("album%1").arg(databaseCounter));
    ++databaseCounter;
    tracksDatabase.setDatabaseName(QStringLiteral("file:memdb1?mode=memory&cache=shared"));
    tracksDatabase.setConnectOptions(QStringLiteral("foreign_keys = ON;QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE"));
    auto result = tracksDatabase.open();
    if (result) {
        qDebug() << "database open";
    } else {
        qDebug() << "database not open";
    }

    d = new DatabaseInterfacePrivate(tracksDatabase);

    initDatabase();
}

DatabaseInterface::~DatabaseInterface()
{
    delete d;
}

QVariant DatabaseInterface::albumDataFromIndex(int albumIndex, DatabaseInterface::AlbumData dataType)
{
    if (albumIndex < 0 || albumIndex >= d->mIndexByPosition.length()) {
        return {};
    }

    auto result = albumDataFromId(d->mIndexByPosition[albumIndex], dataType);

    return result;
}

MusicAlbum DatabaseInterface::albumFromIndex(int albumIndex) const
{
    if (albumIndex < 0 || albumIndex >= d->mIndexByPosition.length()) {
        return {};
    }

    auto result = albumFromId(d->mIndexByPosition[albumIndex]);

    return result;
}

QVariant DatabaseInterface::albumDataFromId(qlonglong albumId, DatabaseInterface::AlbumData dataType) const
{
    d->mSelectAlbumQuery.bindValue(QStringLiteral(":albumId"), albumId);

    auto result = d->mSelectAlbumQuery.exec();

    if (!result || !d->mSelectAlbumQuery.isSelect() || !d->mSelectAlbumQuery.isActive()) {
        qDebug() << "DatabaseInterface::albumFromId" << "not select" << d->mSelectAlbumQuery.lastQuery();
        qDebug() << "DatabaseInterface::albumFromId" << d->mSelectAlbumQuery.lastError();

        return {};
    }

    if (!d->mSelectAlbumQuery.next()) {
        return {};
    }

    auto itAlbum = d->mAlbumCache.find(albumId);

    if (itAlbum == d->mAlbumCache.end()) {
        MusicAlbum retrievedAlbum;

        retrievedAlbum.setDatabaseId(d->mSelectAlbumQuery.record().value(0).toLongLong());
        retrievedAlbum.setTitle(d->mSelectAlbumQuery.record().value(1).toString());
        retrievedAlbum.setId(d->mSelectAlbumQuery.record().value(2).toString());
        retrievedAlbum.setArtist(d->mSelectAlbumQuery.record().value(3).toString());
        retrievedAlbum.setAlbumArtURI(d->mSelectAlbumQuery.record().value(4).toUrl());
        retrievedAlbum.setTracksCount(d->mSelectAlbumQuery.record().value(5).toInt());
        retrievedAlbum.setTracks(fetchTracks(retrievedAlbum.databaseId()));
        retrievedAlbum.setTrackIds(retrievedAlbum.tracksKeys());
        retrievedAlbum.setValid(true);

        d->mAlbumCache[albumId] = retrievedAlbum;
        itAlbum = d->mAlbumCache.find(albumId);
    }

    switch(dataType)
    {
    case DatabaseInterface::AlbumData::Id:
        return itAlbum->id();
    case DatabaseInterface::AlbumData::Image:
        return itAlbum->albumArtURI();
    case DatabaseInterface::AlbumData::Title:
        return itAlbum->title();
    case DatabaseInterface::AlbumData::Artist:
        return itAlbum->artist();
    case DatabaseInterface::AlbumData::TracksCount:
        return itAlbum->tracksCount();
    }

    return {};
}

MusicAlbum DatabaseInterface::albumFromId(qlonglong albumId) const
{
    d->mSelectAlbumQuery.bindValue(QStringLiteral(":albumId"), albumId);

    auto result = d->mSelectAlbumQuery.exec();

    if (!result || !d->mSelectAlbumQuery.isSelect() || !d->mSelectAlbumQuery.isActive()) {
        qDebug() << "DatabaseInterface::albumFromId" << "not select" << d->mSelectAlbumQuery.lastQuery();
        qDebug() << "DatabaseInterface::albumFromId" << d->mSelectAlbumQuery.lastError();

        return {};
    }

    if (!d->mSelectAlbumQuery.next()) {
        return {};
    }

    auto itAlbum = d->mAlbumCache.find(albumId);

    if (itAlbum == d->mAlbumCache.end()) {
        MusicAlbum retrievedAlbum;

        retrievedAlbum.setDatabaseId(d->mSelectAlbumQuery.record().value(0).toLongLong());
        retrievedAlbum.setTitle(d->mSelectAlbumQuery.record().value(1).toString());
        retrievedAlbum.setId(d->mSelectAlbumQuery.record().value(2).toString());
        retrievedAlbum.setArtist(d->mSelectAlbumQuery.record().value(3).toString());
        retrievedAlbum.setAlbumArtURI(d->mSelectAlbumQuery.record().value(4).toUrl());
        retrievedAlbum.setTracksCount(d->mSelectAlbumQuery.record().value(5).toInt());
        retrievedAlbum.setTracks(fetchTracks(retrievedAlbum.databaseId()));
        retrievedAlbum.setTrackIds(retrievedAlbum.tracksKeys());
        retrievedAlbum.setValid(true);

        d->mAlbumCache[albumId] = retrievedAlbum;

        return retrievedAlbum;
    }

    return itAlbum.value();
}

int DatabaseInterface::albumIdFromTrackId(quintptr trackId) const
{
    d->mSelectAlbumIdOfTrackQuery.bindValue(QStringLiteral(":trackId"), trackId);

    auto result = d->mSelectAlbumIdOfTrackQuery.exec();

    if (!result || !d->mSelectAlbumIdOfTrackQuery.isSelect() || !d->mSelectAlbumIdOfTrackQuery.isActive()) {
        qDebug() << "DatabaseInterface::insertAlbumsList" << "not select" << d->mSelectAlbumIdOfTrackQuery.lastQuery();
        qDebug() << "DatabaseInterface::insertAlbumsList" << d->mSelectAlbumIdOfTrackQuery.lastError();

        return -1;
    }

    if (!d->mSelectAlbumIdOfTrackQuery.next()) {
        return -1;
    }

    return d->mSelectAlbumIdOfTrackQuery.record().value(0).toInt();
}

int DatabaseInterface::albumCount() const
{
    d->mSelectCountAlbumsQuery.exec();

    if (!d->mSelectCountAlbumsQuery.isSelect() || !d->mSelectCountAlbumsQuery.isActive()) {
        qDebug() << "DatabaseInterface::albumsList" << "not select" << d->mSelectCountAlbumsQuery.lastQuery();
        qDebug() << "DatabaseInterface::albumsList" << d->mSelectCountAlbumsQuery.lastError();

        return 0;
    }

    if (!d->mSelectCountAlbumsQuery.next()) {
        return 0;
    }

    return d->mSelectCountAlbumsQuery.record().value(0).toInt();
}

int DatabaseInterface::albumPositionByIndex(qlonglong index) const
{
    return d->mPositionByIndex[index];
}

void DatabaseInterface::insertAlbumsList(const QVector<MusicAlbum> &allAlbums)
{
    int albumId = 0;
    for(const auto &album : allAlbums) {
        auto transactionResult = d->mTracksDatabase.transaction();
        if (!transactionResult) {
            qDebug() << "transaction failed";
            return;
        }

        d->mSelectAlbumIdFromTitleQuery.bindValue(QStringLiteral(":title"), album.title());
        d->mSelectAlbumIdFromTitleQuery.bindValue(QStringLiteral(":artist"), album.artist());

        auto result = d->mSelectAlbumIdFromTitleQuery.exec();

        if (!result || !d->mSelectAlbumIdFromTitleQuery.isSelect() || !d->mSelectAlbumIdFromTitleQuery.isActive()) {
            qDebug() << "DatabaseInterface::insertAlbumsList" << "not select" << d->mSelectAlbumIdFromTitleQuery.lastQuery();
            qDebug() << "DatabaseInterface::insertAlbumsList" << d->mSelectAlbumIdFromTitleQuery.lastError();
        }

        if (d->mSelectAlbumIdFromTitleQuery.next()) {
            albumId = std::max(albumId, d->mSelectAlbumIdFromTitleQuery.record().value(0).toInt());
        } else {
            d->mInsertAlbumQuery.bindValue(QStringLiteral(":title"), album.title());
            d->mInsertAlbumQuery.bindValue(QStringLiteral(":artist"), album.artist());
            d->mInsertAlbumQuery.bindValue(QStringLiteral(":coverFileName"), album.albumArtURI());
            d->mInsertAlbumQuery.bindValue(QStringLiteral(":tracksCount"), album.tracksCount());

            result = d->mInsertAlbumQuery.exec();

            if (!result || !d->mInsertAlbumQuery.isActive()) {
                qDebug() << "DatabaseInterface::insertAlbumsList" << "not select" << d->mInsertAlbumQuery.lastQuery();
                qDebug() << "DatabaseInterface::insertAlbumsList" << d->mInsertAlbumQuery.lastError();

                transactionResult = d->mTracksDatabase.commit();
                if (!transactionResult) {
                    qDebug() << "commit failed";
                    return;
                }

                continue;
            }

            result = d->mSelectAlbumIdFromTitleQuery.exec();

            if (!result || !d->mSelectAlbumIdFromTitleQuery.isSelect() || !d->mSelectAlbumIdFromTitleQuery.isActive()) {
                qDebug() << "DatabaseInterface::insertAlbumsList" << "not select" << d->mSelectAlbumIdFromTitleQuery.lastQuery();
                qDebug() << "DatabaseInterface::insertAlbumsList" << d->mSelectAlbumIdFromTitleQuery.lastError();

                transactionResult = d->mTracksDatabase.commit();
                if (!transactionResult) {
                    qDebug() << "commit failed";
                    return;
                }

                continue;
            }

            if (d->mSelectAlbumIdFromTitleQuery.next()) {
                albumId = std::max(albumId, d->mSelectAlbumIdFromTitleQuery.record().value(0).toInt());
            }
        }

        transactionResult = d->mTracksDatabase.commit();
        if (!transactionResult) {
            qDebug() << "commit failed";
            return;
        }
    }

    updateIndexCache();
}

void DatabaseInterface::insertTracksList(QHash<QString, QVector<MusicAudioTrack> > tracks, QHash<QString, QString> covers)
{
    int maximumAlbumId = 0;
    quintptr albumId = 0;
    for (const auto &album : tracks) {
        MusicAlbum newAlbum;

        auto transactionResult = d->mTracksDatabase.transaction();
        if (!transactionResult) {
            qDebug() << "transaction failed";
            return;
        }

        for(const auto &track : album) {
            if (newAlbum.artist().isNull()) {
                newAlbum.setArtist(track.artist());
            }

            if (newAlbum.title().isNull()) {
                newAlbum.setTitle(track.albumName());
            }

            if (newAlbum.albumArtURI().isEmpty()) {
                newAlbum.setAlbumArtURI(QUrl::fromLocalFile(covers[track.albumName()]));
            }

            if (!newAlbum.artist().isNull() && !newAlbum.title().isNull() && !newAlbum.albumArtURI().isEmpty()) {
                break;
            }
        }

        newAlbum.setTracksCount(album.size());

        d->mSelectAlbumIdFromTitleQuery.bindValue(QStringLiteral(":title"), newAlbum.title());
        d->mSelectAlbumIdFromTitleQuery.bindValue(QStringLiteral(":artist"), newAlbum.artist());

        auto result = d->mSelectAlbumIdFromTitleQuery.exec();

        if (!result || !d->mSelectAlbumIdFromTitleQuery.isSelect() || !d->mSelectAlbumIdFromTitleQuery.isActive()) {
            qDebug() << "DatabaseInterface::insertTracksList" << "not select" << d->mSelectAlbumIdFromTitleQuery.lastQuery();
            qDebug() << "DatabaseInterface::insertTracksList" << d->mSelectAlbumIdFromTitleQuery.lastError();
        }

        if (d->mSelectAlbumIdFromTitleQuery.next()) {
            albumId = d->mSelectAlbumIdFromTitleQuery.record().value(0).toInt();
            maximumAlbumId = std::max(maximumAlbumId, d->mSelectAlbumIdFromTitleQuery.record().value(0).toInt());
        } else {
            d->mInsertAlbumQuery.bindValue(QStringLiteral(":title"), newAlbum.title());
            d->mInsertAlbumQuery.bindValue(QStringLiteral(":artist"), newAlbum.artist());
            d->mInsertAlbumQuery.bindValue(QStringLiteral(":coverFileName"), newAlbum.albumArtURI());
            d->mInsertAlbumQuery.bindValue(QStringLiteral(":tracksCount"), newAlbum.tracksCount());

            result = d->mInsertAlbumQuery.exec();

            if (!result) {
                qDebug() << "DatabaseInterface::insertTracksList" << d->mInsertAlbumQuery.lastQuery();
                qDebug() << "DatabaseInterface::insertTracksList" << d->mInsertAlbumQuery.lastError();

                transactionResult = d->mTracksDatabase.commit();
                if (!transactionResult) {
                    qDebug() << "commit failed";
                    return;
                }

                continue;
            }

            result = d->mSelectAlbumIdFromTitleQuery.exec();

            if (!result || !d->mSelectAlbumIdFromTitleQuery.isSelect() || !d->mSelectAlbumIdFromTitleQuery.isActive()) {
                qDebug() << "DatabaseInterface::insertTracksList" << "not select" << d->mSelectAlbumIdFromTitleQuery.lastQuery();
                qDebug() << "DatabaseInterface::insertTracksList" << d->mSelectAlbumIdFromTitleQuery.lastError();

                transactionResult = d->mTracksDatabase.commit();
                if (!transactionResult) {
                    qDebug() << "commit failed";
                    return;
                }

                continue;
            }

            if (d->mSelectAlbumIdFromTitleQuery.next()) {
                albumId = d->mSelectAlbumIdFromTitleQuery.record().value(0).toInt();
                maximumAlbumId = std::max(maximumAlbumId, d->mSelectAlbumIdFromTitleQuery.record().value(0).toInt());
            }
        }

        for(const auto &track : album) {
            //quintptr currentElementId = 0;
            QString artistName = track.artist();

            if (artistName.isEmpty()) {
                artistName = newAlbum.artist();
            }

            d->mSelectTrackIfFromTitleAlbumArtistQuery.bindValue(QStringLiteral(":title"), track.title());
            d->mSelectTrackIfFromTitleAlbumArtistQuery.bindValue(QStringLiteral(":album"), albumId);
            d->mSelectTrackIfFromTitleAlbumArtistQuery.bindValue(QStringLiteral(":artist"), artistName);

            result = d->mSelectTrackIfFromTitleAlbumArtistQuery.exec();

            if (!result || !d->mSelectTrackIfFromTitleAlbumArtistQuery.isSelect() || !d->mSelectTrackIfFromTitleAlbumArtistQuery.isActive()) {
                qDebug() << "DatabaseInterface::insertTracksList" << "not select" << d->mSelectTrackIfFromTitleAlbumArtistQuery.lastQuery();
                qDebug() << "DatabaseInterface::insertTracksList" << d->mSelectTrackIfFromTitleAlbumArtistQuery.lastError();
            }

            if (d->mSelectTrackIfFromTitleAlbumArtistQuery.next()) {
                //currentElementId = d->selectTrackQuery.record().value(0).toInt();
                continue;
            } else {
                d->mInsertTrackQuery.bindValue(QStringLiteral(":title"), track.title());
                d->mInsertTrackQuery.bindValue(QStringLiteral(":album"), albumId);
                d->mInsertTrackQuery.bindValue(QStringLiteral(":artist"), artistName);
                d->mInsertTrackQuery.bindValue(QStringLiteral(":fileName"), track.resourceURI());
                d->mInsertTrackQuery.bindValue(QStringLiteral(":trackNumber"), track.trackNumber());
                d->mInsertTrackQuery.bindValue(QStringLiteral(":trackDuration"), QVariant::fromValue<qlonglong>(track.duration().msecsSinceStartOfDay()));

                result = d->mInsertTrackQuery.exec();

                if (!result || !d->mInsertTrackQuery.isActive()) {
                    qDebug() << "DatabaseInterface::insertTracksList" << d->mInsertTrackQuery.lastQuery();
                    qDebug() << "DatabaseInterface::insertTracksList" << d->mInsertTrackQuery.lastError();
                }

                result = d->mSelectTrackIfFromTitleAlbumArtistQuery.exec();

                if (!result || !d->mSelectTrackIfFromTitleAlbumArtistQuery.isSelect() || !d->mSelectTrackIfFromTitleAlbumArtistQuery.isActive()) {
                    qDebug() << "DatabaseInterface::insertTracksList" << "not select" << d->mSelectTrackIfFromTitleAlbumArtistQuery.lastQuery();
                    qDebug() << "DatabaseInterface::insertTracksList" << d->mSelectTrackIfFromTitleAlbumArtistQuery.lastError();
                }

                if (d->mSelectTrackIfFromTitleAlbumArtistQuery.next()) {
                    //auto currentElementId = selectTrackQuery.record().value(0).toInt();
                    //qDebug() << "DatabaseInterface::insertTracksList" << "insert track" << track.mTitle << artistName << track.mAlbumName << currentElementId << albumId;
                }
            }
        }

        transactionResult = d->mTracksDatabase.commit();
        if (!transactionResult) {
            qDebug() << "commit failed";
            return;
        }
    }

    updateIndexCache();
}

void DatabaseInterface::initDatabase() const
{
    if (!d->mTracksDatabase.tables().contains(QStringLiteral("DiscoverSource"))) {
        QSqlQuery createSchemaQuery(d->mTracksDatabase);

        const auto &result = createSchemaQuery.exec(QStringLiteral("CREATE TABLE `DiscoverSource` (`ID` INTEGER PRIMARY KEY NOT NULL, "
                                              "`UUID` TEXT NOT NULL, "
                                              "UNIQUE (`UUID`))"));

        if (!result) {
            qDebug() << "AbstractAlbumModel::initDatabase" << createSchemaQuery.lastError();
        }
    }

    if (!d->mTracksDatabase.tables().contains(QStringLiteral("Albums"))) {
        QSqlQuery createSchemaQuery(d->mTracksDatabase);

        const auto &result = createSchemaQuery.exec(QStringLiteral("CREATE TABLE `Albums` (`ID` INTEGER PRIMARY KEY NOT NULL, "
                                                                   "`Title` TEXT NOT NULL, "
                                                                   "`Artist` TEXT NOT NULL, "
                                                                   "`CoverFileName` TEXT NOT NULL, "
                                                                   "`TracksCount` INTEGER NOT NULL, "
                                                                   "`AlbumInternalID` TEXT, "
                                                                   "UNIQUE (`Title`, `Artist`))"));

        if (!result) {
            qDebug() << "AbstractAlbumModel::initDatabase" << createSchemaQuery.lastError();
        }
    }

    if (!d->mTracksDatabase.tables().contains(QStringLiteral("Tracks"))) {
        QSqlQuery createSchemaQuery(d->mTracksDatabase);

        const auto &result = createSchemaQuery.exec(QStringLiteral("CREATE TABLE `Tracks` (`ID` INTEGER PRIMARY KEY NOT NULL, "
                                                                   "`Title` TEXT NOT NULL, "
                                                                   "`AlbumID` INTEGER NOT NULL, "
                                                                   "`Artist` TEXT NOT NULL, "
                                                                   "`FileName` TEXT NOT NULL UNIQUE, "
                                                                   "`TrackNumber` INTEGER NOT NULL, "
                                                                   "`Duration` INTEGER NOT NULL, "
                                                                   "UNIQUE (`Title`, `AlbumID`, `Artist`), "
                                                                   "CONSTRAINT fk_album FOREIGN KEY (`AlbumID`) REFERENCES `Albums`(`ID`))"));

        if (!result) {
            qDebug() << "AbstractAlbumModel::initDatabase" << createSchemaQuery.lastError();
        }
    }

    if (!d->mTracksDatabase.tables().contains(QStringLiteral("TracksMapping"))) {
        QSqlQuery createSchemaQuery(d->mTracksDatabase);

        const auto &result = createSchemaQuery.exec(QStringLiteral("CREATE TABLE `TracksMapping` ("
                                                                   "`TrackID` INTEGER NOT NULL, "
                                                                   "`DiscoverID` INTEGER NOT NULL, "
                                                                   "PRIMARY KEY (`TrackID`, `DiscoverID`), "
                                                                   "CONSTRAINT fk_trackID FOREIGN KEY (`TrackID`) REFERENCES `Tracks`(`ID`), "
                                                                   "CONSTRAINT fk_discoverID FOREIGN KEY (`DiscoverID`) REFERENCES `DiscoverSource`(`ID`))"));

        if (!result) {
            qDebug() << "AbstractAlbumModel::initDatabase" << createSchemaQuery.lastError();
        }
    }

    {
        QSqlQuery createTrackIndex(d->mTracksDatabase);

        const auto &result = createTrackIndex.exec(QStringLiteral("CREATE INDEX "
                                                                  "IF NOT EXISTS "
                                                                  "`TracksAlbumIndex` ON `Tracks` "
                                                                  "(`AlbumID`)"));

        if (!result) {
            qDebug() << "AbstractAlbumModel::initDatabase" << createTrackIndex.lastError();
        }
    }

    {
        auto selectAlbumQueryText = QStringLiteral("SELECT `ID`, "
                                                   "`Title`, "
                                                   "`AlbumInternalID`, "
                                                   "`Artist`, "
                                                   "`CoverFileName`, "
                                                   "`TracksCount` "
                                                   "FROM `Albums`"
                                                   "WHERE "
                                                   "`ID` = :albumId");

        auto result = d->mSelectAlbumQuery.prepare(selectAlbumQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::albumFromId" << d->mSelectAlbumQuery.lastError();
        }
    }

    {
        auto selectTrackQueryText = QStringLiteral("SELECT `ID`, "
                                                   "`Title`, "
                                                   "`AlbumID`, "
                                                   "`Artist`, "
                                                   "`FileName`, "
                                                   "`TrackNumber`, "
                                                   "`Duration` "
                                                   "FROM `Tracks` "
                                                   "WHERE "
                                                   "`AlbumID` = :albumId");

        auto result = d->mSelectTrackQuery.prepare(selectTrackQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::fetchTracks" << d->mSelectTrackQuery.lastError();
        }
    }
    {
        auto selectAlbumIdOfTrackQueryText = QStringLiteral("SELECT `AlbumID` FROM `Tracks` "
                                              "WHERE "
                                              "`ID` = :trackId");

        auto result = d->mSelectAlbumIdOfTrackQuery.prepare(selectAlbumIdOfTrackQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::albumIdFromTrackId" << d->mSelectAlbumIdOfTrackQuery.lastError();
        }
    }
    {
        auto selectCountAlbumsQueryText = QStringLiteral("SELECT count(*) "
                                                   "FROM Albums");

        const auto result = d->mSelectCountAlbumsQuery.prepare(selectCountAlbumsQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::albumCount" << d->mSelectCountAlbumsQuery.lastError();
        }
    }
    {
        auto selectAlbumIdFromTitleQueryText = QStringLiteral("SELECT ID FROM `Albums` "
                                              "WHERE "
                                              "`Title` = :title");

        auto result = d->mSelectAlbumIdFromTitleQuery.prepare(selectAlbumIdFromTitleQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::insertAlbumsList" << d->mSelectAlbumIdFromTitleQuery.lastError();
        }
    }
    {
        auto insertAlbumQueryText = QStringLiteral("INSERT INTO Albums (`Title`, `Artist`, `CoverFileName`, `TracksCount`)"
                                              "VALUES (:title, :artist, :coverFileName, :tracksCount)");

        auto result = d->mInsertAlbumQuery.prepare(insertAlbumQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::insertAlbumsList" << d->mInsertAlbumQuery.lastError();
        }
    }
    {
        auto selectTrackQueryText = QStringLiteral("SELECT ID FROM `Tracks` "
                                              "WHERE "
                                              "`Title` = :title AND "
                                              "`AlbumID` = :album AND "
                                              "`Artist` = :artist");

        auto result = d->mSelectTrackIfFromTitleAlbumArtistQuery.prepare(selectTrackQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::insertTracksList" << d->mSelectTrackIfFromTitleAlbumArtistQuery.lastError();
        }

        auto insertTrackQueryText = QStringLiteral("INSERT INTO `Tracks` (`Title`, `AlbumID`, `Artist`, `FileName`, `TrackNumber`, `Duration`)"
                                              "VALUES (:title, :album, :artist, :fileName, :trackNumber, :trackDuration)");

        result = d->mInsertTrackQuery.prepare(insertTrackQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::insertTracksList" << d->mInsertTrackQuery.lastError();
        }
    }
    {
        auto selectAlbumTrackCountQueryText = QStringLiteral("SELECT `TracksCount` "
                                                   "FROM `Albums`"
                                                   "WHERE "
                                                   "`ID` = :albumId");

        auto result = d->mSelectAlbumTrackCountQuery.prepare(selectAlbumTrackCountQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::albumFromId" << d->mSelectAlbumTrackCountQuery.lastError();
        }
    }
    {
        auto updateAlbumQueryText = QStringLiteral("UPDATE `Albums` "
                                                   "SET `TracksCount`=:tracksCount "
                                                   "WHERE "
                                                   "`ID` = :albumId");

        auto result = d->mUpdateAlbumQuery.prepare(updateAlbumQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::albumFromId" << d->mUpdateAlbumQuery.lastError();
        }
    }
    {
        auto selectAlbumQueryText = QStringLiteral("SELECT `ID` "
                                                   "FROM `Albums`");

        auto result = d->selectAllAlbumIdsQuery.prepare(selectAlbumQueryText);

        if (!result) {
            qDebug() << "DatabaseInterface::albumFromIndex" << d->selectAllAlbumIdsQuery.lastError();

            return;
        }
    }
}

QMap<qlonglong, MusicAudioTrack> DatabaseInterface::fetchTracks(qlonglong albumId) const
{
    QMap<qlonglong, MusicAudioTrack> allTracks;

    d->mSelectTrackQuery.bindValue(QStringLiteral(":albumId"), albumId);

    auto result = d->mSelectTrackQuery.exec();

    if (!result || !d->mSelectTrackQuery.isSelect() || !d->mSelectTrackQuery.isActive()) {
        qDebug() << "DatabaseInterface::fetchTracks" << "not select" << d->mSelectTrackQuery.lastQuery();
        qDebug() << "DatabaseInterface::fetchTracks" << d->mSelectTrackQuery.lastError();
    }

    while (d->mSelectTrackQuery.next()) {
        MusicAudioTrack newTrack;

        newTrack.setDatabaseId(d->mSelectTrackQuery.record().value(0).toLongLong());
        newTrack.setTitle(d->mSelectTrackQuery.record().value(1).toString());
        newTrack.setParentId(d->mSelectTrackQuery.record().value(2).toString());
        newTrack.setArtist(d->mSelectTrackQuery.record().value(3).toString());
        newTrack.setResourceURI(d->mSelectTrackQuery.record().value(4).toUrl());
        newTrack.setTrackNumber(d->mSelectTrackQuery.record().value(5).toInt());
        newTrack.setDuration(QTime::fromMSecsSinceStartOfDay(d->mSelectTrackQuery.record().value(6).toInt()));
        newTrack.setValid(true);

        allTracks[newTrack.databaseId()] = newTrack;
    }

    updateTracksCount(albumId, allTracks.size());

    return allTracks;
}

void DatabaseInterface::updateTracksCount(qlonglong albumId, int tracksCount) const
{
    d->mSelectAlbumTrackCountQuery.bindValue(QStringLiteral(":albumId"), albumId);

    auto result = d->mSelectAlbumTrackCountQuery.exec();

    if (!result || !d->mSelectAlbumTrackCountQuery.isSelect() || !d->mSelectAlbumTrackCountQuery.isActive()) {
        qDebug() << "DatabaseInterface::albumFromId" << "not select" << d->mSelectAlbumTrackCountQuery.lastQuery();
        qDebug() << "DatabaseInterface::albumFromId" << d->mSelectAlbumTrackCountQuery.lastError();

        return;
    }

    if (!d->mSelectAlbumTrackCountQuery.next()) {
        return;
    }

    auto oldTracksCount = d->mSelectAlbumTrackCountQuery.record().value(0).toInt();

    if (oldTracksCount != tracksCount) {
        d->mUpdateAlbumQuery.bindValue(QStringLiteral(":tracksCount"), tracksCount);
        d->mUpdateAlbumQuery.bindValue(QStringLiteral(":albumId"), albumId);

        result = d->mUpdateAlbumQuery.exec();

        if (!result || !d->mUpdateAlbumQuery.isActive()) {
            qDebug() << "DatabaseInterface::albumFromId" << "not select" << d->mUpdateAlbumQuery.lastQuery();
            qDebug() << "DatabaseInterface::albumFromId" << d->mUpdateAlbumQuery.lastError();

            return;
        }
    }
}

void DatabaseInterface::updateIndexCache()
{
    initDatabase();

    MusicAlbum retrievedAlbum;

    auto result = d->selectAllAlbumIdsQuery.exec();

    if (!result || !d->selectAllAlbumIdsQuery.isSelect() || !d->selectAllAlbumIdsQuery.isActive()) {
        qDebug() << "DatabaseInterface::albumFromIndex" << "not select" << d->selectAllAlbumIdsQuery.lastQuery();
        qDebug() << "DatabaseInterface::albumFromIndex" << d->selectAllAlbumIdsQuery.lastError();

        return;
    }

    d->mIndexByPosition.clear();
    d->mPositionByIndex.clear();
    d->mAlbumCache.clear();

    while(d->selectAllAlbumIdsQuery.next()) {
        d->mPositionByIndex[d->selectAllAlbumIdsQuery.record().value(0).toLongLong()] = d->mIndexByPosition.length();
        d->mIndexByPosition.push_back(d->selectAllAlbumIdsQuery.record().value(0).toLongLong());
    }

    Q_EMIT resetModel();
}


#include "moc_databaseinterface.cpp"