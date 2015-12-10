#ifndef TWITCHAPI_HPP
#define TWITCHAPI_HPP

#include <functional>

#include <QtCore>
#include <QJsonObject>

namespace TwitchAPI {

// Get all of the channels a user follows.
void GetFollowedChannelsForUsername(
      const QString & username, std::function<void(const QList<QJsonObject> &)> onCompletion);

// Get subset of channels that is currently live.
void GetLiveChannels(
      const QStringList & channels, std::function<void(const QList<QJsonObject> &)> onCompletion);

// Combine the above two methods.
void GetLiveFollowedChannelsForUsername(
      const QString & username, std::function<void(const QList<QJsonObject> &)> onCompletion);

// Get a stream playlist.
void GetStreamPlaylist(const QString & channel,
      std::function<void(const QHash<QString, QByteArray> &)> onCompletion);
};

#endif // TWITCHAPI_HPP
