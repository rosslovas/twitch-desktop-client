#ifndef NETWORKACCESSMANAGER_HPP
#define NETWORKACCESSMANAGER_HPP

#include <QNetworkAccessManager>

namespace NetworkAccessManager {
QNetworkAccessManager * Get();
void Destroy();
};

#endif // NETWORKACCESSMANAGER_HPP
