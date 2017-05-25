/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "h5hltest.h"

/*
 * Extending datasets in WATCH.h5 generated by h5watchgentest.c
 */
#define DSET_ONE "DSET_ONE"
#define DSET_TWO "DSET_TWO"
#define DSET_CMPD "DSET_CMPD"
#define DSET_CMPD_ESC "DSET_CMPD_ESC"
#define DSET_CMPD_TWO "DSET_CMPD_TWO"
#define DSET_ALLOC_LATE "DSET_ALLOC_LATE"
#define DSET_ALLOC_EARLY "DSET_ALLOC_EARLY"

/* The message sent by this process (extend_dset) to the test script to start "h5watch" */
#define WRITER_MESSAGE  "writer_message"
/* The message received from the test script to start extending dataset */
#define READER_MESSAGE  "reader_message"

/* Size of data buffer */
#define TEST_BUF_SIZE 100

static herr_t extend_dset_two(const char *file, char *dname, int action1, int action2);
static herr_t extend_dset_one(const char *file, char *dname, int action);


/* Data structures for datasets with compound data type */
typedef struct sub22_t {
    unsigned int a;
    unsigned int b;
    unsigned int c;
} sub22_t;

typedef struct sub2_t {
    unsigned int a;
    sub22_t b;
    unsigned int c;
} sub2_t;

typedef struct sub4_t {
    unsigned int a;
    unsigned int b;
} sub4_t;

typedef struct set_t {
    unsigned int field1;
    sub2_t field2;
    double field3;
    sub4_t field4;
} set_t;

/*
 ***********************************************************************
 *
 * Extending a two-dimensional dataset by action1 and action2.
 *        --action1 and action2 can be a positive # or negative # or 0.
 *  
 ***********************************************************************
 */
