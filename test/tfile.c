/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/***********************************************************
*
* Test program:	 tfile
*
* Test the low-level file I/O features.
*
*************************************************************/

#include "hdf5.h"
#include "testhdf5.h"

#include "H5Bprivate.h"
#include "H5Pprivate.h"

#define F1_USERBLOCK_SIZE  (hsize_t)0
#define F1_OFFSET_SIZE	   sizeof(haddr_t)
#define F1_LENGTH_SIZE	   sizeof(hsize_t)
#define F1_SYM_LEAF_K	   4
#define F1_SYM_INTERN_K	   16
#define FILE1	"tfile1.h5"

#define F2_USERBLOCK_SIZE  (hsize_t)512
#define F2_OFFSET_SIZE	   8
#define F2_LENGTH_SIZE	   8
#define F2_SYM_LEAF_K	   8
#define F2_SYM_INTERN_K	   32
#define F2_RANK            2
#define F2_DIM0            4
#define F2_DIM1            6
#define F2_DSET            "dset"                                                
#define FILE2	"tfile2.h5"

#define F3_USERBLOCK_SIZE  (hsize_t)0
#define F3_OFFSET_SIZE	   F2_OFFSET_SIZE
#define F3_LENGTH_SIZE	   F2_LENGTH_SIZE
#define F3_SYM_LEAF_K	   F2_SYM_LEAF_K
#define F3_SYM_INTERN_K	   F2_SYM_INTERN_K
#define FILE3	"tfile3.h5"

#define GRP_NAME         "/group"
#define DSET_NAME         "dataset"
#define ATTR_NAME          "attr"
#define TYPE_NAME          "type"
#define FILE4	           "tfile4.h5"

#define OBJ_ID_COUNT_0     0
#define OBJ_ID_COUNT_1     1 
#define OBJ_ID_COUNT_2     2
#define OBJ_ID_COUNT_3     3 
#define OBJ_ID_COUNT_4     4 
#define OBJ_ID_COUNT_6	   6    
#define OBJ_ID_COUNT_8     8 

static void
create_objects(hid_t, hid_t, hid_t *, hid_t *, hid_t *, hid_t *);
static void
test_obj_count_and_id(hid_t, hid_t, hid_t, hid_t, hid_t, hid_t);
static void 
check_file_id(hid_t, hid_t);

