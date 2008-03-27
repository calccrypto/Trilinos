//@HEADER
/*
************************************************************************

              Isorropia: Partitioning and Load Balancing Package
                Copyright (2006) Sandia Corporation

Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
license for use of this work by or on behalf of the U.S. Government.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA
Questions? Contact Alan Williams (william@sandia.gov)
                or Erik Boman    (egboman@sandia.gov)

************************************************************************
*/
//@HEADER

#include <Isorropia_Exception.hpp>
#include <Isorropia_Utils.hpp>
#include <Isorropia_Epetra.hpp>
#include <Isorropia_EpetraCostDescriber.hpp>
#include <Isorropia_EpetraPartitioner.hpp>

#ifdef HAVE_EPETRA
#include <Epetra_Map.h>
#include <Epetra_Import.h>
#include <Epetra_Vector.h>
#include <Epetra_Comm.h>
#include <Epetra_IntVector.h>
#include <Epetra_CrsGraph.h>
#include <Epetra_CrsMatrix.h>
#include <Epetra_RowMatrix.h>
#include <Epetra_LinearProblem.h>
#endif

#if defined(HAVE_EPETRA) && defined(HAVE_MPI)
#include <Epetra_MpiComm.h>
#endif

#ifdef HAVE_MPI
#include <mpi.h>
#endif

