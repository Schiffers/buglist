int LuaScriptInterface::luaGameCreateItem(lua_State* L)
{
	// Game.createItem(itemId[, count[, position]])
	uint16_t itemId;
	if (isNumber(L, 1)) {
		itemId = getNumber<uint16_t>(L, 1);
	} else {
		itemId = Item::items.getItemIdByName(getString(L, 1));
		if (itemId == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	int32_t count = getNumber<int32_t>(L, 2, 1);
	int32_t itemCount = 1;
	int32_t subType = 1;

	const ItemType& it = Item::items[itemId];
	if (it.hasSubType()) {
		if (it.stackable) {
			itemCount = std::ceil(count / 100.f);
		}

		subType = count;
	} else {
		itemCount = std::max<int32_t>(1, count);
	}

	Position position;
	if (lua_gettop(L) >= 3) {
		position = getPosition(L, 3);
	}

	bool hasTable = itemCount > 1;
	if (hasTable) {
		lua_newtable(L);
	} else if (itemCount == 0) {
		lua_pushnil(L);
		return 1;
	}

	for (int32_t i = 1; i <= itemCount; ++i) {
		int32_t stackCount = subType;
		if (it.stackable) {
			stackCount = std::min<int32_t>(stackCount, 1000);
			subType -= stackCount;
		}

		Item* item = Item::CreateItem(itemId, stackCount);
		if (!item) {
			if (!hasTable) {
				lua_pushnil(L);
			}
			return 1;
		}

		if (position.x != 0) {
			Tile* tile = g_game.map.getTile(position);
			if (!tile) {
				delete item;
				if (!hasTable) {
					lua_pushnil(L);
				}
				return 1;
			}

			ReturnValue ret = g_game.internalAddItem(tile, item, INDEX_WHEREEVER, FLAG_NOLIMIT);
			if (ret != RETURNVALUE_NOERROR) {
				delete item;
				if (!hasTable) {
					lua_pushnil(L);
				}
				return 1;
			}

		} else {
			getScriptEnv()->addTempItem(item);
			item->setParent(VirtualCylinder::virtualCylinder);
		}

		if (hasTable) {
			lua_pushnumber(L, i);
			pushUserdata<Item>(L, item);
			setItemMetatable(L, -1, item);
			lua_settable(L, -3);
		} else {
			pushUserdata<Item>(L, item);
			setItemMetatable(L, -1, item);
		}
	}

	return 1;
}
