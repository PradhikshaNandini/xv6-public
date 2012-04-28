#include "types.h"
#include "stat.h"
#include "user.h"
#include "mtrace.h"
#include "amd64.h"
#include "fcntl.h"

static const bool pinit = true;

enum { nfile = 10 };
enum { nlookup = 100 };

void
bench(u32 tid, int nloop)
{
  char pn[MAXNAME];

  if (pinit)
    setaffinity(tid);

  for (u32 x = 0; x < nloop; x++) {
    for (u32 i = 0; i < nfile; i++) {
      snprintf(pn, sizeof(pn), "/dbx/f:%d:%d", tid, i);

      int fd = open(pn, O_CREATE | O_RDWR);
      if (fd < 0)
	die("create failed\n");

      close(fd);
    }

    for (u32 i = 0; i < nlookup; i++) {
      snprintf(pn, sizeof(pn), "/dbx/f:%d:%d", tid, (i % nfile));
      int fd = open(pn, O_RDWR);
      if (fd < 0)
        die("open failed\n");

      close(fd);
    }

    for (u32 i = 0; i < nfile; i++) {
      snprintf(pn, sizeof(pn), "/dbx/f:%d:%d", tid, i);
      if (unlink(pn) < 0)
	die("unlink failed\n");
    }
  }

  exit();
}

int
main(int ac, char** av)
{
  int nthread;
  int nloop;
  
#ifdef HW_qemu
  nloop = 50;
#else
  nloop = 1000;
#endif

  if (ac < 2)
    die("usage: %s nthreads [nloop]", av[0]);

  nthread = atoi(av[1]);
  if (ac > 2)
    nloop = atoi(av[2]);

  mkdir("/dbx");

  mtenable("xv6-dirbench");
  u64 t0 = rdtsc();
  for(u32 i = 0; i < nthread; i++) {
    int pid = fork(0);
    if (pid == 0)
      bench(i, nloop);
    else if (pid < 0)
      die("fork");
  }

  for (u32 i = 0; i < nthread; i++)
    wait();
  u64 t1 = rdtsc();
  mtdisable("xv6-dirbench");

  printf("dirbench: %lu\n", t1-t0);
  return 0;
}