/** Isorropia is the namespace that contains isorropia's declarations
  for classes and functions.
*/
namespace Isorropia {
namespace Epetra {

#ifdef HAVE_EPETRA
Teuchos::RefCountPtr<Partitioner>
create_partitioner(Teuchos::RefCountPtr<const Epetra_CrsGraph> input_graph,
		   const Teuchos::ParameterList& paramlist)
{
  Teuchos::RefCountPtr<Partitioner> partitioner =
    Teuchos::rcp(new Partitioner(input_graph, paramlist));
  return(partitioner);
}

Teuchos::RefCountPtr<Partitioner>
create_partitioner(Teuchos::RefCountPtr<const Epetra_CrsGraph> input_graph,
		   Teuchos::RefCountPtr<CostDescriber> costs,
		   const Teuchos::ParameterList& paramlist)
{
  Teuchos::RefCountPtr<Partitioner> partitioner =
    Teuchos::rcp(new Partitioner(input_graph, costs, paramlist));
  return(partitioner);
}

Teuchos::RefCountPtr<Partitioner>
create_partitioner(Teuchos::RefCountPtr<const Epetra_RowMatrix> input_matrix,
		   const Teuchos::ParameterList& paramlist)
{
  Teuchos::RefCountPtr<Partitioner> partitioner =
    Teuchos::rcp(new Partitioner(input_matrix, paramlist));
  return(partitioner);
}

Teuchos::RefCountPtr<Partitioner>
create_partitioner(Teuchos::RefCountPtr<const Epetra_RowMatrix> input_matrix,
		   Teuchos::RefCountPtr<CostDescriber> costs,
		   const Teuchos::ParameterList& paramlist)
{
  Teuchos::RefCountPtr<Partitioner> partitioner =
    Teuchos::rcp(new Partitioner(input_matrix, costs, paramlist));
  return(partitioner);
}

Teuchos::RefCountPtr<Epetra_Map>
create_target_map(const Epetra_Comm& comm, Partitioner& partitioner)
{
  if (!partitioner.partitioning_already_computed()) {
    partitioner.compute_partitioning();
  }

  int myPID = comm.MyPID();
  int numMyElements = partitioner.numElemsInPartition(myPID);
  std::vector<int> myElements(numMyElements);
  partitioner.elemsInPartition(myPID, &myElements[0], numMyElements);

  Teuchos::RefCountPtr<Epetra_Map> target_map =
    Teuchos::rcp(new Epetra_Map(-1, numMyElements, &myElements[0], 0, comm));

  return(target_map);
}

Epetra_Vector* create_row_weights_nnz(const Epetra_RowMatrix& input_matrix)
{
  const Epetra_BlockMap& input_rowmap = input_matrix.RowMatrixRowMap();
  Epetra_Vector* weights = new Epetra_Vector(input_rowmap);
  double* weights_ptr = 0;
  weights->ExtractView(&weights_ptr);
  int local_num_rows = input_rowmap.NumMyElements();

  for(int i=0; i<local_num_rows; ++i) {
    int nnz;
    int err = input_matrix.NumMyRowEntries(i, nnz);
    if (err != 0) {
      throw Isorropia::Exception("create_row_weights_nnz: error in input_matrix.NumMyRowEntries");
    }

    weights_ptr[i] = 1.0*nnz;
  }

  return( weights );
}

Epetra_Vector* create_row_weights_nnz(const Epetra_CrsGraph& input_graph)
{
  const Epetra_BlockMap& input_rowmap = input_graph.RowMap();
  Epetra_Vector* weights = new Epetra_Vector(input_rowmap);
  double* weights_ptr = 0;
  weights->ExtractView(&weights_ptr);
  int local_num_rows = input_rowmap.NumMyElements();

  for(int i=0; i<local_num_rows; ++i) {
    int nnz = input_graph.NumMyIndices(i);

    weights_ptr[i] = 1.0*nnz;
  }

  return( weights );
}

double compute_imbalance(int nprocs, std::vector<int> &offsets, double *wgts, double target)
{
  double imbalance = 1.0;

  for (int p=0; p < nprocs; p++){

    double pweight = 0.0;

    for (int row=offsets[p]; row < offsets[p+1]; row++){
      pweight += wgts[row];
    }

    double ib = 1.0;

    if (pweight <= target)
      ib += ((target - pweight) / target);
    else
      ib += ((pweight - target) / target);

    if (ib > imbalance) imbalance = ib;
  }

  return imbalance;
}

int
repartition(const Epetra_BlockMap& input_map,
	    const Epetra_Vector& weights,
	    std::vector<int>& myNewElements,
            std::map<int,int>& exports,
            std::map<int,int>& imports)
{
  if (!input_map.PointSameAs(weights.Map())) {
    std::string str1("Epetra::repartition ERROR, input_map not ");
    std::string str2("equivalent size/layout to weights.Map()");
    throw Isorropia::Exception(str1+str2);
  }

  const Epetra_Comm& input_comm = input_map.Comm();

  exports.clear();
  imports.clear();

  //first we're going to collect weights onto proc 0.
  int myPID = input_comm.MyPID();
  int numProcs = input_comm.NumProc();
  int global_num_rows = input_map.NumGlobalElements();
  int local_num_rows = myPID == 0 ? global_num_rows : 0;
  Epetra_BlockMap proc0_rowmap(global_num_rows, local_num_rows, 1,0,input_comm);
  Epetra_Vector proc0_weights(proc0_rowmap);

  Epetra_Import importer(proc0_rowmap, input_map);
  proc0_weights.Import(weights, importer, Insert);

  double total_weight;
  weights.Norm1(&total_weight);

  std::vector<int> all_proc_old_offsets;
  gather_all_proc_global_offsets(input_map, all_proc_old_offsets);
  std::vector<int> all_proc_new_offsets(numProcs+1);

  if (myPID == 0) {
    double avg_weight = total_weight/numProcs;

    double* proc0_weights_ptr;
    proc0_weights.ExtractView(&proc0_weights_ptr);
    int weights_length = proc0_weights.MyLength();

    double old_imbalance = 
      compute_imbalance(numProcs, all_proc_old_offsets, proc0_weights_ptr, avg_weight);

    int offset = 0;
    for(int p=0; p<numProcs; ++p) {
      all_proc_new_offsets[p] = offset;

      double tmp_weight = 0.0;

      while(offset < weights_length && tmp_weight < avg_weight) {
        tmp_weight += proc0_weights_ptr[offset++];
      }
    }
    all_proc_new_offsets[numProcs] = weights_length;

    double new_imbalance = 
      compute_imbalance(numProcs, all_proc_new_offsets, proc0_weights_ptr, avg_weight);

    // Because this is a quick and dirty partitioning, it is possible that
    // if the balance was good to begin with, that we have just created a
    // slightly worse balance.  In that case, return to the old partitioning.

    if (new_imbalance > old_imbalance){
      for (int proc=0; proc <= numProcs; proc++){
        all_proc_new_offsets[proc] = all_proc_old_offsets[proc];
      }
    }

#ifdef HAVE_MPI
    if (numProcs > 1) {
      //now broadcast the new offsets
      input_comm.Broadcast(&all_proc_new_offsets[0], numProcs+1, 0);
    }
#endif
  }
  else { //myPID != 0
#ifdef HAVE_MPI
    input_comm.Broadcast(&all_proc_new_offsets[0], numProcs+1, 0);
#endif
  }

  //Now we need to figure out which elements we need to send/recv
  //to/from neighboring processors.

  std::vector<int> send_info;
  std::vector<int> recv_info;
  Isorropia::Utils::create_comm_plan(myPID, all_proc_old_offsets,
                                     all_proc_new_offsets,
                                     send_info, recv_info);

  //Create the list to hold local elements for the new map.
  int new_num_local = all_proc_new_offsets[myPID+1]-all_proc_new_offsets[myPID];
  myNewElements.resize(new_num_local);

  const int* old_gids = input_map.MyGlobalElements();

#ifdef HAVE_MPI
  int tag = 1212121;
  const Epetra_MpiComm* mpiComm =
    dynamic_cast<const Epetra_MpiComm*>(&input_comm);
  if (mpiComm == 0) {
    throw Isorropia::Exception("dynamic_cast to MpiComm failed.");
  }
  int num_reqs = recv_info.size()/3;
  MPI_Comm mpicomm = mpiComm->GetMpiComm();
  MPI_Request* reqs = num_reqs > 0 ? new MPI_Request[num_reqs] : 0;
  MPI_Status* sts = num_reqs > 0 ? new MPI_Status[num_reqs] : 0;

  unsigned i=0;
  while(i<recv_info.size()) {
    int proc = recv_info[i];
    int recv_offset = recv_info[i+1];
    int num_recv = recv_info[i+2];
    
    MPI_Irecv(&myNewElements[recv_offset], num_recv, MPI_INT, proc,
             tag, mpicomm, &reqs[i/3]);
    i += 3;
  }

  i=0;
  while(i<send_info.size()) {
    int proc = send_info[i];
    int send_offset = send_info[i+1];
    int num_send = send_info[i+2];

    MPI_Send((void*)&old_gids[send_offset], num_send, MPI_INT,
             proc, tag, mpicomm);

    for(int j=0; j<num_send; ++j) {
      exports[old_gids[send_offset+j]] = proc;
    }

    i += 3;
  }
#endif

  //copy any overlapping elements from old_gids into myNewElements.
  int old_start = all_proc_old_offsets[myPID];
  int new_start = all_proc_new_offsets[myPID];
  int old_end = all_proc_old_offsets[myPID+1]-1;
  int new_end = all_proc_new_offsets[myPID+1]-1;

  int overlap_start = new_start > old_start ? new_start : old_start;
  int overlap_end = new_end < old_end ? new_end : old_end;

  int copy_src_offset = overlap_start - old_start;
  int copy_dest_offset = overlap_start - new_start;

  int num_copy = overlap_end - overlap_start + 1;

  for(int j=0; j<num_copy; ++j) {
    myNewElements[copy_dest_offset++] = old_gids[copy_src_offset++];
  }

#ifdef HAVE_MPI
  //make sure the recvs are finished...
  if (recv_info.size() > 0) {
    MPI_Waitall(recv_info.size()/3, reqs, sts);

    unsigned i=0;
    while(i<recv_info.size()) {
      int proc = recv_info[i];
      int recv_offset = recv_info[i+1];
      int num_recv = recv_info[i+2];

      for(int j=0; j<num_recv; ++j) {
	imports[myNewElements[recv_offset+j]] = proc;
      }

      i += 3;
    }
  }
  delete [] reqs;
  delete [] sts;
#endif

  return(0);
}

void gather_all_proc_global_offsets(const Epetra_BlockMap& blkmap,
                                    std::vector<int>& all_proc_offsets)
{
  const Epetra_Comm& comm = blkmap.Comm();
  int numProcs = comm.NumProc();
  int myPID = comm.MyPID();

  all_proc_offsets.resize(numProcs+1);
  for(int i=0; i<numProcs+1; ++i) {
    all_proc_offsets[0] = 0;
  }

  //first put num-local-elements in position myPID, and gather-all so
  //that each proc has all entries.
  all_proc_offsets[myPID] = blkmap.NumMyElements();
  comm.GatherAll(&all_proc_offsets[myPID], &all_proc_offsets[0], 1);

  //now run the list and turn the local-sizes into global offsets.
  int offset = 0;
  for(int p=0; p<numProcs; ++p) {
    int tmp = all_proc_offsets[p];
    all_proc_offsets[p] = offset;
    offset += tmp;
  }
  all_proc_offsets[numProcs] = offset;
}

Teuchos::RefCountPtr<Epetra_RowMatrix>
create_balanced_copy(const Epetra_RowMatrix& input_matrix)
{
  CostDescriber costs; 
  Teuchos::ParameterList paramlist;

  Teuchos::RefCountPtr<Epetra_RowMatrix> balanced_matrix =
    create_balanced_copy(input_matrix, costs, paramlist);

  return balanced_matrix;
}

Teuchos::RefCountPtr<Epetra_RowMatrix>
create_balanced_copy(const Epetra_RowMatrix& input_matrix,
		     const Teuchos::ParameterList& paramlist)
{
  CostDescriber costs; 

  Teuchos::RefCountPtr<Epetra_RowMatrix> balanced_matrix =
    create_balanced_copy(input_matrix, costs, paramlist);

  return balanced_matrix;
}

Teuchos::RefCountPtr<Epetra_RowMatrix>
create_balanced_copy(const Epetra_RowMatrix& input_matrix,
                     CostDescriber &costs,
		     const Teuchos::ParameterList& paramlist)
{
  Teuchos::RefCountPtr<const Epetra_RowMatrix> matrixPtr=
    Teuchos::rcp(&(input_matrix), false);

  Teuchos::RefCountPtr<CostDescriber> costPtr =
    Teuchos::rcp(&(costs), false);

  Teuchos::RefCountPtr<Partitioner> partitioner =
    Teuchos::rcp(new Partitioner(matrixPtr, costPtr, paramlist));

  const Epetra_Comm& comm = input_matrix.RowMatrixRowMap().Comm();

  Teuchos::RefCountPtr<Epetra_Map> bal_rowmap;
  try {
    bal_rowmap = create_target_map(comm, *partitioner);
  }
  catch(std::exception& exc) {
    std::string str1("create_balanced_copy: caught exception: ");
    std::string str2(exc.what());
    throw Isorropia::Exception(str1+str2);
  }

  //next, create a new Epetra_RowMatrix (which will be the return-value of
  //this function) with the new row-distribution.
  Teuchos::RefCountPtr<Epetra_RowMatrix> balanced_matrix =
    redistribute_rows(input_matrix, *bal_rowmap);

  return balanced_matrix;
}

Teuchos::RefCountPtr<Epetra_CrsMatrix>
create_balanced_copy(const Epetra_CrsMatrix& input_matrix)
{
  CostDescriber costs; 
  Teuchos::ParameterList paramlist;

  Teuchos::RefCountPtr<Epetra_CrsMatrix> balanced_matrix =
    create_balanced_copy(input_matrix, costs, paramlist);

  return balanced_matrix;
}

Teuchos::RefCountPtr<Epetra_CrsMatrix>
create_balanced_copy(const Epetra_CrsMatrix& input_matrix,
		     const Teuchos::ParameterList& paramlist)
{
  CostDescriber costs; 

  Teuchos::RefCountPtr<Epetra_CrsMatrix> balanced_matrix =
    create_balanced_copy(input_matrix, costs, paramlist);

  return balanced_matrix;
}

Teuchos::RefCountPtr<Epetra_CrsMatrix>
create_balanced_copy(const Epetra_CrsMatrix& input_matrix,
                     CostDescriber &costs,
		     const Teuchos::ParameterList& paramlist)
{
  Teuchos::RefCountPtr<const Epetra_CrsGraph> input_graph =
    Teuchos::rcp(&(input_matrix.Graph()), false);

  Teuchos::RefCountPtr<CostDescriber> costPtr =
    Teuchos::rcp(&(costs), false);

  Teuchos::RefCountPtr<Partitioner> partitioner =
    Teuchos::rcp(new Partitioner(input_graph, costPtr, paramlist));

  const Epetra_Comm& comm = input_graph->RowMap().Comm();

  Teuchos::RefCountPtr<Epetra_Map> bal_rowmap;
  try {
    bal_rowmap = create_target_map(comm, *partitioner);
  }
  catch(std::exception& exc) {
    std::string str1("create_balanced_copy: caught exception: ");
    std::string str2(exc.what());
    throw Isorropia::Exception(str1+str2);
  }

  //next, create a new Epetra_CrsMatrix (which will be the return-value of
  //this function) with the new row-distribution.
  Teuchos::RefCountPtr<Epetra_CrsMatrix> balanced_matrix =
    redistribute_rows(input_matrix, *bal_rowmap);

  return balanced_matrix;
}

Teuchos::RefCountPtr<Epetra_CrsGraph>
create_balanced_copy(const Epetra_CrsGraph& input_graph)
{
  CostDescriber costs; 
  Teuchos::ParameterList paramlist;

  Teuchos::RefCountPtr<Epetra_CrsGraph> balanced_graph =
    create_balanced_copy(input_graph, costs, paramlist);

  return balanced_graph;
}

Teuchos::RefCountPtr<Epetra_CrsGraph>
create_balanced_copy(const Epetra_CrsGraph& input_graph,
		     const Teuchos::ParameterList& paramlist)
{
  CostDescriber costs; 

  Teuchos::RefCountPtr<Epetra_CrsGraph> balanced_graph =
    create_balanced_copy(input_graph, costs, paramlist);

  return balanced_graph;
}

Teuchos::RefCountPtr<Epetra_CrsGraph>
create_balanced_copy(const Epetra_CrsGraph& input_graph,
                     CostDescriber &costs,
		     const Teuchos::ParameterList& paramlist)
{
  Teuchos::RefCountPtr<const Epetra_CrsGraph> graphPtr=
    Teuchos::rcp(&(input_graph), false);

  Teuchos::RefCountPtr<CostDescriber> costPtr =
    Teuchos::rcp(&(costs), false);

  Teuchos::RefCountPtr<Partitioner> partitioner =
    Teuchos::rcp(new Partitioner(graphPtr, costPtr, paramlist));

  const Epetra_Comm& comm = input_graph.RowMap().Comm();

  Teuchos::RefCountPtr<Epetra_Map> bal_rowmap;
  try {
    bal_rowmap = create_target_map(comm, *partitioner);
  }
  catch(std::exception& exc) {
    std::string str1("create_balanced_copy: caught exception: ");
    std::string str2(exc.what());
    throw Isorropia::Exception(str1+str2);
  }

  //next, create a new Epetra_CrsGraph (which will be the return-value of
  //this function) with the new row-distribution.
  Teuchos::RefCountPtr<Epetra_CrsGraph> balanced_graph =
    redistribute_rows(input_graph, *bal_rowmap);

  return balanced_graph;
}

Teuchos::RefCountPtr<Epetra_LinearProblem>
create_balanced_copy(const Epetra_LinearProblem& input_problem)
{
  CostDescriber costs; 
  Teuchos::ParameterList paramlist;

  Teuchos::RefCountPtr<Epetra_LinearProblem> linprob =
    create_balanced_copy(input_problem, costs, paramlist);

  return linprob;
}

Teuchos::RefCountPtr<Epetra_LinearProblem>
create_balanced_copy(const Epetra_LinearProblem& input_problem,
		     const Teuchos::ParameterList& paramlist)
{
  CostDescriber costs; 

  Teuchos::RefCountPtr<Epetra_LinearProblem> linprob =
    create_balanced_copy(input_problem, costs, paramlist);

  return linprob;
}

Teuchos::RefCountPtr<Epetra_LinearProblem>
create_balanced_copy(const Epetra_LinearProblem& input_problem,
                     CostDescriber &costs,
		     const Teuchos::ParameterList& paramlist)
{
  Teuchos::RefCountPtr<const Epetra_RowMatrix> rowmat =
    Teuchos::rcp(input_problem.GetMatrix(), false);

  Teuchos::RefCountPtr<CostDescriber> costPtr =
    Teuchos::rcp(&(costs), false);

  Teuchos::RefCountPtr<Partitioner> partitioner =
    Teuchos::rcp(new Partitioner(rowmat, costPtr, paramlist));

  const Epetra_Comm& comm = rowmat->RowMatrixRowMap().Comm();

  Teuchos::RefCountPtr<Epetra_Map> bal_rowmap;
  try {
    bal_rowmap = create_target_map(comm, *partitioner);
  }
  catch(std::exception& exc) {
    std::string str1("create_balanced_copy: caught exception: ");
    std::string str2(exc.what());
    throw Isorropia::Exception(str1+str2);
  }

  Teuchos::RCP<Epetra_CrsMatrix> A;
  Teuchos::RCP<Epetra_MultiVector> x;
  Teuchos::RCP<Epetra_MultiVector> b;

  try {
    A = redistribute_rows(*input_problem.GetMatrix(), *bal_rowmap);
    x = redistribute(*input_problem.GetLHS(), *bal_rowmap);
    b = redistribute(*input_problem.GetRHS(), *bal_rowmap);
  }
  catch(std::exception& exc) {
    std::string str1("create_balanced_copy(Epetra_LinearProblem): caught exception:");
    std::string str2(exc.what());
    throw Isorropia::Exception(str1+str2);
  }

  // Problem: A, x, and b are reference counted pointers.  The objects they
  // point to get deallocated at the return from this function.  So I need 
  // to make copies of the objects they point to, and use those copies to 
  // create the Epetra_LinearProblem.

  Epetra_CrsMatrix *A2 = new Epetra_CrsMatrix(*A);
  Epetra_MultiVector *x2 = new Epetra_MultiVector(*x);
  Epetra_MultiVector *b2 = new Epetra_MultiVector(*b);

  Teuchos::RefCountPtr<Epetra_LinearProblem> linprob =
    Teuchos::rcp(new Epetra_LinearProblem(A2, x2, b2));

  return( linprob );
}

Teuchos::RefCountPtr<Epetra_CrsMatrix>
redistribute_rows(const Epetra_CrsMatrix& input_matrix,
                  const Epetra_Map& target_rowmap,
                  Epetra_Import* importer)
{
  Epetra_Import* new_importer = 0;
  if (importer == 0) {
    new_importer = new Epetra_Import(target_rowmap, input_matrix.RowMap());
    importer = new_importer;
  }

  Teuchos::RefCountPtr<Epetra_CrsMatrix> target_matrix =
    Teuchos::rcp(new Epetra_CrsMatrix(Copy, target_rowmap, 0));

  target_matrix->Import(input_matrix, *importer, Insert);

  //it is safe to delete new_importer even if it is NULL
  delete new_importer;

  if (!target_matrix->Filled()) {
    Epetra_Map newDomainMap(input_matrix.NumGlobalCols(), 0, input_matrix.Comm());
    target_matrix->FillComplete(newDomainMap, target_rowmap);
  }

  return(target_matrix);
}

Teuchos::RefCountPtr<Epetra_CrsMatrix>
redistribute_rows(const Epetra_RowMatrix& input_matrix,
                  const Epetra_Map& target_rowmap,
                  Epetra_Import* importer)
{
  Epetra_Import* new_importer = 0;
  if (importer == 0) {
    new_importer = new Epetra_Import(target_rowmap, input_matrix.RowMatrixRowMap());
    importer = new_importer;
  }

  Teuchos::RefCountPtr<Epetra_CrsMatrix> target_matrix =
    Teuchos::rcp(new Epetra_CrsMatrix(Copy, target_rowmap, 0));

  target_matrix->Import(input_matrix, *importer, Insert);

  //it is safe to delete new_importer even if it is NULL
  delete new_importer;

  if (!target_matrix->Filled()) {
    Epetra_Map newDomainMap(input_matrix.NumGlobalCols(), 0, input_matrix.Comm());
    target_matrix->FillComplete(newDomainMap, target_rowmap);
  }

  return(target_matrix);
}

Teuchos::RefCountPtr<Epetra_CrsGraph>
redistribute_rows(const Epetra_CrsGraph& input_graph,
                  const Epetra_Map& target_rowmap,
                  Epetra_Import* importer)
{
  Epetra_Import* new_importer = 0;
  if (importer == 0) {
    new_importer = new Epetra_Import(target_rowmap, input_graph.RowMap());
    importer = new_importer;
  }

  Teuchos::RefCountPtr<Epetra_CrsGraph> target_graph =
    Teuchos::rcp(new Epetra_CrsGraph(Copy, target_rowmap, 0));

  target_graph->Import(input_graph, *importer, Insert);

  //it is safe to delete new_importer even if it is NULL
  delete new_importer;

  if (!target_graph->Filled()) {
    Epetra_Map newDomainMap(input_graph.NumGlobalCols(), 0, input_graph.Comm());
    target_graph->FillComplete(newDomainMap, target_rowmap);
  }

  return(target_graph);
}

Teuchos::RefCountPtr<Epetra_MultiVector>
redistribute(const Epetra_MultiVector& input,
             const Epetra_BlockMap& target_map,
             Epetra_Import* importer)
{
  Epetra_Import* new_importer = 0;
  if (importer == 0) {
    new_importer = new Epetra_Import(target_map, input.Map());
    importer = new_importer;
  }

  Teuchos::RefCountPtr<Epetra_MultiVector> target_multivec =
    Teuchos::rcp(new Epetra_MultiVector(target_map, input.NumVectors(), false));

  target_multivec->Import(input, *importer, Insert);

  //it is safe to delete new_importer even if it is NULL
  delete new_importer;

  return(target_multivec);
}

Teuchos::RefCountPtr<Epetra_Vector>
redistribute(const Epetra_Vector& input,
             const Epetra_Map& target_map,
             Epetra_Import* importer)
{
  Epetra_Import* new_importer = 0;
  if (importer == 0) {
    new_importer = new Epetra_Import(target_map, input.Map());
    importer = new_importer;
  }

  Teuchos::RefCountPtr<Epetra_Vector> target_vec =
    Teuchos::rcp(new Epetra_Vector(target_map, false));

  target_vec->Import(input, *importer, Insert);

  //it is safe to delete new_importer even if it is NULL
  delete new_importer;

  return(target_vec);
}

#endif //HAVE_EPETRA

}//namespace Epetra
}//namespace Isorropia

