/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 *  This can be compiled on all library release versions with:
 *      h5cc tests/t_ref_region.c
 *
 *  This will add a referenced dataset to the existing test file FILENAME.
 *  --for library release 1.12 and above, it will create with the revised reference type
 *      "Add_revised_ref_region"
 *  --for library releaes 1.10 and below, it will create with the old reference type
 *      "Add_old_ref_region"
 */

#include "hdf5.h"
#include <stdlib.h>

/* The test file */
#define FILENAME "ref_compat.h5"    

int main(int argc, char *argv[])
{
    hid_t fid = -1;		/* File ID */
    hid_t did1 = -1, did2 = -1;     /* Dataset ID */
    hid_t sid1 = -1, sid2 = -1;	    /* Dataspace ID */
    hsize_t dims1[] = {2}; /* Dimension size */
    int i;              /* Local index variable */
    int ret;            /* Return status */
    unsigned major, minor, release;     /* Library release versions */

    hsize_t start[1];     /* Starting location of hyperslab */
    hsize_t stride[1];    /* Stride of hyperslab */
    hsize_t count[1];     /* Element count of hyperslab */
    hsize_t block[1];     /* Block size of hyperslab */

    hsize_t coord1[10][1]; /* Coordinates for point selection */

#if defined H5_USE_114_API_DEFAULT || H5_USE_112_API_DEFAULT
    H5R_ref_t *wbuf;    /* buffer to write to disk          */
#else
    hdset_reg_ref_t *wbuf;
#endif

    /* Get the library release versions */
    H5get_libversion(&major, &minor, &release);

    /* Open the test file */
    fid = H5Fopen(FILENAME, H5F_ACC_RDWR, H5P_DEFAULT);

    /* Create dataspace */
    sid1 = H5Screate_simple(1, dims1, NULL);

    /* Set up to select 15 2x1 hyperslabs for first reference */
    start[0]  = 2;
    stride[0] = 5;
    count[0]  = 15;
    block[0]  = 2;

    /* Set up to select sequence of ten points for second reference */
    coord1[0][0] = 16;
    coord1[1][0] = 22;
    coord1[2][0] = 38;
    coord1[3][0] = 41;
    coord1[4][0] = 52;
    coord1[5][0] = 63;
    coord1[6][0] = 70;
    coord1[7][0] = 89;
    coord1[8][0] = 97;
    coord1[9][0] = 03;

#if defined H5_USE_114_API_DEFAULT || defined H5_USE_112_API_DEFAULT

    wbuf = calloc(sizeof(H5R_ref_t), 2);

    /* Create a dataset with revised reference type */
    did1 = H5Dcreate2(fid, "Add_revised_ref_region", H5T_STD_REF, sid1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /* Create references */
    did2 = H5Dopen2(fid, "/Group/Dataset", H5P_DEFAULT);
    sid2 = H5Dget_space(did2);

    /* Select 15 2x1 hyperslabs for first reference */
    ret = H5Sselect_hyperslab(sid2, H5S_SELECT_SET, start, stride, count, block);

    /* Store first dataset region */
    ret = H5Rcreate_region(fid, "/Group/Dataset", sid2, H5P_DEFAULT, &wbuf[0]);

    /* Select sequence of ten points for second reference */
    ret = H5Sselect_elements(sid2, H5S_SELECT_SET, (size_t)10, (const hsize_t *)coord1);

    /* Store second dataset region */
    ret = H5Rcreate_region(fid, "/Group/Dataset", sid2, H5P_DEFAULT, &wbuf[1]);

    /* Write selection to the referenced dataset */
    ret = H5Dwrite(did1, H5T_STD_REF, H5S_ALL, H5S_ALL, H5P_DEFAULT, wbuf);

   /* Destroy references */
   ret = H5Rdestroy(&wbuf[0]);
   ret = H5Rdestroy(&wbuf[1]);

#else

    wbuf = calloc(sizeof(hdset_reg_ref_t),  2);

    /* Create a dataset with old reference type */
    did1 = H5Dcreate1(fid, "Add_old_ref_reg", H5T_STD_REF_DSETREG, sid1, H5P_DEFAULT);

    /* Create references */
    did2 = H5Dopen1(fid, "/Group/Dataset");
    sid2 = H5Dget_space(did2);

    /* Select 15 2x1 hyperslabs for first reference */
    ret = H5Sselect_hyperslab(sid2, H5S_SELECT_SET, start, stride, count, block);

    /* Store first dataset region */
    ret = H5Rcreate(&wbuf[0], fid, "/Group/Dataset", H5R_DATASET_REGION, sid2);

    /* Select sequence of ten points for second reference */
    ret = H5Sselect_elements(sid2, H5S_SELECT_SET, (size_t)10, (const hsize_t *)coord1);

    /* Store second dataset region */
    ret = H5Rcreate(&wbuf[1], fid, "/Group/Dataset", H5R_DATASET_REGION, sid2);

    /* Write selection to the referenced dataset */
    ret = H5Dwrite(did1, H5T_STD_REF_DSETREG, H5S_ALL, H5S_ALL, H5P_DEFAULT, wbuf);

#endif

    /* Closing */
    H5Dclose(did1);
    H5Dclose(did2);
    H5Sclose(sid1);
    H5Sclose(sid2);
    H5Fclose(fid);

    if(wbuf) free(wbuf);

    return 0;
}
