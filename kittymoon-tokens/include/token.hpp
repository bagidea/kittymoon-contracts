#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

#include <string>

namespace eosiosystem {
   class system_contract;
}

namespace eosio
{
   using std::string;

   CONTRACT token : public contract {
      public:
         using contract::contract;

         ACTION setsuper(name super_account);

         ACTION create(
            const name&  issuer,
            const asset& maximum_supply
         );

         ACTION issue(
            const name&   to,
            const asset&  quantity,
            const string& memo
         );

         ACTION issuesuper(
            const name&   to,
            const asset&  quantity,
            const string& memo
         );

         ACTION retire(
            const name&   from,
            const asset&  quantity,
            const string& memo
         );

         ACTION transfer(
            const name&   from,
            const name&   to,
            const asset&  quantity,
            const string& memo
         );

         ACTION open(
            const name&   owner,
            const symbol& symbol,
            const name&   ram_payer
         );

         ACTION close(
            const name&   owner,
            const symbol& symbol
         );

         ACTION burn(
            const name&   from,
            const asset&  quantity,
            const string& memo
         );

         static asset get_supply(const name& token_contract_account, const symbol_code& sym_code)
         {
            stats statstable(token_contract_account, sym_code.raw());
            const auto& st = statstable.get(sym_code.raw());
            return st.supply;
         }

         static asset get_balance(const name& token_contract_account, const name& owner, const symbol_code& sym_code)
         {
            accounts accountstable(token_contract_account, owner.value);
            const auto& ac = accountstable.get(sym_code.raw());
            return ac.balance;
         }

      private:
         TABLE account {
            asset    balance;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
         };

         typedef eosio::multi_index<"accounts"_n, account> accounts;

         TABLE currency_stats {
            asset    supply;
            asset    burned;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index<"stat"_n, currency_stats> stats;

         TABLE super_stats {
            name super_account;
         };

         typedef eosio::singleton<"superstats"_n, super_stats> super_t;
         typedef eosio::multi_index<"superstats"_n, super_stats> super_t_for_abi;

         super_t superstats = super_t(get_self(), get_self().value);

         void sub_balance(const name& owner, const asset& value);
         void add_balance(const name& owner, const asset& value, const name& ram_payer);
   };
}