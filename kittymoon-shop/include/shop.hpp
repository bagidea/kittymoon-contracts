#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

#include <string>
#include <vector>
#include <map>
#include <cmath>

#include <atomicassets-interface.hpp>

using namespace eosio;
using namespace std;
using namespace atomicassets;

static constexpr name ASSETS_ACCOUNT = "atomicassets"_n;

CONTRACT shop : public contract {
   public:
      using contract::contract;

      shop(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds) {}

      ACTION setconfig(
         name     CORE_TOKEN_ACCOUNT,
         symbol   CORE_TOKEN_SYMBOL,
         name     ASSETS_COLLECTION_NAME,
         name     CORE_GAME_ACCOUNT,
         uint32_t price_energy_per_token
      );

      ACTION addprod(
         int32_t  product_template_id,
         string   product_name,
         name     schema_name,
         asset    price,
         uint32_t burn,
         uint32_t retire
      );

      ACTION updateprod(
         int32_t  product_template_id,
         string   product_name,
         name     schema_name,
         asset    price,
         uint32_t burn,
         uint32_t retire
      );

      ACTION delprod(uint32_t template_id);

      [[eosio::on_notify("*::transfer")]]
      void on_receive_token(
         name   from,
         name   to,
         asset  quantity,
         string memo
      );

      uint64_t math_pow(
         uint64_t base,
         uint8_t exp
      );

   private:
      TABLE config_init {
         name     CORE_TOKEN_ACCOUNT      = "kittentokens"_n;
         symbol   CORE_TOKEN_SYMBOL       = symbol("KITTEN", 8);
         name     ASSETS_COLLECTION_NAME  = "kittymoonnft"_n;
         name     CORE_GAME_ACCOUNT       = "kittengamepl"_n;
         uint32_t price_energy_per_token  = 25;
      };

      typedef singleton<"config"_n, config_init> config_t;
      typedef multi_index<"config"_n, config_init> config_t_for_abi;

      TABLE product {
         int32_t  product_template_id;
         string   product_name;
         name     schema_name;
         asset    price;
         uint32_t burn;
         uint32_t retire;

         uint32_t primary_key() const { return product_template_id; }
      };

      typedef multi_index<"products"_n, product> products_t;

      config_t config = config_t(get_self(), get_self().value);
      products_t products = products_t(get_self(), get_self().value);
};