#include <stdio.h>

main(int argc, char **argv)
{
  int width, height, shift, x, y, i;
  unsigned short row[7000];
  unsigned char out[7000];
  char str[64];
  int min[4], max[4], count[4];
  long long total[4];

  if (argc < 4) {
    fprintf(stderr,"Usage:  %s <width> <height> <shift>\n",argv[0]);
    exit(1);
  }
  width =atoi(argv[1]);
  height=atoi(argv[2]);
  shift =atoi(argv[3]);

  printf("P5 %d %d 255\n", width, height);

  for (i=0; i < 4; i++) {
    max[i]=count[i]=total[i]=0;
    min[i]=0xfffffff;
  }

  for (y=0; y < height; y++) {
    fread (row, 2, width, stdin);
    for (x=0; x < width; x++) {
      i = ((y << 1) & 2) + (x & 1);
      if (max[i] < row[x]) max[i] = row[x];
      if (min[i] > row[x]) min[i] = row[x];
      count[i]++;
      total[i] += row[x];
      out[x] = row[x] >> shift;
    }
    fwrite (out, 1, width, stdout);
  }

  for (i=0; i < 4; i++) {
    fprintf (stderr,"min=%d  max=%d  avg=%f\n",
	min[i],max[i],(float) total[i]/count[i]);
  }
}
