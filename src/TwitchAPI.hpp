#ifndef TWITCHAPI_HPP
#define TWITCHAPI_HPP

#include <atomic>
#include <functional>
#include <memory>

#include <QtCore>
#include <QJsonObject>

class QNetworkReply;

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
      std::function<void(const QHash<QString, QByteArray> &)> onCompletion,
      std::shared_ptr<std::atomic<QNetworkReply *> > currentReply = nullptr);
};

#endif // TWITCHAPI_HPP
