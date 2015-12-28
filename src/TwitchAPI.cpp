#include "TwitchAPI.hpp"

#include <regex>

#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

#include "NetworkAccessManager.hpp"

namespace {
namespace local {

	// Client-ID sent to Twitch in API requests, to avoid rate limiting.
	const QByteArray clientID = "flb24qpkrzc3c0liq64ks42alsidb0u";

	// Send a Twitch API request for the followed channels for a username.
	template <typename QNetworkReplyFinishedSlot>
	void SendUsersFollowedChannelsRequest(const QString & username, const QString & limit,
	      const QString & offset, const QString & direction, const QString & sortby,
	      QNetworkReplyFinishedSlot finishedSlot);

	// Send a Twitch API request for the live streaming subset of a set of channels.
	template <typename QNetworkReplyFinishedSlot>
	void SendLiveChannelsRequest(const QStringList & channels, const QString & limit,
	      const QString & offset, const QString & stream_type,
	      QNetworkReplyFinishedSlot finishedSlot);

	// Handle the completion of SendUsersFollowedChannelsRequest in GetFollowedChannelsForUsername.
	template <typename CompletionCallback>
	void OnFollowedChannelsRequestComplete(QList<QJsonObject> followedChannels,
	      const QString username, CompletionCallback onCompletion, QNetworkReply * reply);

	// Handle the completion of SendLiveChannelsRequest in GetLiveChannels.
	template <typename CompletionCallback>
	void OnLiveChannelsRequestComplete(QList<QJsonObject> liveChannels, QStringList channels,
	      CompletionCallback onCompletion, QNetworkReply * reply);
}
}

void TwitchAPI::GetFollowedChannelsForUsername(
      const QString & username, std::function<void(const QList<QJsonObject> &)> onCompletion)
{
	auto requestCompleteSlot = [username, onCompletion](QNetworkReply * reply) {
		local::OnFollowedChannelsRequestComplete(QList<QJsonObject>{}, username, onCompletion, reply);
	};
	local::SendUsersFollowedChannelsRequest(
	      username, "100", "0", "DESC", "created_at", requestCompleteSlot);
}

void TwitchAPI::GetLiveChannels(
      const QStringList & channels, std::function<void(const QList<QJsonObject> &)> onCompletion)
{
	auto requestCompleteSlot = [channels, onCompletion](QNetworkReply * reply) {
		local::OnLiveChannelsRequestComplete(QList<QJsonObject>{}, channels, onCompletion, reply);
	};
	local::SendLiveChannelsRequest(channels, "100", "0", "live", requestCompleteSlot);
}

void TwitchAPI::GetLiveFollowedChannelsForUsername(
      const QString & username, std::function<void(const QList<QJsonObject> &)> onCompletion)
{
	GetFollowedChannelsForUsername(
	      username, [onCompletion](const QList<QJsonObject> & followedChannels) {
		      QStringList followedChannelNames{};
		      for (const auto & channel : followedChannels) {
			      auto channelName = channel["channel"].toObject()["name"].toString();
			      followedChannelNames.append(channelName);
		      }
		      GetLiveChannels(followedChannelNames, onCompletion);
		   });
}

void TwitchAPI::GetStreamPlaylist(const QString & channel,
      std::function<void(const QHash<QString, QByteArray> &)> onCompletion,
      std::shared_ptr<std::atomic<QNetworkReply *> > currentReply /* = nullptr */)
{
	// Request the access token from Twitch.
	QNetworkRequest request{ QUrl("https://api.twitch.tv/api/channels/"
		   + QUrl::toPercentEncoding(channel) + "/access_token") };
	request.setRawHeader(
	      QByteArray("Referer"), QByteArray("https://api.twitch.tv/crossdomain/receiver.html?v=2"));
	request.setRawHeader(QByteArray("X-Requested-With"), QByteArray("XMLHttpRequest"));

	// Handle the JSON response when it comes.
	auto reply = NetworkAccessManager::Get()->get(request);
	*currentReply = reply;
	QObject::connect(reply, &QNetworkReply::finished, [reply, currentReply, channel,
	                                                        onCompletion]() {

		// Use 'sig' and 'token' from the access_token response to get access to the playlist.
		auto response = QJsonDocument::fromJson(reply->readAll());
		auto responseObject = response.object();

		QUrlQuery query{};
		query.addQueryItem("allow_source", "true");
		query.addQueryItem("p", "");
		query.addQueryItem("player", "twitchweb");
		query.addQueryItem("segment_preference", "4");
		query.addQueryItem("sig", responseObject["sig"].toString());
		query.addQueryItem("token", responseObject["token"].toString());

		QUrl url{ "http://usher.twitch.tv/api/channel/hls/" + QUrl::toPercentEncoding(channel)
			+ ".m3u8" };
		url.setQuery(query);

		// Send the request for a playlist and handle the response when it comes.
		auto reply = NetworkAccessManager::Get()->get(QNetworkRequest(url));
		*currentReply = reply;
		QObject::connect(reply, &QNetworkReply::finished, [reply, currentReply, onCompletion]() {
			*currentReply = nullptr;

			// Parse the M3U8 format just enough to get the quality and its URL.
			auto playlist = reply->readAll();
			auto playlistString = playlist.toStdString();

			std::regex playlistParser{ "#EXT-X-MEDIA:TYPE=VIDEO,GROUP-ID=\"\\w+?\",NAME=\"("
				                        "\\w+?)\",AUTOSELECT=YES,DEFAULT=YES\n"
				                        "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=\\d+?,RESOLUTION=\\d+"
				                        "?x\\d+?,VIDEO=\"\\w+?\"\n"
				                        "(.+?)(?:\n|$)" };

			// Associate the qualities with their URLs in a hash map.
			QHash<QString, QByteArray> results;
			auto parserBegin = std::sregex_iterator(
			      std::cbegin(playlistString), std::cend(playlistString), playlistParser);
			auto parserEnd = std::sregex_iterator();
			for (auto matchIt = parserBegin; matchIt != parserEnd; ++matchIt) {
				const auto & match = *matchIt;
				results[QString::fromStdString(match[1])] = QByteArray::fromStdString(match[2]);
			}

			// Call the callback with the retreived qualities.
			onCompletion(results);
		});

	});
}