static herr_t
extend_dset_two(const char *file, char *dname, int action1, int action2)
{
    hid_t fid = -1;         /* file id                                          */
    hid_t fapl = -1;        /* file access property list id                     */
    hid_t did = -1;         /* dataset id                                       */
    hid_t sid = -1;         /* dataspace id                                     */
    hid_t dtid = -1;        /* dataset's datatype id                            */
    int ndims;              /* # of dimension sizes                             */
    unsigned i;             /* local index variable                             */
    hsize_t ext_dims[2];    /* new dimension sizes after extension              */
    hsize_t cur_dims[2];    /* current dimension sizes                          */
    size_t dtype_size;      /* size of the dataset's datatype                   */
    unsigned num_elmts;     /* number of elements in the dataset                */
    int *ibuf = NULL;       /* buffer for storing retrieved elements (integer)  */
    set_t *cbuf = NULL;     /* buffer for storing retrieved elemnets (compound) */

    /* Allocate memory */
    if(NULL == (ibuf = (int *)HDcalloc(TEST_BUF_SIZE, sizeof(int))))
        goto error;
    if(NULL == (cbuf = (set_t *)HDcalloc(TEST_BUF_SIZE, sizeof(set_t))))
        goto error;

    /* Create a copy of file access property list */
    if((fapl = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        goto error;

    /* Set to use the latest library format */
    if(H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0)
        goto error;

    /* Open the file and dataset with SWMR write */
    if((fid = H5Fopen(file, H5F_ACC_RDWR|H5F_ACC_SWMR_WRITE, fapl)) < 0)
        goto error;

    if((did = H5Dopen2(fid, dname, H5P_DEFAULT)) < 0)
        goto error;

    /* Send message to the test script to start "h5watch" */
    h5_send_message(WRITER_MESSAGE, NULL, NULL);

    if((sid = H5Dget_space(did)) < 0)
        goto error;

    if((ndims = H5Sget_simple_extent_ndims(sid)) < 0)
        goto error;

    /* Get the size of the dataset's datatype */
    if((dtype_size = H5LDget_dset_type_size(did, NULL)) == 0)
        goto error;

    /* Get the dataset's data type */
    if((dtid = H5Tget_native_type(H5Dget_type(did), H5T_DIR_DEFAULT)) < 0)
        goto error;

    /* Wait for message from the test script to start extending dataset */
    if(h5_wait_message(READER_MESSAGE) < 0)
        goto error;

    /* sleep to emulate about 2 seconds of application operation */
    HDsleep(2);

    /* Get current dimension sizes */
    if(H5LDget_dset_dims(did, cur_dims) < 0)
        goto error;

    /* Set up the new extended dimension sizes  */
    ext_dims[0] = cur_dims[0] + (hsize_t)action1;
    ext_dims[1] = cur_dims[1] + (hsize_t)action2;

    /* Extend the dataset */
    if(H5Dset_extent(did, ext_dims) < 0)
        goto error;

    num_elmts = 1;
    for(i = 0; i < (unsigned)ndims; i++)
        num_elmts *= (unsigned)ext_dims[i];

    /* Compound type */
    if(!HDstrcmp(dname, DSET_CMPD_TWO)) {

        HDmemset(cbuf, 0, TEST_BUF_SIZE * sizeof(set_t));
        for(i = 0; i < num_elmts; i++) {
            cbuf[i].field1 = action1;
            cbuf[i].field2.a = action1;
            cbuf[i].field2.c = action1;
            cbuf[i].field2.b.a = action1;
            cbuf[i].field2.b.b = action1;
            cbuf[i].field2.b.c = action1;
            cbuf[i].field3 = action1;
            cbuf[i].field4.a = action1;
            cbuf[i].field4.b = action1;
        } /* end for */

       /* Write to the dataset */
        if(H5Dwrite(did, dtid, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuf) < 0)
            goto error;

    } else { /* Integer type */
        HDmemset(ibuf, 0, TEST_BUF_SIZE * sizeof(int));
        for(i = 0; i < num_elmts; i++)
            ibuf[i] = action1;

        /* Write to the dataset */
        if(H5Dwrite(did, dtid, H5S_ALL, H5S_ALL, H5P_DEFAULT, ibuf) < 0)
            goto error;
    } /* end if-else */

    if(H5Dflush(did) < 0)
        goto error;

    /* Closing */
    if(H5Tclose(dtid) < 0) goto error;
    if(H5Dclose(did) < 0) goto error;
    if(H5Pclose(fapl) < 0) goto error;
    if(H5Fclose(fid) < 0) goto error;

    if(ibuf) HDfree(ibuf);
    if(cbuf) HDfree(cbuf);

    return SUCCEED;

error:
    H5E_BEGIN_TRY
        H5Tclose(dtid);
        H5Dclose(did);
        H5Pclose(fapl);
        H5Fclose(fid);
    H5E_END_TRY

    if(ibuf)
        HDfree(ibuf);
    if(cbuf)
        HDfree(cbuf);

    return FAIL;

} /* end extend_dset_two() */

/*
 ***********************************************************************
 *
 * Extending a one-dimensional dataset by action:
 *        --action can be a positive # or negative # or 0.
 *
 ***********************************************************************
 */
static herr_t
extend_dset_one(const char *file, char *dname, int action)
{
    hid_t fid = -1;         /* file id                                          */
    hid_t fapl = -1;        /* file access property list id                     */
    hid_t did = -1;         /* dataset id                                       */
    hid_t dtid = -1;        /* dataset's datatype id                            */
    hid_t sid = -1;         /* dataspace id                                     */
    hid_t mid = -1;         /* memory space id                                  */
    unsigned i;             /* local index variable                             */
    hsize_t cur_dims[1];    /* current dimension sizes                          */
    hsize_t ext_dims[1];    /* new dimension sizes after extension              */
    hsize_t offset[1];      /* starting offsets of appended data                */
    hsize_t count[1];       /* dimension sizes of appended data                 */
    size_t dtype_size;      /* size of the dataset's datatype                   */
    int *ibuf = NULL;       /* buffer for storing retrieved elements (integer)  */
    set_t *cbuf = NULL;     /* buffer for storing retrieved elemnets (compound) */

    /* Allocate memory */
    if(NULL == (ibuf = (int *)HDcalloc(TEST_BUF_SIZE, sizeof(int))))
        goto error;
    if(NULL == (cbuf = (set_t *)HDcalloc(TEST_BUF_SIZE, sizeof(set_t))))
        goto error;

    /* Create a copy of file access property list */
    if((fapl = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        goto error;

    /* Set to use the latest library format */
    if(H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0)
        goto error;

    /* Open the file and dataset with SWMR write */
    if((fid = H5Fopen(file, H5F_ACC_RDWR|H5F_ACC_SWMR_WRITE, fapl)) < 0)
        goto error;

    /* Send message to the test script to start "h5watch" */
    h5_send_message(WRITER_MESSAGE, NULL, NULL);

    if((did = H5Dopen2(fid, dname, H5P_DEFAULT)) < 0)
        goto error;

    /* Get size of the dataset's datatype */
    if((dtype_size = H5LDget_dset_type_size(did, NULL)) == 0)
        goto error;

    /* Get dataset's datatype */
    if((dtid = H5Tget_native_type(H5Dget_type(did), H5T_DIR_DEFAULT)) < 0)
        goto error;

    /* Wait for message from the test script to start extending dataset */
    if(h5_wait_message(READER_MESSAGE) < 0)
        goto error;

    /* sleep to emulate about 2 seconds of application operation */
    HDsleep(2);

    /* Get current dimension sizes */
    if(H5LDget_dset_dims(did, cur_dims) < 0)
        goto error;

    /* Set up the new extended dimension sizes  */
    ext_dims[0] = cur_dims[0] + (hsize_t)action;

    /* Extend the dataset */
    if(H5Dset_extent(did, ext_dims) < 0)
        goto error;

    /* Write to the new appended region of the dataset */
    if(action > 0) {

        /* Select the extended region */
        offset[0] = cur_dims[0];
        count[0] = (hsize_t)action;
        if((sid = H5Dget_space(did)) < 0)
            goto error;
        if(H5Sselect_hyperslab(sid, H5S_SELECT_SET, offset, NULL, count, NULL) < 0)
            goto error;

        /* Set up memory space and get dataset's datatype */
        if((mid = H5Screate_simple(1, count, NULL)) < 0)
            goto error;

        /* Initialize data for the extended region of the dataset */
        /* Compound type */
        if(!HDstrcmp(dname, DSET_CMPD) || !HDstrcmp(dname, DSET_CMPD_ESC)) {

            HDmemset(cbuf, 0, TEST_BUF_SIZE * sizeof(set_t));
            for(i = 0; i < (unsigned)action; i++) {
                cbuf[i].field1 = i + 1;
                cbuf[i].field2.a = i + 2;
                cbuf[i].field2.b.a = i + 2;
                cbuf[i].field2.b.b = i + 2;
                cbuf[i].field2.b.c = i + 2;
                cbuf[i].field2.c = i + 2;

                cbuf[i].field3 = i + 3;

                cbuf[i].field4.a = i + 4;
                cbuf[i].field4.b = i + 4;
            } /* end for */

            /* Write to the extended region of the dataset */
            if(H5Dwrite(did, dtid, mid, sid, H5P_DEFAULT, cbuf) < 0)
                goto error;
        } else { /* Integer type */

            HDmemset(ibuf, 0, TEST_BUF_SIZE * sizeof(int));
            for(i = 0; i < (unsigned)action; i++)
                ibuf[i] = (int)i;

            /* Write to the extended region of the dataset */
            if(H5Dwrite(did, dtid, mid, sid, H5P_DEFAULT, ibuf) < 0)
                goto error;
        } /* end if-else */

        /* Closing */
        if(H5Sclose(sid) < 0) goto error;
        if(H5Sclose(mid) < 0) goto error;
    } /* end if */

    if(H5Dflush(did) < 0)
        goto error;

    /* Closing */
    if(H5Tclose(dtid) < 0) goto error;
    if(H5Dclose(did) < 0) goto error;
    if(H5Pclose(fapl) < 0) goto error;
    if(H5Fclose(fid) < 0) goto error;

    if(ibuf) HDfree(ibuf);
    if(cbuf) HDfree(cbuf);

    return SUCCEED;

error:
    H5E_BEGIN_TRY
        H5Sclose(sid);
        H5Sclose(mid);
        H5Tclose(dtid);
        H5Dclose(did);
        H5Pclose(fapl);
        H5Fclose(fid);
    H5E_END_TRY

    if(ibuf)
        HDfree(ibuf);
    if(cbuf)
        HDfree(cbuf);

    return FAIL;
} /* end extend_dset_one() */


/*
 ***********************************************************************
 *
 * Usage: extend_dset xx.h5 dname action1 action2
 *        --action1 and action2 can be a positive # or negative # or 0.
 *
 ***********************************************************************
 */
int
main(int argc, const char *argv[])
{
    char *dname = NULL;
    char *fname = NULL;
    int action1, action2;

    if(argc != 5) {
        HDfprintf(stderr, "Should have file name, dataset name, and the extended amount...\n");
        goto error;
    } /* end if */

    /* Get the dataset name to be extended */
    fname = HDstrdup(argv[1]);
    dname = HDstrdup(argv[2]);
    action1 = HDatoi(argv[3]);
    action2 = HDatoi(argv[4]);

    if(!HDstrcmp(dname, DSET_CMPD) || !HDstrcmp(dname, DSET_CMPD_ESC)) {
        if(extend_dset_one(fname, dname, action1) < 0)
            goto error;
    } else if(!HDstrcmp(dname, DSET_ONE) || 
              !HDstrcmp(dname, DSET_ALLOC_LATE) || 
              !HDstrcmp(dname, DSET_ALLOC_EARLY)) {
        if(extend_dset_one(fname, dname, action1) < 0)
            goto error;
    } else if(!HDstrcmp(dname, DSET_TWO) || 
              !HDstrcmp(dname, DSET_CMPD_TWO)) {
        if(extend_dset_two(fname, dname, action1, action2) < 0)
            goto error;
    } else {
        HDfprintf(stdout, "Dataset cannot be extended...\n");
        goto error;
    } /* end if-else */

    HDexit(EXIT_SUCCESS);

error:
    if(dname)
        HDfree(dname);
    if(fname)
        HDfree(fname);
    HDexit(EXIT_FAILURE);
} /* end main() */
