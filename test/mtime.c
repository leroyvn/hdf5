/*
 * Copyright � 1998 NCSA
 *                  All rights reserved.
 *
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Thursday, July 30, 1998
 *
 * Purpose:	Determines if the modification time message is working
 *		properly.  Specifically, the code in H5O_mtime_decode() is
 *		very OS-dependent and this test tries to figure out if it's
 *		working properly.
 */
#include <h5test.h>

const char *FILENAME[] = {
    "mtime",
    NULL
};


/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	H5O_mtime_decode() test.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 30, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    hid_t	fapl, file, space, dset;
    hsize_t	size[1] = {2};
    time_t	now;
    struct tm	*tm;
    H5G_stat_t	sb1, sb2;
    char	buf1[32], buf2[32], filename[1024];

    h5_reset();
    fapl = h5_fileaccess();
    
    TESTING("modification time messages");
    
    /* Create the file, create a dataset, then close the file */
    h5_fixname(FILENAME[0], fapl, filename, sizeof filename);
    if ((file=H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl))<0)
	return 1;
    if ((space=H5Screate_simple(1, size, NULL))<0) return 1;
    if ((dset=H5Dcreate(file, "dset", H5T_NATIVE_CHAR, space, H5P_DEFAULT))<0)
	return 1;
    now = time(NULL);
    if (H5Dclose(dset)<0) return 1;
    if (H5Sclose(space)<0) return 1;
    if (H5Fclose(file)<0) return 1;

    /*
     * Open the file and get the modification time. We'll test the new
     * H5Gget_objinfo() arguments too: being able to stat something without
     * knowing its name.
     */
    h5_fixname(FILENAME[0], fapl, filename, sizeof filename);
    if ((file = H5Fopen(filename, H5F_ACC_RDONLY, fapl))<0) return 1;
    if (H5Gget_objinfo(file, "dset", TRUE, &sb1)<0) return 1;
    if ((dset=H5Dopen(file, "dset"))<0) return 1;
    if (H5Gget_objinfo(dset, ".", TRUE, &sb2)<0) return 1;
    if (H5Dclose(dset)<0) return 1;
    if (H5Fclose(file)<0) return 1;

    /* Compare times from the two ways of calling H5Gget_objinfo() */
    if (sb1.objno[0]!=sb2.objno[0] || sb1.objno[1]!=sb2.objno[1] ||
	sb1.mtime!=sb2.mtime) {
	FAILED();
	puts("    Calling H5Gget_objinfo() with the dataset ID returned");
	puts("    different values than calling it with a file and dataset");
	puts("    name.");
	return 1;
    }
    
    /* Compare times -- they must be within 60 seconds of one another */
    if (0==sb1.mtime) {
	SKIPPED();
	puts("    The modification time could not be decoded on this OS.");
	puts("    Modification times will be mantained in the file but");
	puts("    cannot be queried on this system.  See H5O_mtime_decode().");
	return 0;
    } else if (fabs(HDdifftime(now, sb1.mtime))>60.0) {
	FAILED();
	tm = localtime(&(sb1.mtime));
	strftime(buf1, sizeof buf1, "%Y-%m-%d %H:%M:%S", tm);
	tm = localtime(&now);
	strftime(buf2, sizeof buf2, "%Y-%m-%d %H:%M:%S", tm);
	printf("    got: %s\n    ans: %s\n", buf1, buf2);
	return 1;
    }
    
    /* All looks good */
    PASSED();
    puts("All modification time tests passed.");
    h5_cleanup(fapl);
    return 0;
}

    
