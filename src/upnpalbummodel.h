/*
 * Copyright 2015-2016 Matthieu Gallien <matthieu_gallien@yahoo.fr>
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

#ifndef UPNPALBUMMODEL_H
#define UPNPALBUMMODEL_H

#include <QtCore/QAbstractItemModel>

#include "musicalbum.h"
#include "musicaudiotrack.h"

class UpnpAlbumModelPrivate;
class UpnpSsdpEngine;
class UpnpControlAbstractDevice;
class UpnpControlContentDirectory;
struct UpnpDiscoveryResult;
class QDomNode;
class MusicStatistics;
class DidlParser;

class UpnpAlbumModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(UpnpControlContentDirectory* contentDirectory
               READ contentDirectory
               WRITE setContentDirectory
               NOTIFY contentDirectoryChanged)

    Q_PROPERTY(MusicStatistics* musicDatabase
               READ musicDatabase
               WRITE setMusicDatabase
               NOTIFY musicDatabaseChanged)

    Q_PROPERTY(DidlParser* didlParser
               READ didlParser
               NOTIFY didlParserChanged)

public:

    enum ItemClass {
        Container = 0,
        Album = 1,
        Artist = 2,
        AudioTrack = 3,
    };

    enum ColumnsRoles {
        TitleRole = Qt::UserRole + 1,
        DurationRole = TitleRole + 1,
        CreatorRole = DurationRole + 1,
        ArtistRole = CreatorRole + 1,
        AlbumRole = ArtistRole + 1,
        RatingRole = AlbumRole + 1,
        ImageRole = RatingRole + 1,
        ResourceRole = ImageRole + 1,
        ItemClassRole = ResourceRole + 1,
        CountRole = ItemClassRole + 1,
        IdRole = CountRole + 1,
        ParentIdRole = IdRole + 1,
        IsPlayingRole = ParentIdRole + 1,
    };

    explicit UpnpAlbumModel(QObject *parent = 0);

    virtual ~UpnpAlbumModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

    bool canFetchMore(const QModelIndex &parent) const override;

    void fetchMore(const QModelIndex &parent) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE QString objectIdByIndex(const QModelIndex &index) const;

    Q_INVOKABLE QVariant getUrl(const QModelIndex &index) const;

    Q_INVOKABLE QModelIndex indexFromId(const QString &id) const;

    UpnpControlContentDirectory* contentDirectory() const;

    MusicStatistics* musicDatabase() const;

    DidlParser* didlParser() const;

Q_SIGNALS:

    void contentDirectoryChanged();

    void musicDatabaseChanged();

    void didlParserChanged();

    void newAlbum(const MusicAlbum &album);

    void newAudioTrack(const MusicAudioTrack &audioTrack);

public Q_SLOTS:

    void setContentDirectory(UpnpControlContentDirectory *directory);

    void setMusicDatabase(MusicStatistics* musicDatabase);

private Q_SLOTS:

private:

    QModelIndex indexFromInternalId(quintptr internalId) const;

    UpnpAlbumModelPrivate *d;
};



Q_DECLARE_METATYPE(UpnpAlbumModel::ItemClass)

#endif // UPNPALBUMMODEL_H