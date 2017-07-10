void Connection::parseHeader(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	readTimer.cancel();

	if (error) {
		close(FORCE_CLOSE);
		return;
	} else if (connectionState == CONNECTION_STATE_DISCONNECTED) {
		return;
	}

	uint32_t timePassed = std::max<uint32_t>(1, (time(nullptr) - timeConnected) + 1);
	if ((++packetsSent / timePassed) > static_cast<uint32_t>(g_config.getNumber(ConfigManager::MAX_PACKETS_PER_SECOND))) {
		std::cout << convertIPToString(getIP()) << " disconnected for exceeding packet per second limit." << std::endl;
		close();
		return;
	}

	if (!receivedLastChar && connectionState == CONNECTION_STATE_CONNECTING_STAGE2) {
		uint8_t* msgBuffer = msg.getBuffer();

		if (!receivedName && msgBuffer[1] == 0x00) {
			receivedLastChar = true;
		} else {
			std::string serverName = g_config.getString(ConfigManager::SERVER_NAME) + "\n";

			if (!receivedName) {
				if ((char)msgBuffer[0] == serverName[0] && (char)msgBuffer[1] == serverName[1]) {
					receivedName = true;
					serverNameTime = 1;

					accept();
					return;
				} else {
					std::cout << "[Network error - Connection::parseHeader] Invalid Client Login" << std::endl;
					close(FORCE_CLOSE);
					return;
				}
			}
			++serverNameTime;

			if ((char)msgBuffer[0] == serverName[serverNameTime]) {
				if (msgBuffer[0] == 0x0A) {
					receivedLastChar = true;
				}

				accept();
				return;
			} else {
				std::cout << "[Network error - Connection::parseHeader] Invalid Client Login" << std::endl;
				close(FORCE_CLOSE);
				return;
			}
		}
	}

	if (receivedLastChar && connectionState == CONNECTION_STATE_CONNECTING_STAGE2) {
		connectionState = CONNECTION_STATE_GAME;
	}

	if (timePassed > 2) {
		timeConnected = time(nullptr);
		packetsSent = 0;
	}

	uint16_t size = msg.getLengthHeader();
	if (size == 0 || size >= NETWORKMESSAGE_MAXSIZE - 16) {
		close(FORCE_CLOSE);
		return;
	}

	try {
		readTimer.expires_from_now(boost::posix_time::seconds(Connection::read_timeout));
		readTimer.async_wait(std::bind(&Connection::handleTimeout, std::weak_ptr<Connection>(shared_from_this()),
			std::placeholders::_1));

		// Read packet content
		msg.setLength(size + NetworkMessage::HEADER_LENGTH);
		boost::asio::async_read(socket, boost::asio::buffer(msg.getBodyBuffer(), size),
			std::bind(&Connection::parsePacket, shared_from_this(), std::placeholders::_1));
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::parseHeader] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}
