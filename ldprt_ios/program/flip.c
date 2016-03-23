//
//  flip.c
//  prt_ios
//
//  Created by iaccepted on 16/3/14.
//  Copyright (c) 2016年 iaccepted. All rights reserved.
//

#if defined(__APPLE__) || defined(__unix)

#ifndef __ENABLE_COMPATIBILITY_WITH_UNIX_2003__
#define __ENABLE_COMPATIBILITY_WITH_UNIX_2003__

#include <stdio.h>
#include <dirent.h>
#include <time.h>

FILE *fopen$UNIX2003( const char *filename, const char *mode )
{
    return fopen(filename, mode);
}

size_t fwrite$UNIX2003( const void *a, size_t b, size_t c, FILE *d )
{
    return fwrite(a, b, c, d);
}

char *strerror$UNIX2003( int errnum )
{
    return strerror(errnum);
}

DIR *opendir$INODE64(const char * a)
{
    return opendir(a);
}

struct dirent *readdir$INODE64(DIR *dir)
{
    return readdir(dir);
}

time_t mktime$UNIX2003(struct tm * a)
{
    return mktime(a);
}

#endif
#endif

