static char help[] = "Example of using graph partitioning to partition a graph\n\n";

/*T
   Concepts: Mat^mat partitioning
   Concepts: Mat^image segmentation
   Processors: n
T*/

#include <petscmat.h>

int main(int argc, char **args)
{
  Mat             A;
  MatPartitioning part;
  IS              is;
  PetscInt        r,N = 10, start, end, *vweights;
  PetscBool       set_vweights=PETSC_FALSE;
  PetscMPIInt     rank;
  PetscErrorCode  ierr;

  ierr = PetscInitialize(&argc, &args, (char*) 0, help);if (ierr) return ierr;
  ierr = PetscOptionsGetInt(NULL,NULL, "-N", &N, NULL);CHKERRQ(ierr);
  ierr = MPI_Comm_rank(PETSC_COMM_WORLD,&rank);CHKERRQ(ierr);
  ierr = MatCreate(PETSC_COMM_WORLD, &A);CHKERRQ(ierr);
  ierr = MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, N, N);CHKERRQ(ierr);
  ierr = MatSetFromOptions(A);CHKERRQ(ierr);
  ierr = MatSeqAIJSetPreallocation(A, 3, NULL);CHKERRQ(ierr);
  ierr = MatMPIAIJSetPreallocation(A, 3, NULL, 2, NULL);CHKERRQ(ierr);
  ierr = PetscOptionsGetBool(NULL,NULL,"-test_vertex_weights",&set_vweights,NULL);CHKERRQ(ierr);
  /* Create a linear mesh */
  ierr = MatGetOwnershipRange(A, &start, &end);CHKERRQ(ierr);
  if (set_vweights) {
    ierr = PetscMalloc1(end-start,&vweights);CHKERRQ(ierr);
    for (r = start; r < end; ++r)
      vweights[r-start] = rank+1;
  }
  for (r = start; r < end; ++r) {
    if (r == 0) {
      PetscInt    cols[2];
      PetscScalar vals[2];

      cols[0] = r;   cols[1] = r+1;
      vals[0] = 1.0; vals[1] = 1.0;

      ierr = MatSetValues(A, 1, &r, 2, cols, vals, INSERT_VALUES);CHKERRQ(ierr);
    } else if (r == N-1) {
      PetscInt    cols[2];
      PetscScalar vals[2];

      cols[0] = r-1; cols[1] = r;
      vals[0] = 1.0; vals[1] = 1.0;

      ierr = MatSetValues(A, 1, &r, 2, cols, vals, INSERT_VALUES);CHKERRQ(ierr);
    } else {
      PetscInt    cols[3];
      PetscScalar vals[3];

      cols[0] = r-1; cols[1] = r;   cols[2] = r+1;
      vals[0] = 1.0; vals[1] = 1.0; vals[2] = 1.0;

      ierr = MatSetValues(A, 1, &r, 3, cols, vals, INSERT_VALUES);CHKERRQ(ierr);
    }
  }
  ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

  ierr = MatPartitioningCreate(PETSC_COMM_WORLD, &part);CHKERRQ(ierr);
  ierr = MatPartitioningSetAdjacency(part, A);CHKERRQ(ierr);
  if (set_vweights) {
    ierr = MatPartitioningSetVertexWeights(part,vweights);CHKERRQ(ierr);
  }
  ierr = MatPartitioningSetFromOptions(part);CHKERRQ(ierr);
  ierr = MatPartitioningApply(part, &is);CHKERRQ(ierr);
  ierr = ISView(is, PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);
  ierr = ISDestroy(&is);CHKERRQ(ierr);
  ierr = MatPartitioningDestroy(&part);CHKERRQ(ierr);

  ierr = MatDestroy(&A);CHKERRQ(ierr);
  ierr = PetscFinalize();
  return ierr;
}


/*TEST

   test:
      nsize: 3
      requires: parmetis
      args: -mat_partitioning_type parmetis

   test:
      suffix: 2
      nsize: 3
      requires: ptscotch
      args: -mat_partitioning_type ptscotch

   test:
      suffix: 3
      nsize: 4
      requires: party
      args: -mat_partitioning_type party

   test:
      suffix: 4
      nsize: 3
      requires: chaco
      args: -mat_partitioning_type chaco

   test:
      suffix: 5
      nsize: 3
      requires: parmetis
      args: -mat_partitioning_type hierarch -mat_partitioning_hierarchical_nfineparts 3 -mat_partitioning_nparts 10 -N 100

   test:
      suffix: 6
      nsize: 3
      requires: parmetis
      args: -mat_partitioning_type hierarch -mat_partitioning_hierarchical_nfineparts 3 -mat_partitioning_nparts 10 -N 100 -test_vertex_weights 1

   test:
      suffix: 7
      nsize: 2
      requires: parmetis
      args: -mat_partitioning_type hierarch -mat_partitioning_hierarchical_nfineparts 2 -mat_partitioning_nparts 10  -mat_partitioning_hierarchical_fineparttype hierarch -malloc_dump -N 100 -mat_partitioning_improve 1
TEST*/
