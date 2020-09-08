// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of NTESS nor the names of its contributors
//       may be used to endorse or promote products derived from this
//       software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#ifndef stk_mesh_GetEntities_hpp
#define stk_mesh_GetEntities_hpp

//----------------------------------------------------------------------

#include <stk_mesh/base/Selector.hpp>   // for Selector
#include <vector>                       // for vector
#include "stk_mesh/base/Types.hpp"      // for BucketVector, EntityRank
namespace stk { namespace mesh { class BulkData; } }
namespace stk { namespace mesh { struct Entity; } }
namespace stk { namespace mesh { class Bucket; } }

//----------------------------------------------------------------------

namespace stk {
namespace mesh {

/** \addtogroup stk_mesh_module
 *  \{
 */

//----------------------------------------------------------------------

/** \brief  Local count selected entities of each type.
 */
void count_entities( const Selector & selector ,
                     const BulkData & mesh ,
                     std::vector<size_t> & count );

/** \brief Get all entities of the specified type, sorted by ID.  */
void get_entities( const BulkData & mesh , EntityRank entity_rank,
                   EntityVector & entities);

/** \brief  Count entities in selected buckets (selected by the
 *          given selector instance), and sorted by ID.
 */
unsigned count_selected_entities( const Selector & selector ,
                                  const BucketVector & input_buckets );

/** \brief  Get entities in selected buckets (selected by the
 *          given selector instance), and sorted by ID.
 */
void get_selected_entities( const Selector & selector ,
                            const BucketVector & input_buckets ,
                            EntityVector & entities ,
                            bool sortByGlobalId = true );

unsigned get_num_entities(const stk::mesh::BulkData &bulk);

/** \} */

} // namespace mesh
} // namespace stk

//----------------------------------------------------------------------
//----------------------------------------------------------------------

#endif //  stk_mesh_GetEntities_hpp
