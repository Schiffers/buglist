void ProtocolGame::sendStoreHistory(uint16_t page, uint32_t entriesPerPage)
 {
    // dispatcher thread
    std::vector<StoreTransaction> storeHistory;
    g_store->getTransactionHistory(player->getAccount(), page, entriesPerPage, storeHistory);

    if (storeHistory.size() == 0) {
        return sendStoreError(STORE_ERROR_HISTORY, "No entries yet.");
    }

    NetworkMessage msg;
    msg.addByte(0xFD);

    msg.add<uint16_t>(page);
    msg.addByte((storeHistory.size() > entriesPerPage) ? 0x01 : 0x00);

    uint8_t entries = std::min<uint8_t>(entriesPerPage, static_cast<uint32_t>(storeHistory.size()));
    msg.addByte(entries);
    size_t count = 0;
    for (const auto& entry : storeHistory) {
        if (count++ == entriesPerPage) {
            break;
        }

        msg.add<uint32_t>(entry.timestamp);
        msg.addByte(0x00); // product type
        msg.add<int32_t>(entry.coins);
        msg.addString(entry.description);
    }

    writeToOutputBuffer(msg);
 }
