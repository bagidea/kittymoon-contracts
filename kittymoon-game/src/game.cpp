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
   asset       reward_common,
   asset       reward_uncommon,
   asset       reward_rare,
   asset       reward_legend,
   uint32_t    cooldown_growing
) {
   require_auth(get_self());

   gameconfig.set(
      {
         .first_energy     = first_energy,
         .reward_common    = reward_common,
         .reward_uncommon  = reward_uncommon,
         .reward_rare      = reward_rare,
         .reward_legend    = reward_legend,
         .cooldown_growing = cooldown_growing
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
      player_account,
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

   tools.emplace(
      get_self(),
      [&](auto& s) {
         s.player_account = player_account;
      }
   );

   lands.emplace(
      get_self(),
      [&](auto& s) {
         s.player_account = player_account;

         LAND land_default;
         land_default.asset_id         = 0;
         land_default.rarity           = "default";
         land_default.cooldown_hr      = 0;
         land_default.energy           = 0;
         land_default.max_energy       = 0;
         land_default.energy_using     = 0;
         land_default.blocks_count     = 4;
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
      player_account,
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
      player_account,
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
   } else {
      check(false, "this schema has not supported");
   }
}

ACTION game::preparing(
   name              player_account,
   uint64_t          asset_id,
   uint8_t           land_num,
   vector<uint8_t>   blocks_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_tool = tools.find(player_account.value);
   check(it_tool != tools.end(), "not found player from account");

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
         //check(now() - it_tool->toolhoes[selected].current_time >= 60 * 60 *it_tool->toolhoes[selected].cooldown_hr, "tool not ready");
         check(it_tool->toolhoes[selected].energy >= use_energy, "tool not enough energy");
         check(it_tool->toolhoes[selected].blocks >= blocks_index.size(), "tool not enough blocks");

         break;
      }
   }

   check(found_tool, "not found tool from asset id");
   check(it_player->energy >= use_energy, "player not enough energy");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land from account");

   if(land_num == 0) check(it_land->lands.size() == 1, "can't used land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

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
      player_account,
      [&](auto& s) {
         s.energy -= use_energy;
      }
   );

   tools.modify(
      it_tool,
      get_self(),
      [&](auto& s) {
         s.toolhoes[selected].energy -= use_energy;
         s.toolhoes[selected].current_time = now();
      }
   );
}

ACTION game::puttheseed(
   name              player_account,
   string            rarity,
   uint8_t           land_num,
   vector<uint8_t>   blocks_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land from account");

   if(land_num == 0) check(it_land->lands.size() == 1, "can't used land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

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
   uint8_t           land_num,
   vector<uint8_t>   blocks_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_tool = tools.find(player_account.value);
   check(it_tool != tools.end(), "not found player from account");

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
         //check(now() - it_tool->toolcans[selected].current_time >= 60 * 60 *it_tool->toolcans[selected].cooldown_hr, "tool not ready");
         check(it_tool->toolcans[selected].energy >= use_energy, "tool not enough energy");
         check(it_tool->toolcans[selected].blocks >= blocks_index.size(), "tool not enough blocks");

         break;
      }
   }

   check(found_tool, "not found tool from asset id");
   check(it_player->energy >= use_energy, "player not enough energy");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land from account");

   if(land_num == 0) check(it_land->lands.size() == 1, "can't used land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

   for(uint8_t i = 0; i < blocks_index.size(); i++) {
      check(blocks_index[i] < it_land->lands[land_num].blocks.size(), "incorrect block index");
      check(it_land->lands[land_num].blocks[blocks_index[i]].status == "watering", "some blocks is not watering");

      lands.modify(
         it_land,
         get_self(),
         [&](auto& s) {
            s.lands[land_num].blocks[blocks_index[i]].status       = "harvesting";
            s.lands[land_num].blocks[blocks_index[i]].cooldown_hr  = gameconfig.get().cooldown_growing;
            s.lands[land_num].blocks[blocks_index[i]].current_time = now();
         }
      );
   }

   players.modify(
      it_player,
      player_account,
      [&](auto& s) {
         s.energy -= use_energy;
      }
   );

   tools.modify(
      it_tool,
      get_self(),
      [&](auto& s) {
         s.toolcans[selected].energy      -= use_energy;
         s.toolcans[selected].current_time = now();
      }
   );
}

ACTION game::harvesting(
   name              player_account,
   uint64_t          asset_id,
   uint8_t           land_num,
   vector<uint8_t>   blocks_index
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_tool = tools.find(player_account.value);
   check(it_tool != tools.end(), "not found player from account");

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
         //check(now() - it_tool->toolaxes[selected].current_time >= 60 * 60 *it_tool->toolaxes[selected].cooldown_hr, "tool not ready");
         check(it_tool->toolaxes[selected].energy >= use_energy, "tool not enough energy");
         check(it_tool->toolaxes[selected].blocks >= blocks_index.size(), "tool not enough blocks");

         break;
      }
   }

   check(found_tool, "not found tool from asset id");
   check(it_player->energy >= use_energy, "player not enough energy");

   auto it_land = lands.find(player_account.value);
   check(it_land != lands.end(), "not found land from account");

   if(land_num == 0) check(it_land->lands.size() == 1, "can't used land default");
   else check(it_land->lands.size() > 1, "you don't have lands staking");

   auto it_reward = rewards.find(player_account.value);
   check(it_reward != rewards.end(), "not found rewards from account");

   uint32_t reward_common   = 0;
   uint32_t reward_uncommon = 0;
   uint32_t reward_rare   = 0;
   uint32_t reward_legend   = 0;

   for(uint8_t i = 0; i < blocks_index.size(); i++) {
      check(blocks_index[i] < it_land->lands[land_num].blocks.size(), "incorrect block index");
      check(it_land->lands[land_num].blocks[blocks_index[i]].status == "harvesting", "some blocks is not harvesting");
      check(now() - it_land->lands[land_num].blocks[blocks_index[i]].current_time >= it_land->lands[land_num].blocks[blocks_index[i]].cooldown_hr, "not harvest, because it not growing");

      string rarity = it_land->lands[land_num].blocks[blocks_index[i]].rarity;

      if(rarity == "common")         reward_common++;
      else if(rarity == "uncommon")  reward_uncommon++;
      else if(rarity == "rare")      reward_rare++;
      else if(rarity == "legendary") reward_legend++;

      lands.modify(
         it_land,
         get_self(),
         [&](auto& s) {
            s.lands[land_num].blocks[blocks_index[i]].status       = "ready";
            s.lands[land_num].blocks[blocks_index[i]].rarity       = "";
            s.lands[land_num].blocks[blocks_index[i]].cooldown_hr  = 0;
            s.lands[land_num].blocks[blocks_index[i]].current_time = 0;
         }
      );
   }

   players.modify(
      it_player,
      player_account,
      [&](auto& s) {
         s.energy -= use_energy;
      }
   );

   tools.modify(
      it_tool,
      get_self(),
      [&](auto& s) {
         s.toolaxes[selected].energy      -= use_energy;
         s.toolaxes[selected].current_time = now();
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
}

ACTION game::sellreward(
   name        player_account,
   uint32_t    common,
   uint32_t    uncommon,
   uint32_t    rare,
   uint32_t    legendary
) {
   require_auth(player_account);

   auto it_player = players.find(player_account.value);
   check(it_player != players.end(), "not found player from account");

   auto it_reward = rewards.find(player_account.value);
   check(it_reward != rewards.end(), "not found rewards from account");

   check(it_reward->common >= common, "not enough common reward");
   check(it_reward->uncommon >= uncommon, "not enough uncommon reward");
   check(it_reward->rare >= rare, "not enough rare reward");
   check(it_reward->legend >= legendary, "not enough legendary reward");

   uint64_t precision = math_pow(10, config.get().CORE_TOKEN_SYMBOL.precision());

   uint64_t reward_token = 0;
   reward_token         += common * precision;
   reward_token         += uncommon * precision;
   reward_token         += rare * precision;
   reward_token         += legendary * precision;

   asset quantity = asset(reward_token, config.get().CORE_TOKEN_SYMBOL);

   action(
      permission_level {
         get_self(),
         "active"_n
      },
      config.get().CORE_TOKEN_ACCOUNT,
      "issuesuper"_n,
      make_tuple(
         player_account,
         quantity,
         string("sell::reward - issue")
      )
   ).send();
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

         auto it_player = players.find(from.value);
         check(it_player != players.end(), "not found player account");

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
         check(false, "Land.");
      }
      else if(idxCard->schema_name == config.get().ASSETS_SCHEMA_HOUSE) {
         check(false, "House.");
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