/****************************************************************
**
**  test_file_create(): Low-level file creation I/O test routine.
** 
****************************************************************/
static void 
test_file_create(void)
{
    hid_t		fid1, fid2, fid3; /* HDF5 File IDs		*/
    hid_t		tmpl1, tmpl2;	/*file creation templates	*/
    hsize_t		ublock;		/*sizeof userblock		*/
    size_t		parm;		/*file-creation parameters	*/
    size_t		parm2;		/*file-creation parameters	*/
    unsigned		iparm;
    unsigned		iparm2;
    herr_t		ret;		/*generic return value		*/

    /* Output message about test being performed */
    MESSAGE(5, ("Testing Low-Level File Creation I/O\n"));

    /* Test create with various sequences of H5F_ACC_EXCL and */
    /* H5F_ACC_TRUNC flags */

    /* Create with H5F_ACC_EXCL */
    /* First ensure the file does not exist */
    remove(FILE1);
    fid1 = H5Fcreate(FILE1, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(fid1, FAIL, "H5Fcreate");

    /*
     * try to create the same file with H5F_ACC_TRUNC. This should fail
     * because fid1 is the same file and is currently open.
     */
    fid2 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    VERIFY(fid2, FAIL, "H5Fcreate");

    /* Close all files */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");
    ret = H5Fclose(fid2);
    VERIFY(ret, FAIL, "H5Fclose"); /*file should not have been open */

    /*
     * Try again with H5F_ACC_EXCL. This should fail because the file already
     * exists from the previous steps.
     */
    fid1 = H5Fcreate(FILE1, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
    VERIFY(fid1, FAIL, "H5Fcreate");

    /* Test create with H5F_ACC_TRUNC. This will truncate the existing file. */
    fid1 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(fid1, FAIL, "H5Fcreate");

    /*
     * Try to truncate first file again. This should fail because fid1 is the
     * same file and is currently open.
     */
    fid2 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    VERIFY(fid2, FAIL, "H5Fcreate");

    /*
     * Try with H5F_ACC_EXCL. This should fail too because the file already
     * exists.
     */
    fid2 = H5Fcreate(FILE1, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
    VERIFY(fid2, FAIL, "H5Fcreate");

    /* Get the file-creation template */
    tmpl1 = H5Fget_create_plist(fid1);
    CHECK(tmpl1, FAIL, "H5Fget_create_plist");

    /* Get the file-creation parameters */
    ret = H5Pget_userblock(tmpl1, &ublock);
    CHECK(ret, FAIL, "H5Pget_userblock");
    VERIFY(ublock, F1_USERBLOCK_SIZE, "H5Pget_userblock");

    ret = H5Pget_sizes(tmpl1, &parm, &parm2);
    CHECK(ret, FAIL, "H5Pget_sizes");
    VERIFY(parm, F1_OFFSET_SIZE, "H5Pget_sizes");
    VERIFY(parm2, F1_LENGTH_SIZE, "H5Pget_sizes");

    ret = H5Pget_sym_k(tmpl1, &iparm, &iparm2);
    CHECK(ret, FAIL, "H5Pget_sym_k");
    VERIFY(iparm, F1_SYM_INTERN_K, "H5Pget_sym_k");
    VERIFY(iparm2, F1_SYM_LEAF_K, "H5Pget_sym_k");

    /* Release file-creation template */
    ret = H5Pclose(tmpl1);
    CHECK(ret, FAIL, "H5Pclose");

#ifdef LATER
    /* Double-check that the atom has been vaporized */
    ret = H5Pclose(tmpl1);
    VERIFY(ret, FAIL, "H5Pclose");
#endif

    /* Create a new file with a non-standard file-creation template */
    tmpl1 = H5Pcreate(H5P_FILE_CREATE);
    CHECK(tmpl1, FAIL, "H5Pnew");

    /* Set the new file-creation parameters */
    ret = H5Pset_userblock(tmpl1, F2_USERBLOCK_SIZE);
    CHECK(ret, FAIL, "H5Pset_userblock");

    ret = H5Pset_sizes(tmpl1, F2_OFFSET_SIZE, F2_LENGTH_SIZE);
    CHECK(ret, FAIL, "H5Pset_sizes");

    ret = H5Pset_sym_k(tmpl1, F2_SYM_INTERN_K, F2_SYM_LEAF_K);
    CHECK(ret, FAIL, "H5Pset_sym_k");

    /*
     * Try to create second file, with non-standard file-creation template
     * params.
     */
    fid2 = H5Fcreate(FILE2, H5F_ACC_TRUNC, tmpl1, H5P_DEFAULT);
    CHECK(fid2, FAIL, "H5Fcreate");

    /* Release file-creation template */
    ret = H5Pclose(tmpl1);
    CHECK(ret, FAIL, "H5Pclose");

    /* Make certain we can create a dataset properly in the file with the userblock */
    {
       hid_t       dataset_id, dataspace_id;  /* identifiers */
       hsize_t     dims[F2_RANK];
       int         data[F2_DIM0][F2_DIM1];
       unsigned i,j;

       /* Create the data space for the dataset. */
       dims[0] = F2_DIM0; 
       dims[1] = F2_DIM1; 
       dataspace_id = H5Screate_simple(F2_RANK, dims, NULL);
       CHECK(dataspace_id, FAIL, "H5Screate_simple");

       /* Create the dataset. */
       dataset_id = H5Dcreate(fid2, F2_DSET, H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT);
       CHECK(dataset_id, FAIL, "H5Dcreate");

       for(i=0; i<F2_DIM0; i++)
           for(j=0; j<F2_DIM1; j++)
               data[i][j]=i*10+j;

       /* Write data to the new dataset */
       ret = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
       CHECK(ret, FAIL, "H5Dwrite");

       /* End access to the dataset and release resources used by it. */
       ret = H5Dclose(dataset_id);
       CHECK(ret, FAIL, "H5Dclose");

       /* Terminate access to the data space. */ 
       ret = H5Sclose(dataspace_id);
       CHECK(ret, FAIL, "H5Sclose");
    }

    /* Get the file-creation template */
    tmpl1 = H5Fget_create_plist(fid2);
    CHECK(tmpl1, FAIL, "H5Fget_create_plist");

    /* Get the file-creation parameters */
    ret = H5Pget_userblock(tmpl1, &ublock);
    CHECK(ret, FAIL, "H5Pget_userblock");
    VERIFY(ublock, F2_USERBLOCK_SIZE, "H5Pget_userblock");

    ret = H5Pget_sizes(tmpl1, &parm, &parm2);
    CHECK(ret, FAIL, "H5Pget_sizes");
    VERIFY(parm, F2_OFFSET_SIZE, "H5Pget_sizes");
    VERIFY(parm2, F2_LENGTH_SIZE, "H5Pget_sizes");

    ret = H5Pget_sym_k(tmpl1, &iparm, &iparm2);
    CHECK(ret, FAIL, "H5Pget_sym_k");
    VERIFY(iparm, F2_SYM_INTERN_K, "H5Pget_sym_k");
    VERIFY(iparm2, F2_SYM_LEAF_K, "H5Pget_sym_k");

    /* Clone the file-creation template */
    tmpl2 = H5Pcopy(tmpl1);
    CHECK(tmpl2, FAIL, "H5Pcopy");

    /* Release file-creation template */
    ret = H5Pclose(tmpl1);
    CHECK(ret, FAIL, "H5Pclose");

    /* Set the new file-creation parameter */
    ret = H5Pset_userblock(tmpl2, F3_USERBLOCK_SIZE);
    CHECK(ret, FAIL, "H5Pset_userblock");

    /*
     * Try to create second file, with non-standard file-creation template
     * params
     */
    fid3 = H5Fcreate(FILE3, H5F_ACC_TRUNC, tmpl2, H5P_DEFAULT);
    CHECK(fid3, FAIL, "H5Fcreate");

    /* Release file-creation template */
    ret = H5Pclose(tmpl2);
    CHECK(ret, FAIL, "H5Pclose");

    /* Get the file-creation template */
    tmpl1 = H5Fget_create_plist(fid3);
    CHECK(tmpl1, FAIL, "H5Fget_create_plist");

    /* Get the file-creation parameters */
    ret = H5Pget_userblock(tmpl1, &ublock);
    CHECK(ret, FAIL, "H5Pget_userblock");
    VERIFY(ublock, F3_USERBLOCK_SIZE, "H5Pget_userblock");

    ret = H5Pget_sizes(tmpl1, &parm, &parm2);
    CHECK(ret, FAIL, "H5Pget_sizes");
    VERIFY(parm, F3_OFFSET_SIZE, "H5Pget_sizes");
    VERIFY(parm2, F3_LENGTH_SIZE, "H5Pget_sizes");

    ret = H5Pget_sym_k(tmpl1, &iparm, &iparm2);
    CHECK(ret, FAIL, "H5Pget_sym_k");
    VERIFY(iparm, F3_SYM_INTERN_K, "H5Pget_sym_k");
    VERIFY(iparm2, F3_SYM_LEAF_K, "H5Pget_sym_k");

    /* Release file-creation template */
    ret = H5Pclose(tmpl1);
    CHECK(ret, FAIL, "H5Pclose");

    /* Close first file */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");

    /* Close second file */
    ret = H5Fclose(fid2);
    CHECK(ret, FAIL, "H5Fclose");

    /* Close third file */
    ret = H5Fclose(fid3);
    CHECK(ret, FAIL, "H5Fclose");
}				/* test_file_create() */

/****************************************************************
**
**  test_file_open(): Low-level file open I/O test routine.
** 
****************************************************************/
static void 
test_file_open(void)
{
    hid_t		fid1, fid2;     /*HDF5 File IDs			*/
    hid_t               did;            /*dataset ID                    */
    hid_t               fapl_id;        /*file access property list ID  */
    hid_t		tmpl1;		/*file creation templates	*/
    hsize_t		ublock;		/*sizeof user block		*/
    size_t		parm;		/*file-creation parameters	*/
    size_t		parm2;		/*file-creation parameters	*/
    unsigned		iparm;
    unsigned		iparm2;
    herr_t		ret;		/*generic return value		*/

    /*
     * Test single file open 
     */
     
    /* Output message about test being performed */
    MESSAGE(5, ("Testing Low-Level File Opening I/O\n"));

    /* Open first file */
    fid1 = H5Fopen(FILE2, H5F_ACC_RDWR, H5P_DEFAULT);
    CHECK(fid1, FAIL, "H5Fopen");

    /* Get the file-creation template */
    tmpl1 = H5Fget_create_plist(fid1);
    CHECK(tmpl1, FAIL, "H5Fget_create_plist");

    /* Get the file-creation parameters */
    ret = H5Pget_userblock(tmpl1, &ublock);
    CHECK(ret, FAIL, "H5Pget_userblock");
    VERIFY(ublock, F2_USERBLOCK_SIZE, "H5Pget_userblock");

    ret = H5Pget_sizes(tmpl1, &parm, &parm2);
    CHECK(ret, FAIL, "H5Pget_sizes");
    VERIFY(parm, F2_OFFSET_SIZE, "H5Pget_sizes");
    VERIFY(parm2, F2_LENGTH_SIZE, "H5Pget_sizes");

    ret = H5Pget_sym_k(tmpl1, &iparm, &iparm2);
    CHECK(ret, FAIL, "H5Pget_sym_k");
    VERIFY(iparm, F2_SYM_INTERN_K, "H5Pget_sym_k");
    VERIFY(iparm2, F2_SYM_LEAF_K, "H5Pget_sym_k");

    /* Release file-creation template */
    ret = H5Pclose(tmpl1);
    CHECK(ret, FAIL, "H5Pclose");

    /* Close first file */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");


    /*
     * Test two file opens: one is opened H5F_ACC_RDONLY and H5F_CLOSE_WEAK.  
     * It's closed with an object left open.  Then another is opened 
     * H5F_ACC_RDWR, which should fail.
     */

    /* Output message about test being performed */
    MESSAGE(5, ("Testing 2 File Openings\n"));

    /* Create file access property list */
    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    CHECK(fapl_id, FAIL, "H5Pcreate");
    
    /* Set file close mode to H5F_CLOSE_WEAK */
    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_WEAK);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* Open file for first time */
    fid1 = H5Fopen(FILE2, H5F_ACC_RDONLY, fapl_id);
    CHECK(fid1, FAIL, "H5Fopen");
    
    /* Open dataset */
    did = H5Dopen(fid1, F2_DSET);
    CHECK(did, FAIL, "H5Dopen");

    /* Close first open */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");

    /* Open file for second time, which should fail. */
    fid2 = H5Fopen(FILE2, H5F_ACC_RDWR, fapl_id);
    VERIFY(fid2, FAIL, "H5Fopen");

    /* Close dataset from first open */
    ret = H5Dclose(did);
    CHECK(ret, FAIL, "H5Dclose");
}   /* test_file_open() */

/****************************************************************
**
**  test_file_close():  low-level file close test routine.  
**                      It mainly tests behavior with close degree.
**
*****************************************************************/
static void 
test_file_close(void)
{
    hid_t               fid1, fid2;
    hid_t               fapl_id, access_id;
    hid_t		dataset_id, group_id1, group_id2, group_id3;
    H5F_close_degree_t  fc_degree;
    herr_t              ret;

    /* Test behavior while opening file multiple times with different
     * file close degree value
     */
    fid1 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(fid1, FAIL, "H5Fcreate");
 
    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    CHECK(fapl_id, FAIL, "H5Pcreate");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    ret = H5Pget_fclose_degree(fapl_id, &fc_degree);
    VERIFY(fc_degree, H5F_CLOSE_STRONG, "H5Pget_fclose_degree");

    /* should fail */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    VERIFY(fid2, FAIL, "H5Fopen");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_DEFAULT);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");
   
    /* should succeed */ 
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    CHECK(fid2, FAIL, "H5Fopen");

    /* Close first open */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");

    /* Close second open */
    ret = H5Fclose(fid2);
    CHECK(ret, FAIL, "H5Fclose");


    /* Test behavior while opening file multiple times with different file 
     * close degree
     */
    fid1 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(fid1, FAIL, "H5Fcreate");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_WEAK);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    ret = H5Pget_fclose_degree(fapl_id, &fc_degree);
    VERIFY(fc_degree, H5F_CLOSE_WEAK, "H5Pget_fclose_degree");

    /* should succeed */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    CHECK(fid2, FAIL, "H5Fopen");

    /* Close first open */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");

    /* Close second open */
    ret = H5Fclose(fid2);
    CHECK(ret, FAIL, "H5Fclose");


    /* Test behavior while opening file multiple times with file close 
     * degree STRONG */
    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    fid1 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
    CHECK(fid1, FAIL, "H5Fcreate");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_WEAK);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* should fail */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    VERIFY(fid2, FAIL, "H5Fopen");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* should succeed */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    CHECK(fid2, FAIL, "H5Fopen");

    /* Create a dataset and a group in each file open respectively */
    create_objects(fid1, fid2, NULL, NULL, NULL, NULL);

    /* Close first open */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");

    /* Close second open */
    ret = H5Fclose(fid2);
    CHECK(ret, FAIL, "H5Fclose");


    /* Test behavior while opening file multiple times with file close
     * degree SEMI */
    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    fid1 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
    CHECK(fid1, FAIL, "H5Fcreate");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_DEFAULT);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* should fail */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    VERIFY(fid2, FAIL, "H5Fopen");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* should succeed */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    CHECK(fid2, FAIL, "H5Fopen");

    /* Create a dataset and a group in each file open respectively */
    create_objects(fid1, fid2, &dataset_id, &group_id1, &group_id2, &group_id3);

    /* Close first open */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");

    /* Close second open, should fail since it is SEMI and objects are 
     * still open. */
    ret = H5Fclose(fid2);
    VERIFY(ret, FAIL, "H5Fclose");

    ret = H5Dclose(dataset_id);
    CHECK(ret, FAIL, "H5Dclose");
  
    ret = H5Gclose(group_id1);
    CHECK(ret, FAIL, "H5Gclose");

    ret = H5Gclose(group_id2);
    CHECK(ret, FAIL, "H5Gclose");

    /* Close second open, should fail since it is SEMI and one group ID is 
     * still open. */
    ret = H5Fclose(fid2);
    VERIFY(ret, FAIL, "H5Fclose");

    ret = H5Gclose(group_id3);
    CHECK(ret, FAIL, "H5Gclose");

    /* Close second open again.  Should succeed. */
    ret = H5Fclose(fid2);
    CHECK(ret, FAIL, "H5Fclose");


    /* Test behavior while opening file multiple times with file close
     * degree WEAK */
    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_WEAK);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    fid1 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
    CHECK(fid1, FAIL, "H5Fcreate");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* should fail */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    VERIFY(fid2, FAIL, "H5Fopen");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_DEFAULT);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* should succeed */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    CHECK(fid2, FAIL, "H5Fopen");

    /* Create a dataset and a group in each file open respectively */
    create_objects(fid1, fid2, &dataset_id, &group_id1, &group_id2, &group_id3);

    /* Create more new files and test object count and ID list functions */
    test_obj_count_and_id(fid1, fid2, dataset_id, group_id1, 
				group_id2, group_id3);
 
    /* Close first open */
    ret = H5Fclose(fid1);
    CHECK(ret, FAIL, "H5Fclose");

    /* Close second open.  File will be finally closed after all objects 
     * are closed. */
    ret = H5Fclose(fid2);
    CHECK(ret, FAIL, "H5Fclose");

    ret = H5Dclose(dataset_id);
    CHECK(ret, FAIL, "H5Dclose");

    ret = H5Gclose(group_id1);
    CHECK(ret, FAIL, "H5Gclose");

    ret = H5Gclose(group_id2);
    CHECK(ret, FAIL, "H5Gclose");

    ret = H5Gclose(group_id3);
    CHECK(ret, FAIL, "H5Gclose");


    /* Test behavior while opening file multiple times with file close
     * degree DEFAULT */
    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_DEFAULT);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    fid1 = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
    CHECK(fid1, FAIL, "H5Fcreate");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* should fail */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    VERIFY(fid2, FAIL, "H5Fopen");

    ret = H5Pset_fclose_degree(fapl_id, H5F_CLOSE_DEFAULT);
    CHECK(ret, FAIL, "H5Pset_fclose_degree");

    /* should succeed */
    fid2 = H5Fopen(FILE1, H5F_ACC_RDWR, fapl_id);
    CHECK(fid2, FAIL, "H5Fopen");

    /* Create a dataset and a group in each file open respectively */
    create_objects(fid1, fid2, &dataset_id, &group_id1, &group_id2, &group_id3);
    
    access_id = H5Fget_access_plist(fid1);
    CHECK(access_id, FAIL, "H5Fget_access_plist");

    ret= H5Pget_fclose_degree(access_id, &fc_degree);
    CHECK(ret, FAIL, "H5Pget_fclose_degree");

    switch(fc_degree) {
	case H5F_CLOSE_STRONG:
    	    /* Close first open */
    	    ret = H5Fclose(fid1);
    	    CHECK(ret, FAIL, "H5Fclose");
    	    /* Close second open */
    	    ret = H5Fclose(fid2);
    	    CHECK(ret, FAIL, "H5Fclose");
	    break;
	case H5F_CLOSE_SEMI:
            /* Close first open */
            ret = H5Fclose(fid1);
            CHECK(ret, FAIL, "H5Fclose");
    	    ret = H5Dclose(dataset_id);
            CHECK(ret, FAIL, "H5Dclose");
            ret = H5Gclose(group_id1);
    	    CHECK(ret, FAIL, "H5Gclose");
            ret = H5Gclose(group_id2);
            CHECK(ret, FAIL, "H5Gclose");
            ret = H5Gclose(group_id3);
            CHECK(ret, FAIL, "H5Gclose");
            /* Close second open */
            ret = H5Fclose(fid2);
            CHECK(ret, FAIL, "H5Fclose");
	    break;
	case H5F_CLOSE_WEAK:
            /* Close first open */
            ret = H5Fclose(fid1);
            CHECK(ret, FAIL, "H5Fclose");
            /* Close second open */
            ret = H5Fclose(fid2);
            CHECK(ret, FAIL, "H5Fclose");
            ret = H5Dclose(dataset_id);
            CHECK(ret, FAIL, "H5Dclose");
            ret = H5Gclose(group_id1);
            CHECK(ret, FAIL, "H5Gclose");
            ret = H5Gclose(group_id2);
            CHECK(ret, FAIL, "H5Gclose");
            ret = H5Gclose(group_id3);
            CHECK(ret, FAIL, "H5Gclose");
	    break;
        default:
            CHECK(fc_degree, H5F_CLOSE_DEFAULT, "H5Pget_fclose_degree");
            break;
    }

    /* Close file access property list */
    ret = H5Pclose(fapl_id);
    CHECK(ret, FAIL, "H5Pclose");
    ret = H5Pclose(access_id);
    CHECK(ret, FAIL, "H5Pclose");
}

