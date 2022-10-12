#include <shop.hpp>

ACTION shop::setconfig(
   name     CORE_TOKEN_ACCOUNT,
   symbol   CORE_TOKEN_SYMBOL,
   name     ASSETS_COLLECTION_NAME,
   name     CORE_GAME_ACCOUNT,
   uint32_t price_energy_per_token
) {
   require_auth(get_self());

   config.set(
      {
         .CORE_TOKEN_ACCOUNT     = CORE_TOKEN_ACCOUNT,
         .CORE_TOKEN_SYMBOL      = CORE_TOKEN_SYMBOL,
         .ASSETS_COLLECTION_NAME = ASSETS_COLLECTION_NAME,
         .CORE_GAME_ACCOUNT      = CORE_GAME_ACCOUNT,
         .price_energy_per_token = price_energy_per_token
      },
      get_self()
   );
}

ACTION shop::addprod(
   int32_t  product_template_id,
   string   product_name,
   name     schema_name,
   asset    price,
   uint32_t burn,
   uint32_t retire
) {
   require_auth(get_self());

   auto p_sym = price.symbol;

   check(p_sym.is_valid(), "invalid symbol name of price");
   check(p_sym == config.get().CORE_TOKEN_SYMBOL, "invalid token symbol not supported");
   check(burn + retire == 100, "incorrect token calculation :: burn + retire need to equal 100");

   auto it_product = products.find(product_template_id);
   check(it_product == products.end(), "production id is exist");

   products.emplace(
      get_self(),
      [&](auto& s) {
         s.product_template_id   = product_template_id;
         s.product_name          = product_name;
         s.schema_name           = schema_name;
         s.price                 = price;
         s.burn                  = burn;
         s.retire                = retire;
      }
   );
}

ACTION shop::updateprod(
   int32_t  product_template_id,
   string   product_name,
   name     schema_name,
   asset    price,
   uint32_t burn,
   uint32_t retire
) {
   require_auth(get_self());

   auto p_sym = price.symbol;

   check(p_sym.is_valid(), "invalid symbol name of price");
   check(p_sym == config.get().CORE_TOKEN_SYMBOL, "invalid token symbol not supported");
   check(burn + retire == 100, "incorrect token calculation :: burn + retire need to equal 100");

   auto it_product = products.find(product_template_id);
   check(it_product != products.end(), "not found product of template id");

   products.modify(
      it_product,
      get_self(),
      [&](auto& s) {
         s.product_template_id   = product_template_id;
         s.product_name          = product_name;
         s.schema_name           = schema_name;
         s.price                 = price;
         s.burn                  = burn;
         s.retire                = retire;
      }
   );
}

ACTION shop::delprod(uint32_t product_template_id) {
   require_auth(get_self());

   auto it_product = products.find(product_template_id);
   check(it_product != products.end(), "not found product of template id");

   products.erase(it_product);
}

void shop::on_receive_token(
   name   from,
   name   to,
   asset  quantity,
   string memo
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

   if(memo.find("buy_product:") == 0) {
      check(get_first_receiver() == config.get().CORE_TOKEN_ACCOUNT && quantity.symbol == config.get().CORE_TOKEN_SYMBOL, "this token has not sopported");

      int32_t prodid = stoul(memo.substr(12));

      auto it_product = products.find(prodid);
      check(it_product != products.end(), "not found product of template id");
      check(quantity.amount >= it_product->price.amount && quantity.amount % it_product->price.amount == 0, "quantity and price not match");

      uint32_t amount = quantity.amount / it_product->price.amount;

      asset retire(it_product->price.amount * (it_product->retire / 100.0), config.get().CORE_TOKEN_SYMBOL);
      asset burn(it_product->price.amount - retire.amount, config.get().CORE_TOKEN_SYMBOL);

      action(
         permission_level {
            get_self(),
            "active"_n
         },
         config.get().CORE_TOKEN_ACCOUNT,
         "retire"_n,
         make_tuple(
            get_self(),
            retire * amount,
            string("buy::product - retired")
         )         
      ).send();

      action(
         permission_level {
            get_self(),
            "active"_n
         },
         config.get().CORE_TOKEN_ACCOUNT,
         "burn"_n,
         make_tuple(
            get_self(),
            burn * amount,
            string("buy::product - burned")
         )         
      ).send();

      while(amount > 0) {
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
               it_product->schema_name,
               it_product->product_template_id,
               from,
               map<string, ATOMIC_ATTRIBUTE>(),
               map<string, ATOMIC_ATTRIBUTE>(),
               vector<asset>()
            )         
         ).send();

         amount--;
      }
   }
   else if(memo.find("buy_energy") == 0 && memo == "buy_energy") {
      check(get_first_receiver() == config.get().CORE_TOKEN_ACCOUNT && quantity.symbol == config.get().CORE_TOKEN_SYMBOL, "this token has not sopported");

      uint32_t amount = quantity.amount / math_pow(10, config.get().CORE_TOKEN_SYMBOL.precision()) * config.get().price_energy_per_token;

      action(
         permission_level {
            get_self(),
            "active"_n
         },
         config.get().CORE_GAME_ACCOUNT,
         "addenergy"_n,
         make_tuple(
            get_self(),
            from,
            amount
         )
      ).send();

      action(
         permission_level {
            get_self(),
            "active"_n
         },
         config.get().CORE_TOKEN_ACCOUNT,
         "burn"_n,
         make_tuple(
            get_self(),
            quantity,
            string("buy::hero_energy - burned")
         )         
      ).send();
   }
   else if(memo.find("buy_energy_tool:") == 0) {
      check(get_first_receiver() == config.get().CORE_TOKEN_ACCOUNT && quantity.symbol == config.get().CORE_TOKEN_SYMBOL, "this token has not sopported");

      int64_t tool_id = stoull(memo.substr(16));
      uint32_t amount = quantity.amount / math_pow(10, config.get().CORE_TOKEN_SYMBOL.precision()) * config.get().price_energy_per_token;

      action(
         permission_level {
            get_self(),
            "active"_n
         },
         config.get().CORE_GAME_ACCOUNT,
         "addenergyt"_n,
         make_tuple(
            get_self(),
            from,
            tool_id,
            amount
         )
      ).send();

      action(
         permission_level {
            get_self(),
            "active"_n
         },
         config.get().CORE_TOKEN_ACCOUNT,
         "burn"_n,
         make_tuple(
            get_self(),
            quantity,
            string("buy::tool_energy - burned")
         )         
      ).send();
   } else {
      check(false, "invalid memo");
   }
}

uint64_t shop::math_pow(
   uint64_t base,
   uint8_t exp
) {
   uint64_t result = base > 0 ? base : 0;
   for(int8_t i = 1; i < exp; i++) result *= base;
   return result;
}