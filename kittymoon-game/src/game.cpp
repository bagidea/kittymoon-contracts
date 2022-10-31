#include <game.hpp>

ACTION game::setconfig(
   name     CORE_TOKEN_ACCOUNT,
   symbol   CORE_TOKEN_SYMBOL,
   name     ASSETS_COLLECTION_NAME,
   name     CORE_SHOP_ACCOUNT,
   name     ASSETS_SCHEMA_SEEDS,
   name     ASSETS_SCHEMA_TOOLS,
   name     ASSETS_SCHEMA_LANDS,
   name     ASSETS_SCHEMA_HOUSE
) {
   require_auth(get_self());

   config.set(
      {
         .CORE_TOKEN_ACCOUNT      = CORE_TOKEN_ACCOUNT,
         .CORE_TOKEN_SYMBOL       = CORE_TOKEN_SYMBOL,
         .ASSETS_COLLECTION_NAME  = ASSETS_COLLECTION_NAME,
         .CORE_SHOP_ACCOUNT       = CORE_SHOP_ACCOUNT,
         .ASSETS_SCHEMA_SEEDS     = ASSETS_SCHEMA_SEEDS,
         .ASSETS_SCHEMA_TOOLS     = ASSETS_SCHEMA_TOOLS,
         .ASSETS_SCHEMA_LANDS     = ASSETS_SCHEMA_LANDS,
         .ASSETS_SCHEMA_HOUSE     = ASSETS_SCHEMA_HOUSE
      },
      get_self()
   );
}

ACTION game::setgameconfig(
   uint32_t    first_energy,
   uint8_t     land_limit,
   asset       reward_common,
   asset       reward_uncommon,
   asset       reward_rare,
   asset       reward_legend,
   uint32_t    cooldown_growing,
   uint32_t    penalised_rare_legendary,
   uint32_t    penalised_uncommon_legendary,
   uint32_t    penalised_uncommon_rare,
   uint32_t    penalised_common_legendary,
   uint32_t    penalised_common_rare,
   uint32_t    penalised_common_uncommon
) {
   require_auth(get_self());

   gameconfig.set(
      {
         .first_energy                 = first_energy,
         .land_limit                   = land_limit,
         .reward_common                = reward_common,
         .reward_uncommon              = reward_uncommon,
         .reward_rare                  = reward_rare,
         .reward_legend                = reward_legend,
         .cooldown_growing             = cooldown_growing,
         .penalised_rare_legendary     = penalised_rare_legendary,
         .penalised_uncommon_legendary = penalised_uncommon_legendary,
         .penalised_uncommon_rare      = penalised_uncommon_rare,
         .penalised_common_legendary   = penalised_common_legendary,
         .penalised_common_rare        = penalised_common_rare,
         .penalised_common_uncommon    = penalised_common_uncommon
      },
      get_self()
   );
}

ACTION game::setnftconfig(
   int32_t     reward_common_template_id,
   int32_t     reward_uncommon_template_id,
   int32_t     reward_rare_template_id,
   int32_t     reward_legend_template_id,
   int32_t     seed_pack_template_id,
   int32_t     tool_pack_template_id
) {
   require_auth(get_self());

   nfttemplates.set(
      {
         .reward_common_template_id    = reward_common_template_id,
         .reward_uncommon_template_id  = reward_uncommon_template_id,
         .reward_rare_template_id      = reward_rare_template_id,
         .reward_legend_template_id    = reward_legend_template_id,
         .seed_pack_template_id        = seed_pack_template_id,
         .tool_pack_template_id        = tool_pack_template_id
      },
      get_self()
   );
}

ACTION game::signup(
   name     player_account,
   string   player_name
) {
   require_auth(player_account);

   check(player_name.length() <= 22, "invalid name, 12 character long");

   auto it_player = players.find(player_account.value);
   check(it_player == players.end(), "this account is exist");

   players.emplace(
      get_self(),
      [&](auto& s) {
         s.player_account  = player_account;
         s.player_name     = player_name;
         s.energy          = gameconfig.get().first_energy;
         s.max_energy      = gameconfig.get().first_energy;
         s.tools_per_type  = 2;
      }
   );

   seeds.emplace(
      player_account,
      [&](auto& s) {
         s.player_account = player_account;
         s.common         = 0;
         s.uncommon       = 0;
         s.rare           = 0;
         s.legend         = 0;
      }
   );

   rewards.emplace(
      player_account,
      [&](auto& s) {
         s.player_account = player_account;
         s.common         = 0;
         s.uncommon       = 0;
         s.rare           = 0;
         s.legend         = 0;
      }
   );

   asset new_asset = asset(0, config.get().CORE_TOKEN_SYMBOL);

   bonusrewards.emplace(
      player_account,
      [&](auto& s) {
         s.player_account = player_account;
         s.common         = new_asset;
         s.uncommon       = new_asset;
         s.rare           = new_asset;
         s.legend         = new_asset;
      }
   );

   penaliseds.emplace(
      player_account,
      [&](auto& s) {
         s.player_account = player_account;
         s.common         = new_asset;
         s.uncommon       = new_asset;
         s.rare           = new_asset;
         s.legend         = new_asset;
      }
   );

   tools.emplace(
      get_self(),
      [&](auto& s) {
         s.player_account = player_account;
      }
   );

   randnumbers.emplace(
      get_self(),
      [&](auto& s) {
         s.player_account = player_account;
         s.min_number     = 0;
         s.max_number     = 0;
         s.status         = "";
         s.value          = "";
      }
   );

   HOUSE house;
   house.asset_id             = 0;
   house.rarity               = "";
   house.holding_tools        = "";
   house.cooldown_hr          = 0;
   house.energy               = 0;
   house.energy_using         = 0;
   house.coolingdown_bonus    = "";
   house.minting_bonus        = "";
   house.current_time         = 0;

   lands.emplace(
      get_self(),
      [&](auto& s) {
         s.player_account = player_account;

         LAND land_default;
         land_default.asset_id         = 0;
         land_default.slot_index       = 0;
         land_default.rarity           = "default";
         land_default.cooldown_hr      = 0;
         land_default.energy           = 0;
         land_default.energy_using     = 0;
         land_default.blocks_count     = 4;
         land_default.house            = house;
         land_default.bonus            = asset(0, config.get().CORE_TOKEN_SYMBOL);
         land_default.mining_bonus     = "";
         land_default.minting_bonus    = "";
         land_default.current_time     = 0;

         for(uint8_t i = 0; i < 4; i++) {
            BLOCK new_block;
            new_block.status        = "ready";
            new_block.rarity        = "";
            new_block.cooldown_hr   = 0;
            new_block.current_time  = 0;

            land_default.blocks.push_back(new_block);
         }

         s.lands.push_back(land_default);
      }
   );
}

