/*
   Fixes dates on Canon PowerShot G2 CRW files to match the
   internal time stamps (assumed to be Universal Time).  This
   doesn't work with JPEG files; use "TZ= jhead -ft" for them.

   Dave Coffin  9/4/2003
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utime.h>

typedef unsigned char uchar;

FILE *ifp;
short order;
int timestamp;

/*
   Get a 2-byte integer, making no assumptions about CPU byte order.
   Nor should we assume that the compiler evaluates left-to-right.
 */
ushort fget2 (FILE *f)
{
  uchar a, b;

  a = fgetc(f);
  b = fgetc(f);
  if (order == 0x4949)          /* "II" means little-endian */
    return a + (b << 8);
  else                          /* "MM" means big-endian */
    return (a << 8) + b;
}

/*
   Same for a 4-byte integer.
 */
int fget4 (FILE *f)
{
  uchar a, b, c, d;

  a = fgetc(f);
  b = fgetc(f);
  c = fgetc(f);
  d = fgetc(f);
  if (order == 0x4949)
    return a + (b << 8) + (c << 16) + (d << 24);
  else
    return (a << 24) + (b << 16) + (c << 8) + d;
}

/*
   Parse the CIFF structure looking for two pieces of information:
   The camera model, and the decode table number.
 */
void parse_ciff(int offset, int length)
{
  int tboff, nrecs, i, type, len, roff, aoff, save;

  fseek (ifp, offset+length-4, SEEK_SET);
  tboff = fget4(ifp) + offset;
  fseek (ifp, tboff, SEEK_SET);
  nrecs = fget2(ifp);
  for (i = 0; i < nrecs; i++) {
    type = fget2(ifp);
    len  = fget4(ifp);
    roff = fget4(ifp);
    aoff = offset + roff;
    save = ftell(ifp);
    if (type == 0x180e) {		/* Get the timestamp */
      fseek (ifp, aoff, SEEK_SET);
      timestamp = fget4(ifp);
    }
    if (type >> 8 == 0x28 || type >> 8 == 0x30)	/* Get sub-tables */
      parse_ciff(aoff, len);
    fseek (ifp, save, SEEK_SET);
  }
}

int main(int argc, char **argv)
{
  struct utimbuf ut;
  char head[26];
  int arg, hlen, fsize, magic;

  if (argc < 2)
  { fprintf(stderr,"Usage: %s file1 file2 ...\n",argv[0]);
    exit(1);
  }
  for (arg=1; arg < argc; arg++)
  {
    ifp = fopen(argv[arg],"r");
    if (!ifp)
    { perror(argv[arg]);
      continue;
    }

    timestamp = 0;
    order = fget2(ifp);
    hlen = fget4(ifp);
    fread (head, 1, 26, ifp);
    fseek (ifp, 0, SEEK_END);
    fsize = ftell(ifp);
    fseek (ifp, 0, SEEK_SET);
    magic = fget4(ifp);
    if (order == 0x4949 || order == 0x4d4d)
      if (!memcmp(head,"HEAPCCDR",8))
        parse_ciff (hlen, fsize - hlen);
    fclose(ifp);

    if (timestamp) {
      ut.actime = ut.modtime = timestamp;
      utime (argv[arg], &ut);
    } else
      fprintf(stderr,"%s:  Internal date stamp not found.\n", argv[arg]);
  }
  return 0;
}
