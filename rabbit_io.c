
#include "rabbit_io.h"

int read_word(FILE *fp, rabbitw *w) {
  int nread = 0;
  unsigned char *wordp = (unsigned char *)w;
  for (int i = 3; i >= 0; i--) {
    int tempread = fread(&wordp[i], sizeof wordp[i], 1, fp);
    if (tempread == 0) {
      return 0;
    }

    nread += tempread;
  }

  return nread;
}