ACTION game::repairplayer(
   name     player_account
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found account");

   bool has_update = false;

   auto it_seed = seeds.find(player_account.value);

   if(it_seed == seeds.end()) {
      seeds.emplace(
         player_account,
         [&](auto& s) {
            s.player_account = player_account;
            s.common         = 0;
            s.uncommon       = 0;
            s.rare           = 0;
            s.legend         = 0;
         }
      );

      has_update = true;
   }

   auto it_reward = rewards.find(player_account.value);

   if(it_reward == rewards.end()) {
      rewards.emplace(
         player_account,
         [&](auto& s) {
            s.player_account = player_account;
            s.common         = 0;
            s.uncommon       = 0;
            s.rare           = 0;
            s.legend         = 0;
         }
      );

      has_update = true;
   }

   asset new_asset = asset(0, config.get().CORE_TOKEN_SYMBOL);

   auto it_bonusreward = bonusrewards.find(player_account.value);

   if(it_bonusreward == bonusrewards.end()) {
      bonusrewards.emplace(
         player_account,
         [&](auto& s) {
            s.player_account = player_account;
            s.common         = new_asset;
            s.uncommon       = new_asset;
            s.rare           = new_asset;
            s.legend         = new_asset;
         }
      );
   }

   auto it_penalised = penaliseds.find(player_account.value);

   if(it_penalised == penaliseds.end()) {
      penaliseds.emplace(
         player_account,
         [&](auto& s) {
            s.player_account = player_account;
            s.common         = new_asset;
            s.uncommon       = new_asset;
            s.rare           = new_asset;
            s.legend         = new_asset;
         }
      );

      has_update = true;
   }

   auto it_tool = tools.find(player_account.value);

   if(it_tool == tools.end()) {
      tools.emplace(
         get_self(),
         [&](auto& s) {
            s.player_account = player_account;
         }
      );

      has_update = true;
   }

   auto it_randnumber = randnumbers.find(player_account.value);

   if(it_randnumber == randnumbers.end()) {
      randnumbers.emplace(
         get_self(),
         [&](auto& s) {
            s.player_account = player_account;
            s.min_number     = 0;
            s.max_number     = 0;
            s.status         = "";
            s.value          = "";
         }
      );
   }

   auto it_land = lands.find(player_account.value);

   if(it_land == lands.end()) {
      HOUSE house;
      house.asset_id             = 0;
      house.rarity               = "";
      house.holding_tools        = "";
      house.cooldown_hr          = 0;
      house.energy               = 0;
      house.energy_using         = 0;
      house.coolingdown_bonus    = "";
      house.minting_bonus        = "";
      house.current_time         = 0;

      lands.emplace(
         get_self(),
         [&](auto& s) {
            s.player_account = player_account;

            LAND land_default;
            land_default.asset_id         = 0;
            land_default.slot_index       = 0;
            land_default.rarity           = "default";
            land_default.cooldown_hr      = 0;
            land_default.energy           = 0;
            land_default.energy_using     = 0;
            land_default.blocks_count     = 4;
            land_default.house            = house;
            land_default.bonus            = asset(0, config.get().CORE_TOKEN_SYMBOL);
            land_default.mining_bonus     = "";
            land_default.minting_bonus    = "";
            land_default.current_time     = 0;

            for(uint8_t i = 0; i < 4; i++) {
               BLOCK new_block;
               new_block.status        = "ready";
               new_block.rarity        = "";
               new_block.cooldown_hr   = 0;
               new_block.current_time  = 0;

               land_default.blocks.push_back(new_block);
            }

            s.lands.push_back(land_default);
         }
      );

      has_update = true;
   }

   check(has_update, "no update because this account is the latest version");
}

ACTION game::modifyname(
   name     player_account,
   string   player_name
) {
   require_auth(player_account);

   check(player_name.length() <= 22, "invalid name, 12 character long");

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found this account");

   players.modify(
      it_player,
      get_self(),
      [&](auto& s) {
         s.player_name = player_name;
      }
   );
}

ACTION game::addenergy(
   name        authorized_account,
   name        player_account,
   uint32_t    energy
) {
   require_auth(authorized_account);

   check(authorized_account == config.get().CORE_SHOP_ACCOUNT, "shop account not has permission");

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found this account");
   check(it_player->energy + energy <= it_player->max_energy, "overflow energy");

   check(energy > 0, "incorrect energy amount");

   players.modify(
      it_player,
      get_self(),
      [&](auto& s) {
         s.energy += energy;
      }
   );
}

ACTION game::addenergyt(
   name        authorized_account,
   name        player_account,
   uint64_t    asset_id,
   uint32_t    energy
) {
   require_auth(authorized_account);

   check(authorized_account == config.get().CORE_SHOP_ACCOUNT, "shop account not has permission");

   atomicassets::assets_t listAssets = atomicassets::get_assets(get_self());
   auto idxCard = listAssets.find(asset_id);

   check(idxCard != listAssets.end(), "Not found asset from id");
   check(idxCard->collection_name == config.get().ASSETS_COLLECTION_NAME, "this collection has not supported");

   atomicassets::schemas_t schemas = atomicassets::get_schemas(idxCard->collection_name);
   auto idxSchema = schemas.find(idxCard->schema_name.value);

   check(idxSchema != schemas.end(), "Not found schema from collection");

   atomicassets::templates_t templates = atomicassets::get_templates(idxCard->collection_name);
   auto idxTemplate = templates.find(idxCard->template_id);

   check(idxTemplate != templates.end(), "Not found template from collection");

   atomicdata::ATTRIBUTE_MAP imdata = atomicdata::deserialize(
      idxTemplate->immutable_serialized_data,
      idxSchema->format
   );

   string name = get<string>(imdata["name"]);
   string tool_key = name.substr(name.size() - 3);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found this account");

   if(idxCard->schema_name == config.get().ASSETS_SCHEMA_TOOLS) {
      auto it_tool = tools.find(player_account.value);
      check(it_tool != tools.end(), "not found tools table from account");

      bool success = false;

      if(tool_key == "Hoe") {
         tools.modify(
            it_tool,
            get_self(),
            [&](auto& s) {
               for(auto i = 0; i < s.toolhoes.size(); i++) {
                  if(s.toolhoes[i].asset_id == asset_id) {
                     check(s.toolhoes[i].energy + energy <= s.toolhoes[i].max_energy, "overflow tool energy");

                     s.toolhoes[i].energy += energy;
                     success = true;

                     break;
                  }
               }
            }
         );
      }
      else if(tool_key == "Can") {
         tools.modify(
            it_tool,
            get_self(),
            [&](auto& s) {
               for(auto i = 0; i < s.toolcans.size(); i++) {
                  if(s.toolcans[i].asset_id == asset_id) {
                     check(s.toolcans[i].energy + energy <= s.toolcans[i].max_energy, "overflow tool energy");

                     s.toolcans[i].energy += energy;
                     success = true;

                     break;
                  }
               }
            }
         );
      }
      else if(tool_key == "Axe") {
         tools.modify(
            it_tool,
            get_self(),
            [&](auto& s) {
               for(auto i = 0; i < s.toolaxes.size(); i++) {
                  if(s.toolaxes[i].asset_id == asset_id) {
                     check(s.toolaxes[i].energy + energy <= s.toolaxes[i].max_energy, "overflow tool energy");

                     s.toolaxes[i].energy += energy;
                     success = true;

                     break;
                  }
               }
            }
         );
      } else {
         check(false, "tool key not support");
      }

      check(success, "not found asset from id");
   } else {
      check(false, "this schema has not supported");
   }
}

