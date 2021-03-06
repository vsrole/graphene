/*
 * Copyright (c) 2015, Cryptonomex, Inc.
 * All rights reserved.
 *
 * This source code is provided for evaluation in private test networks only, until September 8, 2015. After this date, this license expires and
 * the code may not be used, modified or distributed for any purpose. Redistribution and use in source and binary forms, with or without modification,
 * are permitted until September 8, 2015, provided that the following conditions are met:
 *
 * 1. The code and/or derivative works are used only for private test networks consisting of no more than 10 P2P nodes.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <graphene/chain/database.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/proposal_object.hpp>

namespace graphene { namespace chain {

bool proposal_object::is_authorized_to_execute(database* db) const
{
   transaction_evaluation_state dry_run_eval(db);
   dry_run_eval._is_proposed_trx = true;
   std::transform(available_active_approvals.begin(), available_active_approvals.end(),
                  std::inserter(dry_run_eval.approved_by, dry_run_eval.approved_by.end()), [](object_id_type id) {
      return make_pair(id, authority::active);
   });
   std::transform(available_owner_approvals.begin(), available_owner_approvals.end(),
                  std::inserter(dry_run_eval.approved_by, dry_run_eval.approved_by.end()), [](object_id_type id) {
      return make_pair(id, authority::owner);
   });

   signed_transaction tmp;
   dry_run_eval._trx = &tmp;
   for( auto key_id : available_key_approvals )
      tmp.signatures[key_id] = fc::ecc::compact_signature();

   //insert into dry_run_eval->_trx.signatures
   //dry_run_eval.signed_by.insert(available_key_approvals.begin(), available_key_approvals.end());

   // Check all required approvals. If any of them are unsatisfied, return false.
   for( const auto& id : required_active_approvals )
      if( !dry_run_eval.check_authority(id(*db), authority::active) )
         return false;
   for( const auto& id : required_owner_approvals )
      if( !dry_run_eval.check_authority(id(*db), authority::owner) )
         return false;

   return true;
}

} } // graphene::chain
