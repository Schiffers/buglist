void ProtocolLogin::getCharacterList(const std::string& accountName, const std::string& password, const std::string& token, uint16_t version)
{
	Account account;
	if (!IOLoginData::loginserverAuthentication(accountName, password, account)) {
		disconnectClient("Account name or password is not correct.", version);
		return;
	}

	uint32_t ticks = time(nullptr) / AUTHENTICATOR_PERIOD;
	
	auto output = OutputMessagePool::getOutputMessage();
	//Update premium days
	Game::updatePremium(account);

	const std::string& motd = g_config.getString(ConfigManager::MOTD);
	if (!motd.empty()) {
		//Add MOTD
		output->addByte(0x14);

		std::ostringstream ss;
		ss << g_game.getMotdNum() << "\n" << motd;
		output->addString(ss.str());
	}

	//Add session key
	output->addByte(0x28);
	output->addString(accountName + "\n" + password + "\n" + token + "\n" + std::to_string(ticks));

	//Add char list
	output->addByte(0x64);

	output->addByte(1); // number of worlds

	output->addByte(0); // world id
	output->addString(g_config.getString(ConfigManager::SERVER_NAME));
	output->addString(g_config.getString(ConfigManager::IP));
	output->add<uint16_t>(g_config.getNumber(ConfigManager::GAME_PORT));
	output->addByte(0);

	uint8_t size = std::min<size_t>(std::numeric_limits<uint8_t>::max(), account.characters.size());
	output->addByte(size);
	for (uint8_t i = 0; i < size; i++) {
		output->addByte(0);
		output->addString(account.characters[i]);
	}

	//Add premium days
	if (version >= 1082) {
		output->addByte(0);
		output->addByte(!g_config.getBoolean(ConfigManager::FREE_PREMIUM) ?  (account.premiumDays > 0) : 0);
		output->add<uint32_t>(!g_config.getBoolean(ConfigManager::FREE_PREMIUM) ? (time(nullptr) + (account.premiumDays * 86400)) : 0);
	} else {
		output->add<uint16_t>(g_config.getBoolean(ConfigManager::FREE_PREMIUM) ? 0xFFFF : account.premiumDays);
	}

	send(output);

	disconnect();
}
