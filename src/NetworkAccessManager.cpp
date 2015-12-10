#include "NetworkAccessManager.hpp"

#include <memory>

namespace {
namespace local {
	std::unique_ptr<QNetworkAccessManager> networkAccessManager{ nullptr };
}
}

QNetworkAccessManager * NetworkAccessManager::Get()
{
	if (local::networkAccessManager == nullptr) {
		local::networkAccessManager = std::move(std::make_unique<QNetworkAccessManager>());
	}
	return local::networkAccessManager.get();
}

void NetworkAccessManager::Destroy()
{
	local::networkAccessManager.reset();
}