/****************************************************************
**
**  create_objects(): routine called by test_file_close to create
**                    a dataset and a group in file.
**
****************************************************************/
static void
create_objects(hid_t fid1, hid_t fid2, hid_t *ret_did, hid_t *ret_gid1, 
		hid_t *ret_gid2, hid_t *ret_gid3)
{
    int	oid_count;
    herr_t	ret;

    /* Check reference counts of file IDs and opened object IDs.
     * The verification is hard-coded.  If in any case, this testing
     * is changed, remember to check this part and update the macros.
     */
    {
       oid_count = H5Fget_obj_count(fid1, H5F_OBJ_ALL);
       CHECK(oid_count, FAIL, "H5Fget_obj_count");
       VERIFY(oid_count, OBJ_ID_COUNT_2, "H5Fget_obj_count");

       oid_count = H5Fget_obj_count(fid1, H5F_OBJ_DATASET|H5F_OBJ_GROUP|H5F_OBJ_DATATYPE|H5F_OBJ_ATTR);
       CHECK(oid_count, FAIL, "H5Fget_obj_count");
       VERIFY(oid_count, OBJ_ID_COUNT_0, "H5Fget_obj_count");

       oid_count = H5Fget_obj_count(fid2, H5F_OBJ_ALL);
       CHECK(oid_count, FAIL, "H5Fget_obj_count");
       VERIFY(oid_count, OBJ_ID_COUNT_2, "H5Fget_obj_count");

       oid_count = H5Fget_obj_count(fid2, H5F_OBJ_DATASET|H5F_OBJ_GROUP|H5F_OBJ_DATATYPE|H5F_OBJ_ATTR);
       CHECK(oid_count, FAIL, "H5Fget_obj_count");
       VERIFY(oid_count, OBJ_ID_COUNT_0, "H5Fget_obj_count");
    }

    /* create a dataset in the first file open */
    {
       hid_t       dataset_id, dataspace_id;  /* identifiers */
       hsize_t     dims[F2_RANK];
       int         data[F2_DIM0][F2_DIM1];
       unsigned    i,j;

       /* Create the data space for the dataset. */
       dims[0] = F2_DIM0;
       dims[1] = F2_DIM1;
       dataspace_id = H5Screate_simple(F2_RANK, dims, NULL);
       CHECK(dataspace_id, FAIL, "H5Screate_simple");

       /* Create the dataset. */
       dataset_id = H5Dcreate(fid1, "/dset", H5T_NATIVE_INT, dataspace_id,
                        H5P_DEFAULT);
       CHECK(dataset_id, FAIL, "H5Dcreate");

       for(i=0; i<F2_DIM0; i++)
           for(j=0; j<F2_DIM1; j++)
               data[i][j]=i*10+j;

       /* Write data to the new dataset */
       ret = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                H5P_DEFAULT, data);
       CHECK(ret, FAIL, "H5Dwrite");

       if(ret_did != NULL)
           *ret_did = dataset_id;

       /* Terminate access to the data space. */
       ret = H5Sclose(dataspace_id);
       CHECK(ret, FAIL, "H5Sclose");
    }

    /* Create a group in the second file open */
    {
        hid_t   gid1, gid2, gid3;
        gid1 = H5Gcreate(fid2, "/group", 0);
        CHECK(gid1, FAIL, "H5Gcreate");
        if(ret_gid1 != NULL)
            *ret_gid1 = gid1;

        gid2 = H5Gopen(fid2, "/group");
        CHECK(gid2, FAIL, "H5Gopen");
        if(ret_gid2 != NULL)
            *ret_gid2 = gid2;

        gid3 = H5Gopen(fid2, "/group");
        CHECK(gid3, FAIL, "H5Gopen");
        if(ret_gid3 != NULL)
            *ret_gid3 = gid3;
    }

    /* Check reference counts of file IDs and opened object IDs.
     * The verification is hard-coded.  If in any case, this testing
     * is changed, remember to check this part and update the macros.
     */
    {
       oid_count = H5Fget_obj_count(fid1, H5F_OBJ_ALL);
       CHECK(oid_count, FAIL, "H5Fget_obj_count");
       VERIFY(oid_count, OBJ_ID_COUNT_6, "H5Fget_obj_count");

       oid_count = H5Fget_obj_count(fid1, H5F_OBJ_DATASET|H5F_OBJ_GROUP|H5F_OBJ_DATATYPE|H5F_OBJ_ATTR);
       CHECK(oid_count, FAIL, "H5Fget_obj_count");
       VERIFY(oid_count, OBJ_ID_COUNT_4, "H5Fget_obj_count");

       oid_count = H5Fget_obj_count(fid2, H5F_OBJ_ALL);
       CHECK(oid_count, FAIL, "H5Fget_obj_count");
       VERIFY(oid_count, OBJ_ID_COUNT_6, "H5Fget_obj_count");

       oid_count = H5Fget_obj_count(fid2, H5F_OBJ_DATASET|H5F_OBJ_GROUP|H5F_OBJ_DATATYPE|H5F_OBJ_ATTR);
       CHECK(oid_count, FAIL, "H5Fget_obj_count");
       VERIFY(oid_count, OBJ_ID_COUNT_4, "H5Fget_obj_count");
    }
}

