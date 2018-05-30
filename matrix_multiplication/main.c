#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include <math.h>
#include <stdlib.h>

#define R1 2
#define C1 3
#define R2 3
#define C2 2

int main(int argc , char * argv[])
{
    int my_rank;		/* rank of process	*/
    int p;			/* number of process	*/
    int source=0;		/* rank of sender	*/
    int dest;		/* rank of receiver	*/
    int tag = 0;		/* tag for messages	*/
	MPI_Status status;
	MPI_Request request;

    int i,j;

    int *mat1Arr,
        *mat2Arr,
        *arr1,
        *arr2,
        *resultMatArr;

    int **mat1,
        **mat2;

    int mat1Rows,
        mat1Cols,
        mat2Rows,
        mat2Cols,
        portion;

    MPI_Init(&argc , &argv );

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    MPI_Comm_size(MPI_COMM_WORLD, &p);

    MPI_Datatype col_mpi_t;

    MPI_Type_vector(3, 1, 2, MPI_INT, &col_mpi_t);
    MPI_Type_commit(&col_mpi_t);

    if(my_rank==0)
    {

        do{

            printf("\nenter matrix 1 rows number, columns number\n");
            scanf("%d %d",&mat1Rows,&mat1Cols);

            printf("\nenter matrix 2 rows number, columns number\n");
            scanf("%d %d",&mat2Rows,&mat2Cols);

        }while(
               mat1Rows!=mat2Cols &&
               printf("######################################################################\n") &&
               printf("############### Please Enter Valid matrices dimensions ###############\n") &&
               printf("######################################################################\n")
               );


        if(mat1Rows==mat2Cols){
            mat1Arr=malloc(mat1Rows*mat1Cols*sizeof(int));
            mat1=malloc(mat1Rows*sizeof(int*));

            mat2Arr=malloc(mat2Rows*mat2Cols*sizeof(int));
            mat2=malloc(mat2Rows*sizeof(int*));

            resultMatArr=malloc(mat1Rows*mat2Cols*sizeof(int));

            if(mat1Arr!=NULL&&mat1!=NULL && mat2Arr!=NULL&&mat2!=NULL)
            {
                //################## First Matrix ##################
                for(i=0;i<mat1Rows;i++)
                {
                    mat1[i]=&(mat1Arr[i*mat1Cols]);
                }

                int min=1, max=6;

                printf("Creating %d * %d matrix:\n",mat1Rows,mat1Cols);
                for(i=0;i<mat1Rows;i++)
                {
                    for(j=0;j<mat1Cols;j++)
                    {
                        int val=min+(max-min) * (double)rand() / (double)RAND_MAX + 0.5;
                        mat1[i][j]=val;
                    }

                }
                printf("The First Matrix Created Is: \n");
                for(i=0;i<mat1Rows;i++)
                {
                    for(j=0;j<mat1Cols;j++)
                    {
                        printf("%*d ",3,mat1[i][j]);
                    }
                    printf("\n");
                }
                printf("\n");
                //################## End Of First Matrix ############

                //################## Second Matrix ##################

                for(i=0;i<mat2Rows;i++)
                {
                    mat2[i]=&(mat2Arr[i*mat2Cols]);
                }

                //int min=1, max=row2*col2;

                printf("Creating %d * %d matrix:\n",mat2Rows,mat2Cols);
                for(i=0;i<mat2Rows;i++)
                {
                    for(j=0;j<mat2Cols;j++)
                    {
                        int val=min+(max-min) * (double)rand() / (double)RAND_MAX + 0.5;
                        mat2[i][j]=val;
                    }

                }
                printf("The Second Matrix Created Is: \n");
                for(i=0;i<mat2Rows;i++)
                {
                    for(j=0;j<mat2Cols;j++)
                    {
                        printf("%*d ",3,mat2[i][j]);
                    }
                    printf("\n");
                }
                printf("\n");
                //################## End Of second Matrix ############
            }
            else
            {
                printf("\nBAD ALLOCATION\n");
                return 0;
            }
        }
    }



    MPI_Bcast(&mat1Cols,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&mat2Rows,1,MPI_INT,0,MPI_COMM_WORLD);
    //MPI_Bcast(&portion,1,MPI_INT,0,MPI_COMM_WORLD);
    arr1=malloc(mat1Cols*sizeof(int));
    arr2=malloc(mat2Rows*sizeof(int));


    if(my_rank==0)
    {
        dest=0;
        for(i=0;i<mat1Rows;i++)
        for(j=0;j<mat2Cols;j++,dest++)
        {
            MPI_Isend(mat1[i], mat1Cols, MPI_INT,dest,tag,MPI_COMM_WORLD,&request);
            if(dest==0){MPI_Recv(&(arr1[0]), mat1Cols, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);}
            MPI_Wait(&request, &status);

            MPI_Isend(&(mat2[0][j]), 1, col_mpi_t,dest,tag,MPI_COMM_WORLD,&request);
            if(dest==0){MPI_Recv(&(arr2[0]), mat2Rows, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);}
            MPI_Wait(&request, &status);
        }
    }
    else
    {
        MPI_Recv(&(arr1[0]), 3, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&(arr2[0]), 3, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    }

    printf("P%d only has to multiply the array: ",my_rank);
    for(i=0;i<3;i++){printf("%d ",arr1[i]);}
    printf("\n");

    printf("With the array the array: ");
    for(i=0;i<3;i++){printf("%d ",arr2[i]);}
    printf("\n");

    int localDotProduct=0;
    for(i=0;i<3;i++)
    {
        localDotProduct+=(arr1[i]*arr2[i]);
    }

    printf("P%d local dot product: %d\n",my_rank,localDotProduct);



    MPI_Gather(&localDotProduct,1,MPI_INT,resultMatArr,1,MPI_INT,source,MPI_COMM_WORLD);

    if(my_rank==0)
    {
        for(i=0;i<mat1Rows;i++)
        {
            mat1[i]=&(resultMatArr[i*mat2Cols]);
        }

        printf("Result of the multiplication: \n");
        for(i=0;i<mat1Rows;i++)
        {
            for(j=0;j<mat2Cols;j++)
            {
                printf("%*d ",3,mat1[i][j]);
            }
            printf("\n");
        }
        printf("\n");

    }



    MPI_Finalize();
    return 0;
}
















