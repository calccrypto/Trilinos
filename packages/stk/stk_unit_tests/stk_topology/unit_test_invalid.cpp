// Copyright (c) 2013, Sandia Corporation.
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
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
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
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

#include <gtest/gtest.h>
#include <stk_topology/topology.hpp>
#include <stk_ngp_test/ngp_test.hpp>
#include "topology_test_utils.hpp"

TEST( stk_topology, invalid_topology)
{
  stk::topology t = stk::topology::INVALID_TOPOLOGY;

  EXPECT_FALSE(t.is_valid());
  EXPECT_FALSE(t.has_homogeneous_faces());
  EXPECT_FALSE(t.is_shell());

  EXPECT_EQ(t.rank(),stk::topology::INVALID_RANK);
  EXPECT_EQ(t.side_rank(),stk::topology::INVALID_RANK);


  EXPECT_EQ(t.dimension(),0u);
  EXPECT_EQ(t.num_nodes(),0u);
  EXPECT_EQ(t.num_vertices(),0u);
  EXPECT_EQ(t.num_edges(),0u);
  EXPECT_EQ(t.num_faces(),0u);
  EXPECT_EQ(t.num_permutations(),0u);
  EXPECT_EQ(t.num_positive_permutations(),0u);

  EXPECT_FALSE(t.defined_on_spatial_dimension(1));
  EXPECT_FALSE(t.defined_on_spatial_dimension(2));
  EXPECT_FALSE(t.defined_on_spatial_dimension(3));

  EXPECT_EQ(t.base(),stk::topology::INVALID_TOPOLOGY);

  EXPECT_EQ(t.face_topology(0), stk::topology::INVALID_TOPOLOGY);
}

void check_invalid_on_device()
{
  Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int i)
  {
    stk::topology t = stk::topology::INVALID_TOPOLOGY;

    NGP_EXPECT_FALSE(t.is_valid());
    NGP_EXPECT_FALSE(t.has_homogeneous_faces());
    NGP_EXPECT_FALSE(t.is_shell());

    NGP_EXPECT_EQ(t.rank(),stk::topology::INVALID_RANK);
    NGP_EXPECT_EQ(t.side_rank(),stk::topology::INVALID_RANK);

    NGP_EXPECT_EQ(t.dimension(),0u);
    NGP_EXPECT_EQ(t.num_nodes(),0u);
    NGP_EXPECT_EQ(t.num_vertices(),0u);
    NGP_EXPECT_EQ(t.num_edges(),0u);
    NGP_EXPECT_EQ(t.num_faces(),0u);
    NGP_EXPECT_EQ(t.num_permutations(),0u);
    NGP_EXPECT_EQ(t.num_positive_permutations(),0u);

    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(1));
    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(2));
    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(3));

    NGP_EXPECT_EQ(t.base(),stk::topology::INVALID_TOPOLOGY);

    NGP_EXPECT_EQ(t.face_topology(0), stk::topology::INVALID_TOPOLOGY);
  });
}

NGP_TEST(stk_topology_ngp, invalid_topology)
{
  check_invalid_on_device();
}