/****************************************************************
**
**  test_get_file_id(): Test H5Iget_file_id() 
**
*****************************************************************/
static void 
test_get_file_id(void)
{
    hid_t               fid, fid2, fid3;
    hid_t		datatype_id, dataset_id, dataspace_id, group_id, attr_id;
    hid_t               plist;
    hsize_t             dims[F2_RANK];
    herr_t              ret;

    /* Create a file */
    fid = H5Fcreate(FILE4, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(fid, FAIL, "H5Fcreate");

    /* Test H5Iget_file_id() */
    check_file_id(fid, fid);

    /* Create a group in the file.  Make a duplicated file ID from the group.
     * And close this duplicated ID
     */
    group_id = H5Gcreate(fid, GRP_NAME, 0);
    CHECK(group_id, FAIL, "H5Gcreate");
   
    /* Test H5Iget_file_id() */
    check_file_id(fid, group_id);

    /* Close the file and get file ID from the group ID */
    ret = H5Fclose(fid);
    CHECK(ret, FAIL, "H5Fclose");

    /* Test H5Iget_file_id() */
    check_file_id(-1, group_id);

    ret = H5Gclose(group_id);
    CHECK(ret, FAIL, "H5Gclose");
   
    /* Open the file again.  Test H5Iget_file_id() */
    fid = H5Fopen(FILE4, H5F_ACC_RDWR, H5P_DEFAULT);       
    CHECK(fid, FAIL, "H5Fcreate");
   
    group_id = H5Gopen(fid, GRP_NAME);
    CHECK(group_id, FAIL, "H5Gcreate");
   
    /* Test H5Iget_file_id() */
    check_file_id(fid, group_id);

    /* Open the file for second time.  Test H5Iget_file_id() */
    fid3 = H5Freopen(fid);
    CHECK(fid3, FAIL, "H5Freopen");
    
    /* Test H5Iget_file_id() */
    check_file_id(fid3, fid3);
    
    ret = H5Fclose(fid3);
    CHECK(ret, FAIL, "H5Fclose");
     
    /* Create a dataset in the group.  Make a duplicated file ID from the 
     * dataset.  And close this duplicated ID. 
     */
    dims[0] = F2_DIM0;
    dims[1] = F2_DIM1;
    dataspace_id = H5Screate_simple(F2_RANK, dims, NULL);
    CHECK(dataspace_id, FAIL, "H5Screate_simple");

    dataset_id = H5Dcreate(group_id, DSET_NAME, H5T_NATIVE_INT, dataspace_id,
                        H5P_DEFAULT);
    CHECK(dataset_id, FAIL, "H5Dcreate");
    
    /* Test H5Iget_file_id() */
    check_file_id(fid, dataset_id);
    
    /* Create an attribute for the dataset.  Make a duplicated file ID from
     * this attribute.  And close it.
     */
    attr_id=H5Acreate(dataset_id,ATTR_NAME,H5T_NATIVE_INT,dataspace_id,H5P_DEFAULT);
    CHECK(ret, FAIL, "H5Acreate");

    /* Test H5Iget_file_id() */
    check_file_id(fid, attr_id);

    /* Create a named datatype.  Make a duplicated file ID from
     * this attribute.  And close it.
     */
    datatype_id=H5Tcopy(H5T_NATIVE_INT);
    CHECK(ret, FAIL, "H5Acreate");
    
    ret = H5Tcommit(fid, TYPE_NAME, datatype_id);
    CHECK(ret, FAIL, "H5Tcommit");
    
    /* Test H5Iget_file_id() */
    check_file_id(fid, datatype_id);

    /* Create a property list and try to get file ID from it.
     * Supposed to fail.
     */
    plist = H5Pcreate(H5P_FILE_ACCESS);
    CHECK(plist, FAIL, "H5Pcreate");
    
    H5E_BEGIN_TRY {
        fid2 = H5Iget_file_id(plist);
    } H5E_END_TRY;
    VERIFY(fid2, FAIL, "H5Iget_file_id");
    
    /* Close objects */ 
    ret = H5Tclose(datatype_id);
    CHECK(ret, FAIL, "H5Tclose");

    ret = H5Aclose(attr_id);
    CHECK(ret, FAIL, "H5Aclose");
    
    ret = H5Sclose(dataspace_id);
    CHECK(ret, FAIL, "H5Sclose");

    ret = H5Dclose(dataset_id);
    CHECK(ret, FAIL, "H5Dclose");
   
    ret = H5Gclose(group_id);
    CHECK(ret, FAIL, "H5Gclose");
   
    ret = H5Fclose(fid);
    CHECK(ret, FAIL, "H5Fclose");
}

/****************************************************************
**
**  check_file_id(): Internal function of test_get_file_id() 
**
*****************************************************************/
static void 
check_file_id(hid_t fid, hid_t object_id)
{
    hid_t               new_fid;
    herr_t              ret;

    /* Return a duplicated file ID even not expecting user to do it.
     * And close this duplicated ID
     */
    new_fid = H5Iget_file_id(object_id);

    if(fid >=0)
        VERIFY(new_fid, fid, "H5Iget_file_id");
    else
        CHECK(new_fid, FAIL, "H5Iget_file_id");

    ret = H5Fclose(new_fid);
    CHECK(ret, FAIL, "H5Fclose");
}

/****************************************************************
**
**  test_obj_count_and_id(): test object count and ID list functions. 
**
****************************************************************/
static void 
test_obj_count_and_id(hid_t fid1, hid_t fid2, hid_t did, hid_t gid1, 
			hid_t gid2, hid_t gid3)
{
    hid_t    fid3, fid4;
    int oid_count;
    herr_t   ret;

    /* Create two new files */
    fid3 = H5Fcreate(FILE2, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(fid3, FAIL, "H5Fcreate");
    fid4 = H5Fcreate(FILE3, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(fid4, FAIL, "H5Fcreate");

    /* test object count of all files IDs open */
    oid_count = H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_FILE);
    CHECK(oid_count, FAIL, "H5Fget_obj_count");
    VERIFY(oid_count, OBJ_ID_COUNT_4, "H5Fget_obj_count");

    /* test object count of all datasets open */
    oid_count = H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_DATASET);
    CHECK(oid_count, FAIL, "H5Fget_obj_count");
    VERIFY(oid_count, OBJ_ID_COUNT_1, "H5Fget_obj_count");

    /* test object count of all groups open */
    oid_count = H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_GROUP);
    CHECK(oid_count, FAIL, "H5Fget_obj_count");
    VERIFY(oid_count, OBJ_ID_COUNT_3, "H5Fget_obj_count");

    /* test object count of all named datatypes open */
    oid_count = H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_DATATYPE);
    CHECK(oid_count, FAIL, "H5Fget_obj_count");
    VERIFY(oid_count, OBJ_ID_COUNT_0, "H5Fget_obj_count");

    /* test object count of all attributes open */
    oid_count = H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_ATTR);
    CHECK(oid_count, FAIL, "H5Fget_obj_count");
    VERIFY(oid_count, OBJ_ID_COUNT_0, "H5Fget_obj_count");

    /* test object count of all objects currently open */
    oid_count = H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_ALL);
    CHECK(oid_count, FAIL, "H5Fget_obj_count");
    VERIFY(oid_count, OBJ_ID_COUNT_8, "H5Fget_obj_count");

    {
        hid_t      *oid_list;
        int   i;
        H5I_type_t id_type;

        oid_list = (hid_t*)calloc((size_t)oid_count, sizeof(hid_t));
        if(oid_list != NULL) {
	    ret = H5Fget_obj_ids(H5F_OBJ_ALL, H5F_OBJ_ALL, oid_count, oid_list);
	    CHECK(ret, FAIL, "H5Fget_obj_ids");
        }

        for(i=0; i<oid_count; i++) {
	    id_type = H5Iget_type(oid_list[i]);
	    switch(id_type) {
	        case H5I_FILE:
		    if(oid_list[i]!=fid1 && oid_list[i]!=fid2 &&
			oid_list[i]!=fid3 && oid_list[i]!=fid4) {
			ret = FAIL;
			CHECK(ret, FAIL, "H5Fget_obj_ids");
		    }
		    break;
	        case H5I_GROUP:
                    if(oid_list[i]!=gid1 && oid_list[i]!=gid2 &&
                        oid_list[i]!=gid3) {
			ret = FAIL;
                        CHECK(ret, FAIL, "H5Fget_obj_ids");
                    }   
		    break;
	        case H5I_DATASET:
	 	    VERIFY(oid_list[i], did, "H5Fget_obj_ids");
		    break;
		default:
		    ret = FAIL;
                    CHECK(ret, FAIL, "H5Fget_obj_ids");
	    }
        }	

        free(oid_list);
    }

    /* close the two new files */
    ret = H5Fclose(fid3);
    CHECK(ret, FAIL, "H5Fclose");
    ret = H5Fclose(fid4);
    CHECK(ret, FAIL, "H5Fclose");
}