ACTION game::unstake(
   name        player_account,
   uint64_t    asset_id
) {
   require_auth(player_account);

   atomicassets::assets_t listAssets = atomicassets::get_assets(get_self());
   auto idxCard = listAssets.find(asset_id);

   check(idxCard != listAssets.end(), "Not found asset from id");
   check(idxCard->collection_name == config.get().ASSETS_COLLECTION_NAME, "this collection has not supported");

   atomicassets::schemas_t schemas = atomicassets::get_schemas(idxCard->collection_name);
   auto idxSchema = schemas.find(idxCard->schema_name.value);

   check(idxSchema != schemas.end(), "Not found schema from collection");

   atomicassets::templates_t templates = atomicassets::get_templates(idxCard->collection_name);
   auto idxTemplate = templates.find(idxCard->template_id);

   check(idxTemplate != templates.end(), "Not found template from collection");

   atomicdata::ATTRIBUTE_MAP imdata = atomicdata::deserialize(
      idxTemplate->immutable_serialized_data,
      idxSchema->format
   );

   string name = get<string>(imdata["name"]);
   string tool_key = name.substr(name.size() - 3);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player account");

   if(idxCard->schema_name == config.get().ASSETS_SCHEMA_TOOLS) {
      auto it_tool = tools.find(player_account.value);
      check(it_tool != tools.end(), "not found tools table from account");

      bool success = false;

      if(tool_key == "Hoe") {
         tools.modify(
            it_tool,
            get_self(),
            [&](auto& s) {
               for(auto i = 0; i < s.toolhoes.size(); i++) {
                  if(s.toolhoes[i].asset_id == asset_id) {
                     // for testnet 1 hr equal 1 second
                     check(now() - s.toolhoes[i].current_time >= s.toolhoes[i].cooldown_hr, "can't unstake please wait tool cooldown");
                     //check(now() - s.toolhoes[i].current_time >= 60 * 60 * s.toolhoes[i].cooldown_hr, "can't unstake please wait tool cooldown");
                     check(s.toolhoes[i].energy >= s.toolhoes[i].max_energy, "can't unstake, because tool energy not full");

                     s.toolhoes.erase(s.toolhoes.begin() + i);
                     success = true;

                     break;
                  }
               }
            }
         );
      }
      else if(tool_key == "Can") {
         tools.modify(
            it_tool,
            get_self(),
            [&](auto& s) {
               for(auto i = 0; i < s.toolcans.size(); i++) {
                  if(s.toolcans[i].asset_id == asset_id) {
                     // for testnet 1 hr equal 1 second
                     check(now() - s.toolcans[i].current_time >= s.toolcans[i].cooldown_hr, "can't unstake please wait tool cooldown");
                     //check(now() - s.toolcans[i].current_time >= 60 * 60 * s.toolcans[i].cooldown_hr, "can't unstake please wait tool cooldown");
                     check(s.toolcans[i].energy >= s.toolcans[i].max_energy, "can't unstake, because tool energy not full");

                     s.toolcans.erase(s.toolcans.begin() + i);
                     success = true;

                     break;
                  }
               }
            }
         );
      }
      else if(tool_key == "Axe") {
         tools.modify(
            it_tool,
            get_self(),
            [&](auto& s) {
               for(auto i = 0; i < s.toolaxes.size(); i++) {
                  if(s.toolaxes[i].asset_id == asset_id) {
                     // for testnet 1 hr equal 1 second
                     check(now() - s.toolaxes[i].current_time >= s.toolaxes[i].cooldown_hr, "can't unstake please wait tool cooldown");
                     //check(now() - s.toolaxes[i].current_time >= 60 * 60 * s.toolaxes[i].cooldown_hr, "can't unstake please wait tool cooldown");
                     check(s.toolaxes[i].energy >= s.toolaxes[i].max_energy, "can't unstake, because tool energy not full");

                     s.toolaxes.erase(s.toolaxes.begin() + i);
                     success = true;

                     break;
                  }
               }
            }
         );
      } else {
         check(false, "tool key not support");
      }

      check(success, "not found asset from id");
   }
   else if(idxCard->schema_name == config.get().ASSETS_SCHEMA_LANDS) {
      auto it_land = lands.find(player_account.value);
      check(it_land != lands.end(), "not found land table from account");
      check(it_land->lands.size() > 1, "can't unstake, because you have default land only");

      bool success = false;

      for(uint8_t i = 1; i < it_land->lands.size(); i++) {
         if(it_land->lands[i].asset_id == asset_id) {
            // for testnet 1 hr equal 1 second
            check(now() - it_land->lands[i].current_time >= it_land->lands[i].cooldown_hr, "the land has not yet completed cooldown");
            //check(now() - it_land->lands[i].current_time >= 60 * 60 * it_land->lands[i].cooldown_hr, "the land has not yet completed cooldown");

            check(it_player->energy >= it_land->lands[i].energy, "not enough energy for unstake land");
            check(it_land->lands[i].house.asset_id == 0, "can't unstake, you need to unstake house on this land first");

            for(uint8_t a = 0; a < it_land->lands[i].blocks_count; a++) {
               check(it_land->lands[i].blocks[a].status == "ready", "can't unstake, the land has some block not ready");
            }

            players.modify(
               it_player,
               get_self(),
               [&](auto& s) {
                  s.energy     -= it_land->lands[i].energy;
                  s.max_energy -= it_land->lands[i].energy;
               }
            );

            lands.modify(
               it_land,
               get_self(),
               [&](auto& s) { s.lands.erase(s.lands.begin() + i); }
            );

            success = true;
            break;
         }
      }

      check(success, "not found land asset from id");
   }
   else if(idxCard->schema_name == config.get().ASSETS_SCHEMA_HOUSE) {
      auto it_land = lands.find(player_account.value);
      check(it_land != lands.end(), "not found land table from account [house]");
      check(it_land->lands.size() > 1, "you not have land and house staking");

      auto it_tool = tools.find(player_account.value);
      check(it_tool != tools.end(), "not found tools table from account");

      bool success = false;

      for(uint8_t i = 1; i < it_land->lands.size(); i++) {
         if(it_land->lands[i].house.asset_id == asset_id) {
            // for testnet 1 hr equal 1 second
            check(now() - it_land->lands[i].house.current_time >= it_land->lands[i].house.cooldown_hr, "the house has not yet completed cooldown");
            //check(now() - it_land->lands[i].house.current_time >= 60 * 60 * it_land->lands[i].house.cooldown_hr, "the house has not yet completed cooldown");

            uint8_t tools_per_type_minus = stoi(it_land->lands[i].house.holding_tools.substr(0, 1));

            check(it_player->tools_per_type - it_tool->toolhoes.size() > tools_per_type_minus, "check and unstake hoe tools first");
            check(it_player->tools_per_type - it_tool->toolcans.size() > tools_per_type_minus, "check and unstake watering can tools first");
            check(it_player->tools_per_type - it_tool->toolaxes.size() > tools_per_type_minus, "check and unstake axe tools first");
            check(it_player->energy >= it_land->lands[i].house.energy, "not enough energy for unstake house");

            players.modify(
               it_player,
               get_self(),
               [&](auto& s) {
                  s.energy         -= it_land->lands[i].house.energy;
                  s.max_energy     -= it_land->lands[i].house.energy;
                  s.tools_per_type -= tools_per_type_minus;
               }
            );

            HOUSE house;
            house.asset_id             = 0;
            house.rarity               = "";
            house.holding_tools        = "";
            house.cooldown_hr          = 0;
            house.energy               = 0;
            house.energy_using         = 0;
            house.coolingdown_bonus    = "";
            house.minting_bonus        = "";
            house.current_time         = 0;

            lands.modify(
               it_land,
               get_self(),
               [&](auto& s) { s.lands[i].house = house; }
            );

            success = true;
            break;
         }
      }

      check(success, "not found house asset from id");
   } else {
      check(false, "this schema has not supported");
   }

   action(
      permission_level {
         get_self(),
         "active"_n
      },
      ASSETS_ACCOUNT,
      "transfer"_n,
      make_tuple(
         get_self(),
         player_account,
         vector<uint64_t>({ asset_id }),
         string("unstake")
      )
   ).send();
}

