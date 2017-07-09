void ProtocolLogin::addWorldInfo(OutputMessage_ptr& output, const std::string& accountName, const std::string& password, uint16_t version, bool isLiveCastLogin /*=false*/)
{
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
	output->addString(accountName + "\n" + password);

	//Add char list
	output->addByte(0x64);

	output->addByte(1); // number of worlds

	output->addByte(0); // world id
	output->addString(g_config.getString(ConfigManager::SERVER_NAME));
	output->addString(g_config.getString(ConfigManager::IP));

	if (isLiveCastLogin) {
		output->add<uint16_t>(g_config.getNumber(ConfigManager::LIVE_CAST_PORT));
	} else {
		output->add<uint16_t>(g_config.getNumber(ConfigManager::GAME_PORT));
	}
	output->addByte(0);
}