/****************************************************************
**
**  test_file_perm(): low-level file test routine.  
**      This test verifies that a file can be opened for both 
**      read-only and read-write access and things will be handled
**      appropriately.
**
*****************************************************************/
static void 
test_file_perm(void)
{
    hid_t    file;      /* File opened with read-write permission */
    hid_t    filero;    /* Same file opened with read-only permission */
    hid_t    dspace;    /* Dataspace ID */
    hid_t    dset;      /* Dataset ID */
    herr_t   ret;
 
    /* Output message about test being performed */
    MESSAGE(5, ("Testing Low-Level File Permissions\n"));

    dspace = H5Screate(H5S_SCALAR);
    CHECK(dspace, FAIL, "H5Screate");

    /* Create the file (with read-write permission) */
    file = H5Fcreate(FILE2, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(file, FAIL, "H5Fcreate");

    /* Create a dataset with the read-write file handle */
    dset = H5Dcreate(file, F2_DSET, H5T_NATIVE_INT, dspace, H5P_DEFAULT);
    CHECK(dset, FAIL, "H5Dcreate");

    ret = H5Dclose(dset);
    CHECK(ret, FAIL, "H5Dclose");

    /* Open the file (with read-only permission) */
    filero = H5Fopen(FILE2, H5F_ACC_RDONLY, H5P_DEFAULT);
    CHECK(filero, FAIL, "H5Fopen");

    /* Create a dataset with the read-only file handle (should fail) */
    H5E_BEGIN_TRY {
        dset = H5Dcreate(filero, F2_DSET, H5T_NATIVE_INT, dspace, H5P_DEFAULT);
    } H5E_END_TRY;
    VERIFY(dset, FAIL, "H5Dcreate");
    if(dset!=FAIL) {
        ret = H5Dclose(dset);
        CHECK(ret, FAIL, "H5Dclose");
    } /* end if */

    ret = H5Fclose(filero);
    CHECK(ret, FAIL, "H5Fclose");

    ret = H5Fclose(file);
    CHECK(ret, FAIL, "H5Fclose");

    ret = H5Sclose(dspace);
    CHECK(ret, FAIL, "H5Sclose");

} /* end test_file_perm() */

/****************************************************************
**
**  test_file_freespace(): low-level file test routine.  
**      This test checks the free space available in a file in various
**      situations.
**
*****************************************************************/
static void 
test_file_freespace(void)
{
    hid_t    file;      /* File opened with read-write permission */
    hssize_t free_space;        /* Amount of free space in file */
    hid_t    dspace;    /* Dataspace ID */
    hid_t    dset;      /* Dataset ID */
    hid_t    dcpl;      /* Dataset creation property list */
    unsigned u;         /* Local index variable */
    char     name[32];  /* Dataset name */
    herr_t   ret;
 
    /* Output message about test being performed */
    MESSAGE(5, ("Testing Low-Level File Free Space\n"));

    /* Create the file (with read-write permission) */
    file = H5Fcreate(FILE1, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    CHECK(file, FAIL, "H5Fcreate");

    /* Check that the free space is 0 */
    free_space = H5Fget_freespace(file);
    CHECK(free_space, FAIL, "H5Fget_freespace");
    VERIFY(free_space, 0, "H5Fget_freespace");

    /* Create dataspace for datasets */
    dspace = H5Screate(H5S_SCALAR);
    CHECK(dspace, FAIL, "H5Screate");

    /* Create a dataset creation property list */
    dcpl = H5Pcreate(H5P_DATASET_CREATE);
    CHECK(dcpl, FAIL, "H5Pcreate"); 

    /* Set the space allocation time to early */
    ret = H5Pset_alloc_time(dcpl,H5D_ALLOC_TIME_EARLY);
    CHECK(ret, FAIL, "H5Pset_alloc_time"); 

    /* Create datasets in file */
    for(u=0; u<10; u++) {
        sprintf(name,"Dataset %u",u);
        dset = H5Dcreate(file, name, H5T_STD_U32LE, dspace, dcpl);
        CHECK(dset, FAIL, "H5Dcreate");

        ret = H5Dclose(dset);
        CHECK(ret, FAIL, "H5Dclose");
    } /* end for */

    /* Close dataspace */
    ret = H5Sclose(dspace);
    CHECK(ret, FAIL, "H5Sclose");

    /* Close dataset creation property list */
    ret=H5Pclose(dcpl);
    CHECK(ret, FAIL, "H5Pclose");

    /* Check that there is the right amount of free space in the file */
    free_space = H5Fget_freespace(file);
    CHECK(free_space, FAIL, "H5Fget_freespace");
#ifdef H5_HAVE_LARGE_HSIZET
    VERIFY(free_space, 168, "H5Fget_freespace");
#else /* H5_HAVE_LARGE_HSIZET */
    VERIFY(free_space, 76, "H5Fget_freespace");
#endif /* H5_HAVE_LARGE_HSIZET */

    /* Delete datasets in file */
    for(u=0; u<10; u++) {
        sprintf(name,"Dataset %u",u);
        ret = H5Gunlink(file, name);
        CHECK(ret, FAIL, "H5Gunlink");
    } /* end for */

    /* Check that there is the right amount of free space in the file */
    free_space = H5Fget_freespace(file);
    CHECK(free_space, FAIL, "H5Fget_freespace");
#ifdef H5_HAVE_LARGE_HSIZET
    VERIFY(free_space, 3584, "H5Fget_freespace");
#else /* H5_HAVE_LARGE_HSIZET */
    VERIFY(free_space, 3428, "H5Fget_freespace");
#endif /* H5_HAVE_LARGE_HSIZET */

    /* Close file */
    ret = H5Fclose(file);
    CHECK(ret, FAIL, "H5Fclose");

} /* end test_file_freespace() */

/****************************************************************
**
**  test_file(): Main low-level file I/O test routine.
** 
****************************************************************/
void 
test_file(void)
{
    /* Output message about test being performed */
    MESSAGE(5, ("Testing Low-Level File I/O\n"));

    test_file_create();		/* Test file creation(also creation templates)*/
    test_file_open();		/* Test file opening */
#ifndef H5_NO_SHARED_WRITING 
    test_file_close();          /* Test file close behavior */
#endif /* H5_NO_SHARED_WRITING */     
    test_get_file_id();         /* Test H5Iget_file_id */
    test_file_perm();           /* Test file access permissions */
    test_file_freespace();      /* Test file free space information */
}				/* test_file() */


/*-------------------------------------------------------------------------
 * Function:	cleanup_file
 *
 * Purpose:	Cleanup temporary test files
 *
 * Return:	none
 *
 * Programmer:	Albert Cheng
 *              July 2, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
cleanup_file(void)
{
    remove(FILE1);
    remove(FILE2);
    remove(FILE3);
    remove(FILE4);
}
