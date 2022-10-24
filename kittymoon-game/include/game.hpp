#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

#include <string>
#include <cstring>
#include <vector>

#include <atomicassets-interface.hpp>
#include <atomicdata.hpp>

using namespace eosio;
using namespace std;
using namespace atomicassets;
using namespace atomicdata;

static constexpr name ASSETS_ACCOUNT         = "atomicassets"_n;
static constexpr name WAX_RNG_ACCOUNT        = "orng.wax"_n;

CONTRACT game : public contract {
   public:
      using contract::contract;

      struct TOOL {
         uint64_t    asset_id;
         string      rarity;
         uint32_t    cooldown_hr;
         uint32_t    energy;
         uint32_t    max_energy;
         uint32_t    energy_using;
         uint32_t    blocks;
         string      mining_bonus;
         uint64_t    current_time;
      };

      struct BLOCK {
         string      status;
         string      rarity;
         uint32_t    cooldown_hr;
         uint64_t    current_time;
      };

      struct HOUSE {
         uint64_t       asset_id;
         string         rarity;
         uint32_t       holding_tools;
         uint32_t       cooldown_hr;
         uint32_t       energy;
         uint32_t       energy_using;
         string         coolingdown_bonus;
         string         minting_bonus;
         uint64_t       current_time;
      };

      struct LAND {
         uint64_t       asset_id;
         uint8_t        slot_index;
         string         rarity;
         uint32_t       cooldown_hr;
         uint32_t       energy;
         uint32_t       energy_using;
         uint8_t        blocks_count;
         vector<BLOCK>  blocks;
         HOUSE          house;
         asset          bonus;
         string         mining_bonus;
         string         minting_bonus;
         uint64_t       current_time;
      };

      game(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds) {}

      ACTION setconfig(
         name     CORE_TOKEN_ACCOUNT,
         symbol   CORE_TOKEN_SYMBOL,
         name     ASSETS_COLLECTION_NAME,
         name     CORE_SHOP_ACCOUNT,
         name     ASSETS_SCHEMA_SEEDS,
         name     ASSETS_SCHEMA_TOOLS,
         name     ASSETS_SCHEMA_LANDS,
         name     ASSETS_SCHEMA_HOUSE
      );

      ACTION setgameconfig(
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
      );

      ACTION setnftconfig(
         int32_t     reward_common_template_id,
         int32_t     reward_uncommon_template_id,
         int32_t     reward_rare_template_id,
         int32_t     reward_legend_template_id,
         int32_t     seed_pack_template_id,
         int32_t     tool_pack_template_id
      );

      ACTION signup(
         name     player_account,
         string   player_name
      );

      ACTION repairplayer(
         name     player_account
      );

      ACTION modifyname(
         name     player_account,
         string   player_name
      );

      ACTION addenergy(
         name        authorized_account,
         name        player_account,
         uint32_t    energy
      );

      ACTION addenergyt(
         name        authorized_account,
         name        player_account,
         uint64_t    asset_id,
         uint32_t    energy
      );

      ACTION unstake(
         name        player_account,
         uint64_t    asset_id
      );

      ACTION preparing(
         name              player_account,
         uint64_t          asset_id,
         uint8_t           slot_index,
         vector<uint8_t>   blocks_index
      );

      ACTION puttheseed(
         name              player_account,
         string            rarity,
         uint8_t           slot_index,
         vector<uint8_t>   blocks_index
      );

      ACTION watering(
         name              player_account,
         uint64_t          asset_id,
         uint8_t           land_num,
         vector<uint8_t>   blocks_index
      );

      ACTION harvesting(
         name              player_account,
         uint64_t          asset_id,
         uint8_t           land_num,
         vector<uint8_t>   blocks_index
      );

      ACTION sellreward(
         name        player_account
      );

      ACTION receiverand(
         uint64_t       assoc_id,
         checksum256&   random_value
      );

      [[eosio::on_notify("atomicassets::transfer")]]
      void on_transfer_nft(
         name              from,
         name              to,
         vector<uint64_t>  asset_ids,
         string            memo
      );

      uint64_t now();

      uint64_t math_pow(
         uint64_t base,
         uint8_t exp
      );

   private:
      TABLE config_init {
         name     CORE_TOKEN_ACCOUNT;
         symbol   CORE_TOKEN_SYMBOL;
         name     ASSETS_COLLECTION_NAME;
         name     CORE_SHOP_ACCOUNT;
         name     ASSETS_SCHEMA_SEEDS;
         name     ASSETS_SCHEMA_TOOLS;
         name     ASSETS_SCHEMA_LANDS;
         name     ASSETS_SCHEMA_HOUSE;
      };

      typedef singleton<"config"_n, config_init> config_t;
      typedef multi_index<"config"_n, config_init> config_t_for_abi;

      TABLE game_config {
         uint32_t    first_energy;
         uint8_t     land_limit;
         asset       reward_common;
         asset       reward_uncommon;
         asset       reward_rare;
         asset       reward_legend;
         uint32_t    cooldown_growing;
         uint32_t    penalised_rare_legendary;
         uint32_t    penalised_uncommon_legendary;
         uint32_t    penalised_uncommon_rare;
         uint32_t    penalised_common_legendary;
         uint32_t    penalised_common_rare;
         uint32_t    penalised_common_uncommon;
      };

      typedef singleton<"gameconfig"_n, game_config> gameconfig_t;
      typedef multi_index<"gameconfig"_n, game_config> gameconfig_t_for_abi;

      TABLE nft_template {
         int32_t    reward_common_template_id;
         int32_t    reward_uncommon_template_id;
         int32_t    reward_rare_template_id;
         int32_t    reward_legend_template_id;
         int32_t    seed_pack_template_id;
         int32_t    tool_pack_template_id;
      };

      typedef singleton<"nfttemplates"_n, nft_template> nfttemplate_t;
      typedef multi_index<"nfttemplates"_n, nft_template> nfttemplate_t_for_abi;

      TABLE player {
         name        player_account;
         string      player_name;
         uint32_t    energy;
         uint32_t    max_energy;
         uint8_t     tools_per_type;

         uint64_t primary_key() const { return player_account.value; }
      };

      typedef multi_index<"players"_n, player> player_t;

      TABLE seed {
         name        player_account;
         uint32_t    common;
         uint32_t    uncommon;
         uint32_t    rare;
         uint32_t    legend;

         uint64_t primary_key() const { return player_account.value; }
      };

      typedef multi_index<"seeds"_n, seed> seed_t;

      TABLE reward {
         name        player_account;
         uint32_t    common;
         uint32_t    uncommon;
         uint32_t    rare;
         uint32_t    legend;

         uint64_t primary_key() const { return player_account.value; }
      };

      typedef multi_index<"rewards"_n, reward> reward_t;

      TABLE bonus_reward {
         name        player_account;
         asset       common;
         asset       uncommon;
         asset       rare;
         asset       legend;

         uint64_t primary_key() const { return player_account.value; }
      };

      typedef multi_index<"bonusrewards"_n, bonus_reward> bonusreward_t;

      TABLE penalised {
         name        player_account;
         asset       common;
         asset       uncommon;
         asset       rare;
         asset       legend;

         uint64_t primary_key() const { return player_account.value; }
      };

      typedef multi_index<"penaliseds"_n, penalised> penalised_t;

      TABLE tool {
         name           player_account;
         vector<TOOL>   toolhoes;
         vector<TOOL>   toolcans;
         vector<TOOL>   toolaxes;

         uint64_t primary_key() const { return player_account.value; }
      };

      typedef multi_index<"tools"_n, tool> tool_t;

      TABLE randnumber {
         name           player_account;
         uint64_t       min_number;
         uint64_t       max_number;
         string         status;
         string         value;

         uint64_t primary_key() const { return player_account.value; }
      };

      typedef multi_index<"randnumbers"_n, randnumber> randnumber_t;

      TABLE land {
         name           player_account;
         vector<LAND>   lands;

         uint64_t primary_key() const { return player_account.value; }
      };

      typedef multi_index<"lands"_n, land> land_t;

      config_t config = config_t(get_self(), get_self().value);
      gameconfig_t gameconfig = gameconfig_t(get_self(), get_self().value);
      nfttemplate_t nfttemplates = nfttemplate_t(get_self(), get_self().value);
      player_t players = player_t(get_self(), get_self().value);
      seed_t seeds = seed_t(get_self(), get_self().value);
      reward_t rewards = reward_t(get_self(), get_self().value);
      bonusreward_t bonusrewards = bonusreward_t(get_self(), get_self().value);
      penalised_t penaliseds = penalised_t(get_self(), get_self().value);
      tool_t tools = tool_t(get_self(), get_self().value);
      randnumber_t randnumbers = randnumber_t(get_self(), get_self().value);
      land_t lands = land_t(get_self(), get_self().value);
};