ACTION game::preparing(
   name              player_account,
   uint64_t          asset_id,
   uint8_t           slot_index,
   vector<uint8_t>   blocks_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_tool = tools.find(player_account.value);
   check(it_tool != tools.end(), "not found player from account");

   check(blocks_index.size() > 0, "not select blocks yet");

   bool found_tool = false;
   uint8_t selected = 0;
   uint32_t use_energy = 0;

   for(uint8_t i = 0; i < it_tool->toolhoes.size(); i++) {
      if(it_tool->toolhoes[i].asset_id == asset_id) {
         found_tool = true;
         selected = i;
         use_energy = it_tool->toolhoes[i].energy_using;

         // for testnet 1 hr equal 1 second
         check(now() - it_tool->toolhoes[selected].current_time >= it_tool->toolhoes[selected].cooldown_hr, "tool not ready");
         //check(now() - it_tool->toolhoes[selected].current_time >= 60 * 60 * it_tool->toolhoes[selected].cooldown_hr, "tool not ready");
         check(it_tool->toolhoes[selected].energy >= use_energy, "tool not enough energy");
         check(it_tool->toolhoes[selected].blocks >= blocks_index.size(), "tool not enough blocks");

         break;
      }
   }

   check(found_tool, "not found tool from asset id");
   check(it_player->energy >= use_energy, "player not enough energy");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land table from account");

   if(slot_index == 0) check(it_land->lands.size() == 1, "can't used land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

   int8_t land_num = slot_index == 0 ? 0 : -1;

   if(land_num == -1) {
      for(uint8_t i = 1; i < it_land->lands.size(); i++) {
         if(it_land->lands[i].slot_index == slot_index) {
            land_num = i;
            break;
         }
      }
   }

   check(land_num > -1, "not found land in slot index");

   for(uint8_t i = 0; i < blocks_index.size(); i++) {
      check(blocks_index[i] < it_land->lands[land_num].blocks.size(), "incorrect block index");
      check(it_land->lands[land_num].blocks[blocks_index[i]].status == "ready", "some blocks is not ready");

      lands.modify(
         it_land,
         get_self(),
         [&](auto& s) {
            s.lands[land_num].blocks[blocks_index[i]].status = "prepared";
         }
      );
   }

   players.modify(
      it_player,
      get_self(),
      [&](auto& s) {
         s.energy -= use_energy;
      }
   );

   uint64_t coolingdown_bonus = 0;

   if(it_land->lands[land_num].house.asset_id != 0) {
      // for testnet 1 hr equal 1 second
      coolingdown_bonus = stoll(it_land->lands[land_num].house.coolingdown_bonus.substr(0, 1));
      //coolingdown_bonus = 60 * 60 * stoll(it_land->lands[land_num].house.coolingdown_bonus.substr(0, 1));
   }

   tools.modify(
      it_tool,
      get_self(),
      [&](auto& s) {
         s.toolhoes[selected].energy       -= use_energy;
         s.toolhoes[selected].current_time  = now() + coolingdown_bonus;
      }
   );
}

ACTION game::puttheseed(
   name              player_account,
   string            rarity,
   uint8_t           slot_index,
   vector<uint8_t>   blocks_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land table from account");

   check(blocks_index.size() > 0, "not select blocks yet");

   if(slot_index == 0) check(it_land->lands.size() == 1, "can't used land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

   int8_t land_num = slot_index == 0 ? 0 : -1;

   if(land_num == -1) {
      for(uint8_t i = 1; i < it_land->lands.size(); i++) {
         if(it_land->lands[i].slot_index == slot_index) {
            land_num = i;
            break;
         }
      }
   }

   check(land_num > -1, "not found land in slot index");

   auto it_seed = seeds.find(player_account.value);
   check(it_seed != seeds.end(), "not found seeds from account");

   if(rarity == "common")         check(it_seed->common >= blocks_index.size(),   "player not enough common seeds");
   else if(rarity == "uncommon")  check(it_seed->uncommon >= blocks_index.size(), "player not enough uncommon seeds");
   else if(rarity == "rare")      check(it_seed->rare >= blocks_index.size(),     "player not enough rare seeds");
   else if(rarity == "legendary") check(it_seed->legend >= blocks_index.size(),   "player not enough legendary seeds");
   else check(false, "rarity not supported");

   for(uint8_t i = 0; i < blocks_index.size(); i++) {
      check(blocks_index[i] < it_land->lands[land_num].blocks.size(), "incorrect block index");
      check(it_land->lands[land_num].blocks[blocks_index[i]].status == "prepared", "some blocks is not prepared");

      lands.modify(
         it_land,
         get_self(),
         [&](auto& s) {
            s.lands[land_num].blocks[blocks_index[i]].status = "watering";
            s.lands[land_num].blocks[blocks_index[i]].rarity = rarity;
         }
      );
   }

   seeds.modify(
      it_seed,
      player_account,
      [&](auto& s) {
         if(rarity == "common")         s.common   -= blocks_index.size();
         else if(rarity == "uncommon")  s.uncommon -= blocks_index.size();
         else if(rarity == "rare")      s.rare     -= blocks_index.size();
         else if(rarity == "legendary") s.legend   -= blocks_index.size();
      }
   );
}

ACTION game::watering(
   name              player_account,
   uint64_t          asset_id,
   uint8_t           slot_index,
   vector<uint8_t>   blocks_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_tool = tools.find(player_account.value);
   check(it_tool != tools.end(), "not found player from account");

   check(blocks_index.size() > 0, "not select blocks yet");

   bool found_tool = false;
   uint8_t selected = 0;
   uint32_t use_energy = 0;

   for(uint8_t i = 0; i < it_tool->toolcans.size(); i++) {
      if(it_tool->toolcans[i].asset_id == asset_id) {
         found_tool = true;
         selected = i;
         use_energy = it_tool->toolcans[i].energy_using;

         // for testnet 1 hr equal 1 second
         check(now() - it_tool->toolcans[selected].current_time >= it_tool->toolcans[selected].cooldown_hr, "tool not ready");
         //check(now() - it_tool->toolcans[selected].current_time >= 60 * 60 * it_tool->toolcans[selected].cooldown_hr, "tool not ready");
         check(it_tool->toolcans[selected].energy >= use_energy, "tool not enough energy");
         check(it_tool->toolcans[selected].blocks >= blocks_index.size(), "tool not enough blocks");

         break;
      }
   }

   check(found_tool, "not found tool from asset id");
   check(it_player->energy >= use_energy, "player not enough energy");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land table from account");

   if(slot_index == 0) check(it_land->lands.size() == 1, "can't used land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

   int8_t land_num = slot_index == 0 ? 0 : -1;

   if(land_num == -1) {
      for(uint8_t i = 1; i < it_land->lands.size(); i++) {
         if(it_land->lands[i].slot_index == slot_index) {
            land_num = i;
            break;
         }
      }
   }

   check(land_num > -1, "not found land in slot index");

   uint64_t coolingdown_bonus = 0;

   if(it_land->lands[land_num].house.asset_id != 0) {
      // for testnet 1 hr equal 1 second
      coolingdown_bonus = stoll(it_land->lands[land_num].house.coolingdown_bonus.substr(0, 1));
      //coolingdown_bonus = 60 * 60 * stoll(it_land->lands[land_num].house.coolingdown_bonus.substr(0, 1));
   }

   for(uint8_t i = 0; i < blocks_index.size(); i++) {
      check(blocks_index[i] < it_land->lands[land_num].blocks.size(), "incorrect block index");
      check(it_land->lands[land_num].blocks[blocks_index[i]].status == "watering", "some blocks is not put seed");

      lands.modify(
         it_land,
         get_self(),
         [&](auto& s) {
            s.lands[land_num].blocks[blocks_index[i]].status       = "harvesting";
            s.lands[land_num].blocks[blocks_index[i]].cooldown_hr  = gameconfig.get().cooldown_growing;
            s.lands[land_num].blocks[blocks_index[i]].current_time = now() + coolingdown_bonus;
         }
      );
   }

   players.modify(
      it_player,
      get_self(),
      [&](auto& s) {
         s.energy -= use_energy;
      }
   );

   tools.modify(
      it_tool,
      get_self(),
      [&](auto& s) {
         s.toolcans[selected].energy      -= use_energy;
         s.toolcans[selected].current_time = now() + coolingdown_bonus;
      }
   );
}

ACTION game::harvesting(
   name              player_account,
   uint64_t          asset_id,
   uint8_t           slot_index,
   vector<uint8_t>   blocks_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_tool = tools.find(player_account.value);
   check(it_tool != tools.end(), "not found player from account");

   check(blocks_index.size() > 0, "not select blocks yet");

   bool found_tool = false;
   uint8_t selected = 0;
   uint32_t use_energy = 0;

   for(uint8_t i = 0; i < it_tool->toolaxes.size(); i++) {
      if(it_tool->toolaxes[i].asset_id == asset_id) {
         found_tool = true;
         selected = i;
         use_energy = it_tool->toolaxes[i].energy_using;

         // for testnet 1 hr equal 1 second
         check(now() - it_tool->toolaxes[selected].current_time >= it_tool->toolaxes[selected].cooldown_hr, "tool not ready");
         //check(now() - it_tool->toolaxes[selected].current_time >= 60 * 60 * it_tool->toolaxes[selected].cooldown_hr, "tool not ready");
         check(it_tool->toolaxes[selected].energy >= use_energy, "tool not enough energy");
         check(it_tool->toolaxes[selected].blocks >= blocks_index.size(), "tool not enough blocks");

         break;
      }
   }

   check(found_tool, "not found tool from asset id");
   check(it_player->energy >= use_energy, "player not enough energy");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land table from account");

   if(slot_index == 0) check(it_land->lands.size() == 1, "can't used land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

   int8_t land_num = slot_index == 0 ? 0 : -1;

   if(land_num == -1) {
      for(uint8_t i = 1; i < it_land->lands.size(); i++) {
         if(it_land->lands[i].slot_index == slot_index) {
            land_num = i;
            break;
         }
      }
   }

   check(land_num > -1, "not found land in slot index");

   auto it_reward = rewards.find(player_account.value);
   check(it_reward != rewards.end(), "not found rewards from account");

   auto it_penalised = penaliseds.find(player_account.value);
   check(it_penalised != penaliseds.end(), "not found penaliseds from account");

   uint32_t reward_common        = 0;
   uint32_t reward_uncommon      = 0;
   uint32_t reward_rare          = 0;
   uint32_t reward_legend        = 0;

   asset new_asset = asset(0, config.get().CORE_TOKEN_SYMBOL);

   asset    penalised_common     = new_asset;
   asset    penalised_uncommon   = new_asset;
   asset    penalised_rare       = new_asset;
   asset    penalised_legend     = new_asset;

   bool is_bonus = false;

   for(uint8_t i = 0; i < blocks_index.size(); i++) {
      check(blocks_index[i] < it_land->lands[land_num].blocks.size(), "incorrect block index");
      check(it_land->lands[land_num].blocks[blocks_index[i]].status == "harvesting", "some blocks is not yet ready to harvest");
      check(now() - it_land->lands[land_num].blocks[blocks_index[i]].current_time >= it_land->lands[land_num].blocks[blocks_index[i]].cooldown_hr, "not harvest, because it not growing");

      string rarity = it_land->lands[land_num].blocks[blocks_index[i]].rarity;

      if(rarity == "common")         reward_common++;
      else if(rarity == "uncommon")  reward_uncommon++;
      else if(rarity == "rare")      reward_rare++;
      else if(rarity == "legendary") reward_legend++;

      string tool_rarity = it_tool->toolaxes[selected].rarity;
      string bonus = it_tool->toolaxes[selected].mining_bonus;

      if(tool_rarity == "common") {
         if(rarity == "legendary") {
            penalised_legend.amount += gameconfig.get().reward_legend.amount * (gameconfig.get().penalised_common_legendary / 100.0);
         }
         else if(rarity == "rare") {
            penalised_rare.amount += gameconfig.get().reward_rare.amount * (gameconfig.get().penalised_common_rare / 100.0);
         }
         else if(rarity == "uncommon") {
            penalised_uncommon.amount += gameconfig.get().reward_uncommon.amount * (gameconfig.get().penalised_common_uncommon / 100.0);
         }
      }
      else if(tool_rarity == "uncommon") {
         if(rarity == "legendary") {
            penalised_legend.amount += gameconfig.get().reward_legend.amount * (gameconfig.get().penalised_uncommon_legendary / 100.0);
         }
         else if(rarity == "rare") {
            penalised_rare.amount += gameconfig.get().reward_rare.amount * (gameconfig.get().penalised_uncommon_rare / 100.0);
         }
      }
      else if(tool_rarity == "rare") {
         if(rarity == "legendary") {
            penalised_legend.amount += gameconfig.get().reward_legend.amount * (gameconfig.get().penalised_rare_legendary / 100.0);
         }
      }
      else if(tool_rarity == "legendary") {
         if(bonus != "0") { 
            auto it_randnumber = randnumbers.find(player_account.value);
            check(it_randnumber != randnumbers.end(), "not found random number table from account");

            if(!is_bonus) {
               char* bonus_c = new char[bonus.length() + 1];
               strcpy(bonus_c, bonus.c_str());
               char* sp = strtok(bonus_c, "-");

               uint64_t min = stoll(sp);
               uint64_t max = stoll(strtok(NULL, "%"));

               randnumbers.modify(
                  it_randnumber,
                  get_self(),
                  [&](auto& s) {
                     s.min_number = min;
                     s.max_number = max;
                     s.status     = "harvest_bonus";
                     s.value      = rarity;
                  }
               );

               is_bonus = true;
            } else {
               randnumbers.modify(
                  it_randnumber,
                  get_self(),
                  [&](auto& s) {
                     s.value += ":"+rarity;
                  }
               );
            }
         }
      }

      asset land_bonus = asset(0, config.get().CORE_TOKEN_SYMBOL);

      if(land_num > 0) {
         uint8_t mining_bonus = stoi(it_land->lands[land_num].mining_bonus.substr(0, 2));

         land_bonus.amount += gameconfig.get().reward_common.amount * reward_common;
         land_bonus.amount += gameconfig.get().reward_uncommon.amount * reward_uncommon;
         land_bonus.amount += gameconfig.get().reward_rare.amount * reward_rare;
         land_bonus.amount += gameconfig.get().reward_legend.amount * reward_legend;

         land_bonus.amount = land_bonus.amount * (mining_bonus / 100.0);
      }

      lands.modify(
         it_land,
         get_self(),
         [&](auto& s) {
            s.lands[land_num].blocks[blocks_index[i]].status       = "ready";
            s.lands[land_num].blocks[blocks_index[i]].rarity       = "";
            s.lands[land_num].blocks[blocks_index[i]].cooldown_hr  = 0;
            s.lands[land_num].blocks[blocks_index[i]].current_time = 0;
            s.lands[land_num].bonus = land_bonus;
         }
      );
   }

   if(is_bonus) {
      action(
         permission_level {
            get_self(),
            "active"_n
         },
         WAX_RNG_ACCOUNT,
         "requestrand"_n,
         make_tuple(
            player_account.value,
            now(),
            get_self()
         )
      ).send();
   }

   players.modify(
      it_player,
      get_self(),
      [&](auto& s) {
         s.energy -= use_energy;
      }
   );

   uint64_t coolingdown_bonus = 0;

   if(it_land->lands[land_num].house.asset_id != 0) {
      // for testnet 1 hr equal 1 second
      coolingdown_bonus = stoll(it_land->lands[land_num].house.coolingdown_bonus.substr(0, 1));
      //coolingdown_bonus = 60 * 60 * stoll(it_land->lands[land_num].house.coolingdown_bonus.substr(0, 1));
   }

   tools.modify(
      it_tool,
      get_self(),
      [&](auto& s) {
         s.toolaxes[selected].energy      -= use_energy;
         s.toolaxes[selected].current_time = now() + coolingdown_bonus;
      }
   );

   rewards.modify(
      it_reward,
      player_account,
      [&](auto& s) {
         s.common   += reward_common;
         s.uncommon += reward_uncommon;
         s.rare     += reward_rare;
         s.legend   += reward_legend;
      }
   );

   penaliseds.modify(
      it_penalised,
      player_account,
      [&](auto& s) {
         s.common.amount   += penalised_common.amount;
         s.uncommon.amount += penalised_uncommon.amount;
         s.rare.amount     += penalised_rare.amount;
         s.legend.amount   += penalised_legend.amount;
      }
   );
}

ACTION game::sellreward(
   name        player_account
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_reward = rewards.find(player_account.value);
   check(it_reward != rewards.end(), "not found rewards from account");

   auto it_penalised = penaliseds.find(player_account.value);
   check(it_penalised != penaliseds.end(), "not found penaliseds from account");

   auto it_bonusreward = bonusrewards.find(player_account.value);
   check(it_bonusreward != bonusrewards.end(), "not found bonus rewards from account");

   uint64_t reward_token = 0;
   reward_token += (it_reward->common * gameconfig.get().reward_common.amount) + it_bonusreward->common.amount - it_penalised->common.amount;
   reward_token += (it_reward->uncommon * gameconfig.get().reward_uncommon.amount) + it_bonusreward->uncommon.amount - it_penalised->uncommon.amount;
   reward_token += (it_reward->rare * gameconfig.get().reward_rare.amount) + it_bonusreward->rare.amount - it_penalised->rare.amount;
   reward_token += (it_reward->legend * gameconfig.get().reward_legend.amount) + it_bonusreward->legend.amount - it_penalised->legend.amount;

   check(reward_token > 0, "not enough reward left");

   asset quantity = asset(reward_token, config.get().CORE_TOKEN_SYMBOL);

   rewards.modify(
      it_reward,
      player_account,
      [&](auto& s) {
         s.common   = 0;
         s.uncommon = 0;
         s.rare     = 0;
         s.legend   = 0;
      }
   );

   asset new_asset = asset(0, config.get().CORE_TOKEN_SYMBOL);

   penaliseds.modify(
      it_penalised,
      player_account,
      [&](auto& s) {
         s.common   = new_asset;
         s.uncommon = new_asset;
         s.rare     = new_asset;
         s.legend   = new_asset;
      }
   );

   bonusrewards.modify(
      it_bonusreward,
      player_account,
      [&](auto& s) {
         s.common   = new_asset;
         s.uncommon = new_asset;
         s.rare     = new_asset;
         s.legend   = new_asset;
      }
   );

   action(
      permission_level {
         get_self(),
         "active"_n
      },
      config.get().CORE_TOKEN_ACCOUNT,
      "issuesuper"_n,
      make_tuple(
         get_self(),
         player_account,
         quantity,
         string("sell::rewards - issue")
      )
   ).send();
}

ACTION game::claimland(
   name     player_account,
   uint8_t  slot_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land table from account");

   if(slot_index == 0) check(it_land->lands.size() == 1, "can not claim land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

   int8_t land_num = slot_index == 0 ? 0 : -1;

   if(land_num == -1) {
      for(uint8_t i = 1; i < it_land->lands.size(); i++) {
         if(it_land->lands[i].slot_index == slot_index) {
            land_num = i;
            break;
         }
      }
   }

   check(land_num > 0, "not found land in slot index");
   check(now() - it_land->lands[land_num].current_time >= it_land->lands[land_num].cooldown_hr, "land can't claim, because cooldown not yet");

   uint32_t use_energy = it_land->lands[land_num].energy_using;
   check(it_player->energy >= use_energy, "player not enough energy");

   asset land_bonus = it_land->lands[land_num].bonus;

   lands.modify(
      it_land,
      get_self(),
      [&](auto& s) {
         s.lands[land_num].current_time = now();
         s.lands[land_num].bonus = asset(0, config.get().CORE_TOKEN_SYMBOL);
      }
   );

   players.modify(
      it_player,
      get_self(),
      [&](auto& s) { s.energy -= use_energy; }
   );

   uint8_t i = stoi(it_land->lands[land_num].minting_bonus.substr(0, 1));

   while(i > 0) {
      action(
         permission_level {
            get_self(),
            "active"_n
         },
         ASSETS_ACCOUNT,
         "mintasset"_n,
         make_tuple(
            get_self(),
            config.get().ASSETS_COLLECTION_NAME,
            config.get().ASSETS_SCHEMA_LANDS,
            nfttemplates.get().seed_pack_template_id,
            player_account,
            map<string, ATOMIC_ATTRIBUTE>(),
            map<string, ATOMIC_ATTRIBUTE>(),
            vector<asset>()
         )         
      ).send();

      i--;
   }

   if(land_bonus.amount > 0) {
      action(
         permission_level {
            get_self(),
            "active"_n
         },
         config.get().CORE_TOKEN_ACCOUNT,
         "issuesuper"_n,
         make_tuple(
            get_self(),
            player_account,
            land_bonus,
            string("sell::rewards - issue")
         )
      ).send();
   }
}

ACTION game::claimhouse(
   name     player_account,
   uint8_t  slot_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land table from account [house]");

   if(slot_index == 0) check(it_land->lands.size() == 1, "can not claim house on land default");
   else check(it_land->lands.size() > 1, "you don't have lands and house staking");

   int8_t land_num = slot_index == 0 ? 0 : -1;

   if(land_num == -1) {
      for(uint8_t i = 1; i < it_land->lands.size(); i++) {
         if(it_land->lands[i].slot_index == slot_index) {
            land_num = i;
            break;
         }
      }
   }

   check(land_num > 0, "not found land in slot index [house]");
   check(it_land->lands[land_num].house.asset_id != 0, "not found house in slot index");
   check(now() - it_land->lands[land_num].house.current_time >= it_land->lands[land_num].house.cooldown_hr, "house can't claim, because cooldown not yet");

   uint32_t use_energy = it_land->lands[land_num].house.energy_using;
   check(it_player->energy >= use_energy, "player not enough energy");

   lands.modify(
      it_land,
      get_self(),
      [&](auto& s) { s.lands[land_num].house.current_time = now(); }
   );

   players.modify(
      it_player,
      get_self(),
      [&](auto& s) { s.energy -= use_energy; }
   );

   uint8_t i = stoi(it_land->lands[land_num].house.minting_bonus.substr(0, 1));

   while(i > 0) {
      action(
         permission_level {
            get_self(),
            "active"_n
         },
         ASSETS_ACCOUNT,
         "mintasset"_n,
         make_tuple(
            get_self(),
            config.get().ASSETS_COLLECTION_NAME,
            config.get().ASSETS_SCHEMA_LANDS,
            nfttemplates.get().tool_pack_template_id,
            player_account,
            map<string, ATOMIC_ATTRIBUTE>(),
            map<string, ATOMIC_ATTRIBUTE>(),
            vector<asset>()
         )         
      ).send();

      i--;
   }
}

ACTION game::receiverand(
   uint64_t       assoc_id,
   checksum256&   random_value
) {
   require_auth(WAX_RNG_ACCOUNT);

   auto byte_array = random_value.extract_as_byte_array();
   uint64_t random_int = 0;

   for(int i = 0; i < 8; i++) {
      random_int <<= 8;
      random_int |= (uint64_t)byte_array[i];
   }

   auto it_randnumber = randnumbers.find(assoc_id);
   check(it_randnumber != randnumbers.end(), "not found account from assoc_id");

   uint64_t max_value = it_randnumber->max_number - it_randnumber->min_number;
   uint64_t result = (random_int % max_value) + it_randnumber->min_number;

   if(it_randnumber->status == "harvest_bonus") {
      auto it_bonusreward = bonusrewards.find(assoc_id);
      check(it_bonusreward != bonusrewards.end(), "not found bonus rewards from account");

      char* bonus_c = new char[it_randnumber->value.length() + 1];
      strcpy(bonus_c, it_randnumber->value.c_str());
      char* sp = strtok(bonus_c, ":");

      while(sp != NULL) {
         string sp_str = sp;

         bonusrewards.modify(
            it_bonusreward,
            it_randnumber->player_account,
            [&](auto& s) {
               s.common.amount += sp_str == "common" ? (gameconfig.get().reward_common.amount * (result / 100.0)) : 0;
               s.uncommon.amount += sp_str == "uncommon" ? (gameconfig.get().reward_uncommon.amount * (result / 100.0)) : 0;
               s.rare.amount += sp_str == "rare" ? (gameconfig.get().reward_rare.amount * (result / 100.0)) : 0;
               s.legend.amount += sp_str == "legendary" ? (gameconfig.get().reward_legend.amount * (result / 100.0)) : 0;
            }
         );

         sp = strtok(NULL, ":");
      }
   } else {
      check(false, "incorrect status not match");
   }

   randnumbers.modify(
      it_randnumber,
      get_self(),
      [&](auto& s) {
         s.min_number = 0;
         s.max_number = 0;
         s.status = "";
         s.value = "";
      }
   );
}

void game::on_transfer_nft(
   name              from,
   name              to,
   vector<uint64_t>  asset_ids,
   string            memo
) {
   const set <name> ignore = set <name> {
      ASSETS_ACCOUNT,
      "eosio.stake"_n,
      "eosio.names"_n,
      "eosio.ram"_n,
      "eosio.rex"_n,
      "eosio"_n
   };

   if(to != get_self() || ignore.find(from) != ignore.end()) return;

   check(asset_ids.size() == 1, "Only one asset can be stake at a time");

   if(memo.find("stake") == 0) {
      atomicassets::assets_t listAssets = atomicassets::get_assets(get_self());
      auto idxCard = listAssets.find(asset_ids[0]);

      check(idxCard != listAssets.end(), "Not found asset from id");
      check(idxCard->collection_name == config.get().ASSETS_COLLECTION_NAME, "this collection has not supported");

      atomicassets::schemas_t schemas = atomicassets::get_schemas(idxCard->collection_name);
      auto idxSchema = schemas.find(idxCard->schema_name.value);

      check(idxSchema != schemas.end(), "Not found schema from collection");

      atomicassets::templates_t templates = atomicassets::get_templates(idxCard->collection_name);
      auto idxTemplate = templates.find(idxCard->template_id);

      check(idxTemplate != templates.end(), "Not found template from collection");

      atomicdata::ATTRIBUTE_MAP imdata = atomicdata::deserialize(
         idxTemplate->immutable_serialized_data,
         idxSchema->format
      );

      auto it_player = players.find(from.value);
      check(it_player != players.end(), "not found player account");

      if(idxCard->schema_name == config.get().ASSETS_SCHEMA_SEEDS) {
         string rarity = get<string>(imdata["rarity"]);

         auto it_seed = seeds.find(from.value);
         check(it_seed != seeds.end(), "not found seeds in account");

         seeds.modify(
            it_seed,
            from,
            [&](auto& s) {
               if(rarity == "Common") s.common += 4;
               else if(rarity == "Uncommon") s.uncommon += 4;
               else if(rarity == "Rare") s.rare += 4;
               else if(rarity == "Legendary") s.legend += 4;
               else check(false, "incorrect rarity not support");
            }
         );

         action(
            permission_level {
               get_self(),
               "active"_n
            },
            ASSETS_ACCOUNT,
            "burnasset"_n,
            make_tuple(
               get_self(),
               asset_ids[0]
            )
         ).send();
      }
      else if(idxCard->schema_name == config.get().ASSETS_SCHEMA_TOOLS) {
         string name = get<string>(imdata["name"]);
         string rarity = get<string>(imdata["rarity"]);
         transform(rarity.begin(), rarity.end(), rarity.begin(), [](unsigned char c) { return tolower(c); });
         uint32_t cooldown_hr = get<uint64_t>(imdata["cooldown_hr"]);
         uint32_t energy = get<uint64_t>(imdata["energy"]);
         uint32_t energy_using = get<uint64_t>(imdata["energy_using"]);
         uint32_t blocks = get<uint64_t>(imdata["blocks"]);
         string mining_bonus = get<string>(imdata["mining_bonus"]);

         string tool_key = name.substr(name.size() - 3);

         TOOL tool;
         tool.asset_id = asset_ids[0];
         tool.rarity = rarity;
         tool.cooldown_hr = cooldown_hr;
         tool.energy = energy;
         tool.max_energy = energy;
         tool.energy_using = energy_using;
         tool.blocks = blocks;
         tool.mining_bonus = mining_bonus;
         tool.current_time = 0;

         auto it_tool = tools.find(from.value);
         check(it_tool != tools.end(), "not found tools table from account");

         if(tool_key == "Hoe") {
            check(it_tool->toolhoes.size() < it_player->tools_per_type, "hoe tools is full");
            tools.modify(it_tool, get_self(), [&](auto& s) { s.toolhoes.push_back(tool); });
         }
         else if(tool_key == "Can") {
            check(it_tool->toolcans.size() < it_player->tools_per_type, "watering can tools is full");
            tools.modify(it_tool, get_self(), [&](auto& s) { s.toolcans.push_back(tool); });
         }
         else if(tool_key == "Axe") {
            check(it_tool->toolaxes.size() < it_player->tools_per_type, "axe tools is full");
            tools.modify(it_tool, get_self(), [&](auto& s) { s.toolaxes.push_back(tool); });
         } else {
            check(false, "tool key not support");
         }
      }
      else if(idxCard->schema_name == config.get().ASSETS_SCHEMA_LANDS) {
         string rarity = get<string>(imdata["rarity"]);
         uint32_t cooldown_hr = get<uint64_t>(imdata["cooldown_hr"]);
         uint32_t energy = get<uint64_t>(imdata["energy"]);
         uint32_t energy_using = get<uint64_t>(imdata["energy_using"]);
         uint32_t blocks_count = get<uint64_t>(imdata["blocks"]);
         string mining_bonus = get<string>(imdata["mining_bonus"]);
         string minting_bonus = get<string>(imdata["minting_bonus"]);

         uint8_t slot_index = stoi(memo.substr(5));

         HOUSE house;
         house.asset_id             = 0;
         house.rarity               = "";
         house.holding_tools        = "";
         house.cooldown_hr          = 0;
         house.energy               = 0;
         house.energy_using         = 0;
         house.coolingdown_bonus    = "";
         house.minting_bonus        = "";
         house.current_time         = 0;

         LAND land_asset;
         land_asset.asset_id         = asset_ids[0];
         land_asset.slot_index       = slot_index;
         land_asset.rarity           = rarity;
         land_asset.cooldown_hr      = cooldown_hr;
         land_asset.energy           = energy;
         land_asset.energy_using     = energy_using;
         land_asset.blocks_count     = blocks_count;
         land_asset.house            = house;
         land_asset.bonus            = asset(0, config.get().CORE_TOKEN_SYMBOL);
         land_asset.mining_bonus     = mining_bonus;
         land_asset.minting_bonus    = minting_bonus;
         land_asset.current_time     = now();

         for(uint8_t i = 0; i < blocks_count; i++) {
            BLOCK new_block;
            new_block.status        = "ready";
            new_block.rarity        = "";
            new_block.cooldown_hr   = 0;
            new_block.current_time  = 0;

            land_asset.blocks.push_back(new_block);
         }

         auto it_land = lands.find(from.value);
         check(it_land != lands.end(), "not found land table from account");

         if(it_land->lands.size() == 1) {
            for(uint8_t i = 0; i < it_land->lands[0].blocks.size(); i++) {
               check(it_land->lands[0].blocks[i].status == "ready", "the default land is not yet ready to stake, please check all blocks status");
            }
         } else {
            check(it_land->lands.size() < gameconfig.get().land_limit + 1, "can't stake land, because full land limited");
         }

         check(slot_index > 0 && slot_index <= gameconfig.get().land_limit, "incorrect slot index");

         for(uint8_t i = 1; i < it_land->lands.size(); i++) {
            check(it_land->lands[i].slot_index != slot_index, "the land is already in slot index");
         }

         lands.modify(
            it_land,
            get_self(),
            [&](auto& s) { s.lands.push_back(land_asset); }
         );

         players.modify(
            it_player,
            get_self(),
            [&](auto& s) {
               s.energy += energy;
               s.max_energy += energy;
            }
         );
      }
      else if(idxCard->schema_name == config.get().ASSETS_SCHEMA_HOUSE) {
         string rarity = get<string>(imdata["rarity"]);
         string holding_tools = get<string>(imdata["holding_tools"]);
         uint32_t cooldown_hr = get<uint64_t>(imdata["cooldown_hr"]);
         uint32_t energy = get<uint64_t>(imdata["energy"]);
         uint32_t energy_using = get<uint64_t>(imdata["energy_using"]);
         string coolingdown_bonus = get<string>(imdata["coolingdown_bonus"]);
         string minting_bonus = get<string>(imdata["minting_bonus"]);

         uint8_t slot_index = stoi(memo.substr(5));

         HOUSE house;
         house.asset_id             = asset_ids[0];
         house.rarity               = rarity;
         house.holding_tools        = holding_tools;
         house.cooldown_hr          = cooldown_hr;
         house.energy               = energy;
         house.energy_using         = energy_using;
         house.coolingdown_bonus    = coolingdown_bonus;
         house.minting_bonus        = minting_bonus;
         house.current_time         = now();

         auto it_land = lands.find(from.value);
         check(it_land != lands.end(), "not found land table from account");
         check(it_land->lands.size() > 1, "you can't stake house, you need to stake land first");
         check(slot_index > 0 && slot_index < gameconfig.get().land_limit + 1, "incorrect land slot index");

         uint8_t selected = 0;

         for(uint8_t i = 1; i < it_land->lands.size(); i++) {
            if(it_land->lands[i].slot_index == slot_index) {
               check(it_land->lands[i].house.asset_id != 0, "this land has stake house already");

               selected = i;
               break;
            }
         }

         check(selected > 0, "not found land from slot index");

         lands.modify(
            it_land,
            get_self(),
            [&](auto& s) { s.lands[selected].house = house; }
         );

         uint8_t tools_per_type_plus = stoi(holding_tools.substr(0, 1));

         players.modify(
            it_player,
            get_self(),
            [&](auto& s) {
               s.energy += energy;
               s.max_energy += energy;
               s.tools_per_type += tools_per_type_plus;
            }
         );
      } else {
         check(false, "this schema has not supported");
      }
   } else {
      check(false, "invalid memo");
   }
}

uint64_t game::now() { return current_time_point().sec_since_epoch(); }

uint64_t game::math_pow(
   uint64_t base,
   uint8_t exp
) {
   uint64_t result = base > 0 ? base : 0;
   for(int8_t i = 1; i < exp; i++) result *= base;
   return result;
}