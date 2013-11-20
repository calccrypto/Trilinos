#include <stk_search/Box.hpp>
#include <stk_search/CoarseSearch.hpp>
#include <stk_search/BoundingBox.hpp>
#include <stk_search/IdentProc.hpp>

#include <stk_util/unit_test_support/stk_utest_macros.hpp>

#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <sstream>
#include <fstream>

namespace std {
template <typename Ident, typename Proc>
std::ostream & operator<<(std::ostream & out, std::pair<stk::search::IdentProc<Ident,Proc>,stk::search::IdentProc<Ident,Proc> > const& ip)
{
  return out << "[" << ip.first << ":" << ip.second << "]";
}
} // namespace std


namespace {

void testCoarseSearchForAlgorithm(stk::search::SearchMethod algorithm, MPI_Comm comm)
{
  typedef stk::search::Point<double> Point;
  typedef stk::search::Box<double> Box;
  typedef stk::search::IdentProc<int,int> Ident;
  typedef std::vector<std::pair<Ident,Ident> > SearchResults;
  typedef std::vector< std::pair<Box,Ident> > BoxVector;

  int num_procs = stk::parallel_machine_size(comm);
  int proc_id   = stk::parallel_machine_rank(comm);

  BoxVector local_domain, local_range;
  // what if identifier is NOT unique

  Box box;
  Ident id;

  box = Box( Point(proc_id + 0.1, 0.0, 0.0), Point(proc_id + 0.9, 1.0, 1.0));
  id = Ident(proc_id * 4, proc_id);
  local_domain.push_back(std::make_pair(box,id));

  box = Box( Point(proc_id + 0.1, 2.0, 0.0), Point(proc_id + 0.9, 3.0, 1.0));
  id = Ident(proc_id * 4+1, proc_id);
  local_domain.push_back(std::make_pair(box,id));

  box = Box( Point(proc_id + 0.6, 0.5, 0.0), Point(proc_id + 1.4, 1.5, 1.0));
  id = Ident(proc_id * 4+2, proc_id);
  local_range.push_back(std::make_pair(box,id));

  box = Box( Point(proc_id + 0.6, 2.5, 0.0), Point(proc_id + 1.4, 3.5, 1.0));
  id = Ident(proc_id * 4+3, proc_id);
  local_range.push_back(std::make_pair(box,id));

  SearchResults searchResults;

  stk::search::coarse_search(local_domain, local_range, algorithm, comm, searchResults);

  if (num_procs == 1) {
    STKUNIT_ASSERT_EQ( searchResults.size(), 2u);
    STKUNIT_EXPECT_EQ( searchResults[0], std::make_pair( Ident(0,0), Ident(2,0)) );
    STKUNIT_EXPECT_EQ( searchResults[1], std::make_pair( Ident(1,0), Ident(3,0)) );
  }
  else {
    if (proc_id == 0) {
      STKUNIT_ASSERT_EQ( searchResults.size(), 4u);
      STKUNIT_EXPECT_EQ( searchResults[0], std::make_pair( Ident(0,0), Ident(2,0)) );
      STKUNIT_EXPECT_EQ( searchResults[1], std::make_pair( Ident(1,0), Ident(3,0)) );
      STKUNIT_EXPECT_EQ( searchResults[2], std::make_pair( Ident(4,1), Ident(2,0)) );
      STKUNIT_EXPECT_EQ( searchResults[3], std::make_pair( Ident(5,1), Ident(3,0)) );
    }
    else if (proc_id == num_procs - 1) {
      STKUNIT_ASSERT_EQ( searchResults.size(), 4u);
      int prev = proc_id -1;
      STKUNIT_EXPECT_EQ( searchResults[0], std::make_pair( Ident(proc_id*4,proc_id), Ident(prev*4+2,prev)) );
      STKUNIT_EXPECT_EQ( searchResults[1], std::make_pair( Ident(proc_id*4,proc_id), Ident(proc_id*4+2,proc_id)) );
      STKUNIT_EXPECT_EQ( searchResults[2], std::make_pair( Ident(proc_id*4+1,proc_id), Ident(prev*4+3,prev)) );
      STKUNIT_EXPECT_EQ( searchResults[3], std::make_pair( Ident(proc_id*4+1,proc_id), Ident(proc_id*4+3,proc_id)) );
    }
    else {
      STKUNIT_ASSERT_EQ( searchResults.size(), 6u);
      int prev = proc_id -1;
      int next = proc_id + 1;
      STKUNIT_EXPECT_EQ( searchResults[0], std::make_pair( Ident(proc_id*4,proc_id), Ident(prev*4+2,prev)) );
      STKUNIT_EXPECT_EQ( searchResults[1], std::make_pair( Ident(proc_id*4,proc_id), Ident(proc_id*4+2,proc_id)) );
      STKUNIT_EXPECT_EQ( searchResults[2], std::make_pair( Ident(proc_id*4+1,proc_id), Ident(prev*4+3,prev)) );
      STKUNIT_EXPECT_EQ( searchResults[3], std::make_pair( Ident(proc_id*4+1,proc_id), Ident(proc_id*4+3,proc_id)) );
      STKUNIT_EXPECT_EQ( searchResults[4], std::make_pair( Ident(next*4,next), Ident(proc_id*4+2,proc_id)) );
      STKUNIT_EXPECT_EQ( searchResults[5], std::make_pair( Ident(next*4+1,next), Ident(proc_id*4+3,proc_id)) );
    }
  }
}

STKUNIT_UNIT_TEST(stk_search, coarse_search_boost_rtree)
{
  testCoarseSearchForAlgorithm(stk::search::BOOST_RTREE, MPI_COMM_WORLD);
}

STKUNIT_UNIT_TEST(stk_search, coarse_search_octree)
{
  testCoarseSearchForAlgorithm(stk::search::OCTREE, MPI_COMM_WORLD);
}

#if 0
STKUNIT_UNIT_TEST(stk_search, coarse_search_one_point)
{
  typedef stk::search::IdentProc<uint64_t, unsigned> Ident;
  typedef stk::search::box::AxisAlignedBoundingBox<double> Box;
  typedef std::vector<std::pair<Box,Ident> > BoxVector;
  typedef std::vector<std::pair<Ident,Ident> > SearchResults;

  stk::ParallelMachine comm = MPI_COMM_WORLD;
  //int num_procs = stk::parallel_machine_size(comm);
  int proc_id   = stk::parallel_machine_rank(comm);

  double data[6];

  BoxVector local_domain, local_range;
  // what if identifier is NOT unique
  // x_min <= x_max
  // y_min <= y_max
  // z_min <= z_max

  data[0] = 0.0; data[1] = 0.0; data[2] = 0.0;
  data[3] = 1.0; data[4] = 1.0; data[5] = 1.0;

  // One bounding box on processor 0 with the label:  0
  // All other processors have empty domain.
  Ident domainBox1(0, 0);
  if (proc_id == 0) {
    local_domain.push_back(Box(data, domainBox1));
  }

  data[0] = 0.5; data[1] = 0.5; data[2] = 0.5;
  data[3] = 0.5; data[4] = 0.5; data[5] = 0.5;

  // One range target on processor 0 with the label:  1
  // All other processors have empty range.
  Ident rangeBox1(1, 0);
  if (proc_id == 0) {
    local_range.push_back(Box(data, rangeBox1));
  }

  SearchResults searchResults;

  stk::search::coarse_search(local_domain, local_range, stk::search::OCTREE, comm, searchResults);

  if (proc_id == 0) {
    STKUNIT_ASSERT_EQ(searchResults.size(), 1u);
    STKUNIT_EXPECT_EQ(searchResults[0], std::make_pair(domainBox1, rangeBox1));
  } else {
    STKUNIT_ASSERT_EQ(searchResults.size(), 0u);
  }
}

void printToCubit(std::ostream& out, const double* data)
{
    std::vector < geometry::Vec3d > corners;
    corners.push_back(geometry::Vec3d(data[0], data[1], data[2]));
    corners.push_back(geometry::Vec3d(data[3], data[1], data[2]));
    corners.push_back(geometry::Vec3d(data[3], data[4], data[2]));
    corners.push_back(geometry::Vec3d(data[0], data[4], data[2]));
    corners.push_back(geometry::Vec3d(data[0], data[1], data[5]));
    corners.push_back(geometry::Vec3d(data[3], data[1], data[5]));
    corners.push_back(geometry::Vec3d(data[3], data[4], data[5]));
    corners.push_back(geometry::Vec3d(data[0], data[4], data[5]));

    corners[0].PrintToCubit(out);
    out << "# {v0 = Id(\"vertex\")}" << std::endl;
    corners[1].PrintToCubit(out);
    out << "# {v1 = Id(\"vertex\")}" << std::endl;
    corners[2].PrintToCubit(out);
    out << "# {v2 = Id(\"vertex\")}" << std::endl;
    corners[3].PrintToCubit(out);
    out << "# {v3 = Id(\"vertex\")}" << std::endl;
    corners[4].PrintToCubit(out);
    out << "# {v4 = Id(\"vertex\")}" << std::endl;
    corners[5].PrintToCubit(out);
    out << "# {v5 = Id(\"vertex\")}" << std::endl;
    corners[6].PrintToCubit(out);
    out << "# {v6 = Id(\"vertex\")}" << std::endl;
    corners[7].PrintToCubit(out);
    out << "# {v7 = Id(\"vertex\")}" << std::endl;

    out << "create surface vertex {v0} {v1} {v2} {v3}" << std::endl;
    out << "create surface vertex {v0} {v4} {v5} {v1}" << std::endl;
    out << "create surface vertex {v1} {v5} {v6} {v2}" << std::endl;
    out << "create surface vertex {v3} {v2} {v6} {v7}" << std::endl;
    out << "create surface vertex {v0} {v3} {v7} {v4}" << std::endl;
    out << "create surface vertex {v4} {v7} {v6} {v5}" << std::endl;
}

STKUNIT_UNIT_TEST(stk_search_not_boost, checkCuts)
{
    typedef stk::search::box::AxisAlignedBoundingBox<Ident, double, 3> Box;
    typedef std::vector<Box> BoxVector;

    MPI_Comm comm = MPI_COMM_WORLD;
    int proc_id = -1;
    int num_procs = -1;
    MPI_Comm_rank(comm, &proc_id);
    MPI_Comm_size(comm, &num_procs);

    double offsetFromEdgeOfProcessorBoundary=0.1;
    double sizeOfDomainPerProcessor=1.0;
    double boxSize=0.8;
    ASSERT_TRUE(offsetFromEdgeOfProcessorBoundary<=sizeOfDomainPerProcessor-offsetFromEdgeOfProcessorBoundary);
    double min=offsetFromEdgeOfProcessorBoundary;
    double max=boxSize+offsetFromEdgeOfProcessorBoundary+1;

    if(num_procs >= 4)
    {
        double data[6];
        data[0] = offsetFromEdgeOfProcessorBoundary;
        data[1] = offsetFromEdgeOfProcessorBoundary;
        data[2] = offsetFromEdgeOfProcessorBoundary;
        data[3] = boxSize+offsetFromEdgeOfProcessorBoundary;
        data[4] = boxSize+offsetFromEdgeOfProcessorBoundary;
        data[5] = boxSize+offsetFromEdgeOfProcessorBoundary;
        if (proc_id == 1)
        {
            data[0] += sizeOfDomainPerProcessor;
            data[3] += sizeOfDomainPerProcessor;
        }
        else if (proc_id == 3)
        {
            data[1] += sizeOfDomainPerProcessor;
            data[4] += sizeOfDomainPerProcessor;
        }
        else if (proc_id == 2)
        {
            data[0] += sizeOfDomainPerProcessor;
            data[1] += sizeOfDomainPerProcessor;
            data[3] += sizeOfDomainPerProcessor;
            data[4] += sizeOfDomainPerProcessor;
        }

        std::stringstream os;
        os << "cubitFile_" << proc_id << ".jou";
        std::ofstream file(os.str().c_str());
        printToCubit(file, data);

        BoxVector local_domain;
        Ident domainBox(proc_id, proc_id);
        if ( proc_id < 4 )
        {
            local_domain.push_back(Box(data, domainBox));
        }

        BoxVector local_range;
        local_range = local_domain;

        std::vector<float> global_box(6);
        stk::search::box_global_bounds(comm,
            local_domain.size(),
            &local_domain[0] ,
            local_range.size(),
            &local_range[0],
            &global_box[0]);

        std::vector<double> globalBoxDouble(global_box.begin(), global_box.end());
        printToCubit(file, &globalBoxDouble[0]);

        float tolerance = 2*std::numeric_limits<float>::epsilon();
        EXPECT_NEAR(float(min), global_box[0], tolerance);
        EXPECT_NEAR(float(min), global_box[1], tolerance);
        EXPECT_NEAR(float(min), global_box[2], tolerance);
        EXPECT_NEAR(float(max), global_box[3], tolerance);
        EXPECT_NEAR(float(max), global_box[4], tolerance);
        EXPECT_NEAR(float(boxSize+offsetFromEdgeOfProcessorBoundary), global_box[5], tolerance);

        typedef std::map< stk::OctTreeKey, std::pair< std::list< Box >, std::list< Box > > > SearchTree ;

        SearchTree searchTree;

        bool local_violations = true;
        unsigned Dim = 3;
        stk::search::createSearchTree(&global_box[0], local_domain.size(), &local_domain[0],
                local_range.size(), &local_range[0], Dim, proc_id, local_violations,
                searchTree);

        const int tree_depth = 4;
        // unsigned maxOffsetForTreeDepthThree = stk::oct_tree_size(tree_depth-1);
        MPI_Barrier(MPI_COMM_WORLD);

        for (int procCounter = 0; procCounter < num_procs; procCounter++)
        {
            if(proc_id == procCounter)
            {
                std::cerr << "\t Nate:=====================================\n";
                std::cerr << "\t Nate: proc_id = " << procCounter << std::endl;
                for(SearchTree::const_iterator i = searchTree.begin(); i != searchTree.end(); ++i)
                {
                    const stk::OctTreeKey & key = (*i).first;

                    // EXPECT_EQ(0u, (stk::oct_tree_offset(tree_depth, key) - 1) % maxOffsetForTreeDepthThree);

                    const std::list<Box> & domain = (*i).second.first;
                    const std::list<Box> & range = (*i).second.second;
                    std::cerr << "\t Nate: depth = " << key.depth() << std::endl;
                    std::cerr << "\t Nate: ordinal = " << stk::oct_tree_offset(tree_depth, key) << std::endl;
                    std::cerr << "\t Nate: num_d = " << domain.size() << std::endl;
                    std::cerr << "\t Nate: num_r = " << range.size() << std::endl;
                    std::cerr << "\t Nate: key = " << key << std::endl;
                }
                std::cerr << "\t Nate:=====================================\n";
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        file.close();
        unlink(os.str().c_str()); // comment this out to view Cubit files for bounding boxes

        const double tol = 0.001 ;

        std::vector< stk::OctTreeKey > cuts ;

        stk::search::oct_tree_partition( comm , searchTree , tol , cuts );

        if ( proc_id == 0 )
        {
            std::cerr << "Nate: For proc size of " << num_procs << std::endl;
            for (int i=0;i<num_procs;i++)
            {
              std::cerr << "Nate: cuts[" << i << "] = " << cuts[i] <<"\t with ordinal = " << stk::oct_tree_offset(tree_depth, cuts[i]) << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "WARNING: Test not setup for anything other than 4 processors; ran with " << num_procs << "." << std::endl;
    }
}

void checkSearchResults(const int proc_id, SearchResults &goldResults, SearchResults& searchResults)
{
    ASSERT_TRUE(searchResults.size() >= goldResults.size());

    Ident unUsedBox1(987654321, 1000000);
    Ident unUsedBox2(987654322, 1000000);
    size_t numResultsMatchingGoldResults = 0;

    for (size_t i = 0; i < searchResults.size(); i++ )
    {
//        if ( proc_id == 0 )
//        {
//            std::cerr << searchResults[i].first << "\t" << searchResults[i].second << std::endl;
//        }
        for (size_t j = 0; j < goldResults.size(); j++)
        {
            if ( searchResults[i] == goldResults[j] )
            {
                goldResults[j] = std::make_pair(unUsedBox1, unUsedBox2);
                numResultsMatchingGoldResults++;
            }
        }
    }
    EXPECT_EQ(goldResults.size(), numResultsMatchingGoldResults) << "proc id = " << proc_id << std::endl;
}

void testCoarseSearchAABBForAlgorithm(stk::search::SearchMethod algorithm, MPI_Comm comm)
{
    typedef stk::search::box::AxisAlignedBoundingBox<Ident, double, 3> Box;
    typedef std::vector<Box> BoxVector;

    int num_procs = stk::parallel_machine_size(comm);
    int proc_id   = stk::parallel_machine_rank(comm);

    double data[6];

    BoxVector local_domain, local_range;
    // what if identifier is NOT unique
    // x_min <= x_max
    // y_min <= y_max
    // z_min <= z_max

    data[0] = proc_id + 0.1; data[1] = 0.0; data[2] = 0.0;
    data[3] = proc_id + 0.9; data[4] = 1.0; data[5] = 1.0;

    Ident domainBox1(proc_id*4, proc_id);
    local_domain.push_back(Box(data, domainBox1));

    data[0] = proc_id + 0.1; data[1] = 2.0; data[2] = 0.0;
    data[3] = proc_id + 0.9; data[4] = 3.0; data[5] = 1.0;

    Ident domainBox2(proc_id*4+1, proc_id);
    local_domain.push_back(Box(data, domainBox2));

    data[0] = proc_id + 0.6; data[1] = 0.5; data[2] = 0.0;
    data[3] = proc_id + 1.4; data[4] = 1.5; data[5] = 1.0;

    Ident rangeBox1(proc_id*4+2, proc_id);
    local_range.push_back(Box(data, rangeBox1));

    data[0] = proc_id + 0.6; data[1] = 2.5; data[2] = 0.0;
    data[3] = proc_id + 1.4; data[4] = 3.5; data[5] = 1.0;

    Ident rangeBox2(proc_id*4+3, proc_id);
    local_range.push_back(Box(data, rangeBox2));

    SearchResults searchResults;

    stk::search::coarse_search(local_domain, local_range, algorithm, comm, searchResults);
    SearchResults goldResults;

    if (num_procs == 1) {
        goldResults.push_back(std::make_pair(domainBox1, rangeBox1));
        goldResults.push_back(std::make_pair(domainBox2, rangeBox2));

        checkSearchResults(proc_id, goldResults, searchResults);
    }
    else {
        if (proc_id == 0) {
            Ident domainBox1OnProcessor1(4,1);
            Ident domainBox2OnProcessor1(5,1);
            goldResults.push_back(std::make_pair(domainBox1, rangeBox1));
            goldResults.push_back(std::make_pair(domainBox1OnProcessor1, rangeBox1));
            goldResults.push_back(std::make_pair(domainBox2, rangeBox2));
            goldResults.push_back(std::make_pair(domainBox2OnProcessor1, rangeBox2));

            checkSearchResults(proc_id, goldResults, searchResults);
        }
        else if (proc_id == num_procs - 1) {
            Ident rangeBox1OnPreviousProcessor1((proc_id-1)*4 + 2, proc_id - 1);
            Ident rangeBox2OnPreviousProcessor1((proc_id-1)*4 + 3, proc_id - 1);

            goldResults.push_back(std::make_pair(domainBox1, rangeBox1OnPreviousProcessor1));
            goldResults.push_back(std::make_pair(domainBox1, rangeBox1));
            goldResults.push_back(std::make_pair(domainBox2, rangeBox2OnPreviousProcessor1));
            goldResults.push_back(std::make_pair(domainBox2, rangeBox2));

            checkSearchResults(proc_id, goldResults, searchResults);
        }
        else {
            Ident rangeBox1OnPreviousProcessor((proc_id-1)*4 + 2, proc_id - 1);
            Ident rangeBox2OnPreviousProcessor((proc_id-1)*4 + 3, proc_id - 1);
            Ident domainBox1OnNextProcessor((proc_id+1)*4,     proc_id + 1);
            Ident domainBox2OnNextProcessor((proc_id+1)*4 + 1, proc_id + 1);

            goldResults.push_back(std::make_pair(domainBox1, rangeBox1OnPreviousProcessor));
            goldResults.push_back(std::make_pair(domainBox1, rangeBox1));
            goldResults.push_back(std::make_pair(domainBox2, rangeBox2OnPreviousProcessor));
            goldResults.push_back(std::make_pair(domainBox2, rangeBox2));
            goldResults.push_back(std::make_pair(domainBox1OnNextProcessor, rangeBox1));
            goldResults.push_back(std::make_pair(domainBox2OnNextProcessor, rangeBox2));

            checkSearchResults(proc_id, goldResults, searchResults);
        }
    }
}

bool compare(const std::pair<geometry::AxisAlignedBB, geometry::AxisAlignedBB> &gold,
             const std::pair<geometry::AxisAlignedBB, geometry::AxisAlignedBB> &result)
{
    bool retVal = false;
    if ( gold.first.get_x_min() == result.first.get_x_min() &&
         gold.first.get_y_min() == result.first.get_y_min() &&
         gold.first.get_z_min() == result.first.get_z_min() )
    {
         if (gold.second.get_x_min() == result.second.get_x_min() &&
             gold.second.get_y_min() == result.second.get_y_min() &&
             gold.second.get_z_min() == result.second.get_z_min() )
        {
            retVal = true;
        }
    }
    return retVal;
}


void checkSearchResults(const int proc_id, const std::vector<std::pair<geometry::AxisAlignedBB, geometry::AxisAlignedBB> > &goldResults,
        const std::vector<std::pair<geometry::AxisAlignedBB, geometry::AxisAlignedBB> > &searchResults)
{
    ASSERT_EQ(goldResults.size(), searchResults.size());
    for (size_t i=0; i<goldResults.size();i++)
    {
        EXPECT_TRUE(compare(goldResults[i], searchResults[i])) << " failed for processor " << proc_id << " for interaction " << i << std::endl;
    }
}
#endif
} //namespace