template <typename QNetworkReplyFinishedSlot>
void local::SendUsersFollowedChannelsRequest(const QString & username, const QString & limit,
      const QString & offset, const QString & direction, const QString & sortby,
      QNetworkReplyFinishedSlot finishedSlot)
{
	// Construct the API request as per the Twitch API documentation.
	QUrlQuery query{};
	query.addQueryItem("limit", limit);
	query.addQueryItem("offset", offset);
	query.addQueryItem("direction", direction);
	query.addQueryItem("sortby", sortby);

	QUrl url{ "https://api.twitch.tv/kraken/users/" + QUrl::toPercentEncoding(username)
		+ "/follows/channels" };
	url.setQuery(query);

	QNetworkRequest request{ url };
	request.setRawHeader(QByteArray("Accept"), QByteArray("application/vnd.twitchtv.v3+json"));
	request.setRawHeader(QByteArray("Client-ID"), local::clientID);

	// Connect to the slot passed as an argument.
	auto reply = NetworkAccessManager::Get()->get(request);
	QObject::connect(
	      reply, &QNetworkReply::finished, [reply, finishedSlot]() { finishedSlot(reply); });
}

template <typename QNetworkReplyFinishedSlot>
void local::SendLiveChannelsRequest(const QStringList & channels, const QString & limit,
      const QString & offset, const QString & stream_type, QNetworkReplyFinishedSlot finishedSlot)
{
	// Construct the API request as per the Twitch API documentation.
	QUrlQuery query{};
	query.addQueryItem("channel", channels.join(','));
	query.addQueryItem("limit", limit);
	query.addQueryItem("offset", offset);
	query.addQueryItem("stream_type", stream_type);

	QUrl url{ "https://api.twitch.tv/kraken/streams" };
	url.setQuery(query);

	QNetworkRequest request{ url };
	request.setRawHeader(QByteArray("Accept"), QByteArray("application/vnd.twitchtv.v3+json"));
	request.setRawHeader(QByteArray("Client-ID"), local::clientID);

	// Connect to the slot passed as an argument.
	auto reply = NetworkAccessManager::Get()->get(request);
	QObject::connect(
	      reply, &QNetworkReply::finished, [reply, finishedSlot]() { finishedSlot(reply); });
}

template <typename CompletionCallback>
void local::OnFollowedChannelsRequestComplete(QList<QJsonObject> followedChannels,
      const QString username, CompletionCallback onCompletion, QNetworkReply * reply)
{
	// The response is in JSON form.
	auto response = QJsonDocument::fromJson(reply->readAll());
	auto responseObject = response.object();

	// Get the "follows" array which contains all the followed channels.
	auto followedChannelObjects = responseObject["follows"].toArray();

	// Append the channel objects to the list of channels.
	for (const auto & followedChannel : followedChannelObjects) {
		auto channelData = followedChannel.toObject();
		followedChannels.append(channelData);
	}

	// If we have more to go, continue getting more.
	if (responseObject["_total"].toInt() > followedChannels.size()
	      && followedChannelObjects.size() > 0) {
		auto requestCompleteSlot = [followedChannels, onCompletion, username](QNetworkReply * reply) {
			OnFollowedChannelsRequestComplete(followedChannels, username, onCompletion, reply);
		};
		SendUsersFollowedChannelsRequest(username, "100", QString::number(followedChannels.size()),
		      "DESC", "created_at", requestCompleteSlot);
	}
	else {
		// We're done.
		onCompletion(followedChannels);
	}
}

template <typename CompletionCallback>
void local::OnLiveChannelsRequestComplete(QList<QJsonObject> liveChannels, QStringList channels,
      CompletionCallback onCompletion, QNetworkReply * reply)
{
	// The response is in JSON form.
	auto response = QJsonDocument::fromJson(reply->readAll());
	auto responseObject = response.object();

	// Get the "streams" array which contains all the live streams.
	auto streamObjects = responseObject["streams"].toArray();

	// Append the channel objects to the list of live streams.
	for (const auto & stream : streamObjects) {
		auto streamData = stream.toObject();
		liveChannels.append(streamData);
	}

	// If we have more to go, continue getting more.
	if (responseObject["_total"].toInt() > liveChannels.size() && streamObjects.size() > 0) {
		auto requestCompleteSlot = [liveChannels, onCompletion, channels](QNetworkReply * reply) {
			OnLiveChannelsRequestComplete(liveChannels, channels, onCompletion, reply);
		};
		SendLiveChannelsRequest(
		      channels, "100", QString::number(liveChannels.size()), "live", requestCompleteSlot);
	}
	else {
		// We're done.
		onCompletion(liveChannels);
	}
}
