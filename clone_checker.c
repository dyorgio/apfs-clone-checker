//
//  clone_checker.c
//
//  To compile:
//    gcc clone_checker.c -o clone_checker
//
//  Created by Dyorgio Nascimento on 2020-12-10.
//
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <string.h>

// declare methods
int compare_blocks(int block_size, char *filenameA, char *filenameB, int fdA, int fdB);
void check_disk_fs(char *filename);
struct stat check_file(char *filename);

// entrypoint
int main(int args_count, char **args) {

  if (args_count < 3) {
    fprintf(stderr, "Usage: clone_checker fileA fileB\n");
    exit(EXIT_FAILURE);
  }

  check_disk_fs(args[1]);
  check_disk_fs(args[2]);

  struct stat statA = check_file(args[1]);
  struct stat statB = check_file(args[2]);

  if (statA.st_dev != statB.st_dev || statA.st_size != statB.st_size || statA.st_blocks != statB.st_blocks) {
    // clones only are supported on same device and have same size em blocks count
    fprintf(stdout,"1\n");
    exit(EXIT_SUCCESS);
  }

  int fdA = open(args[1], O_RDONLY);
  if (fdA < 0 ) {
    fprintf(stderr,"%s: Cannot open. %s\n", args[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  int fdB = open(args[2], O_RDONLY);
  if ( fdB < 0 ) {
    fprintf(stderr,"%s: Cannot open. %s\n", args[2], strerror(errno));
    close(fdA);
    exit(EXIT_FAILURE);
  }

  int result = compare_blocks(statA.st_blksize, args[1], args[2], fdA, fdB);

  close(fdA);
  close(fdB);

  if (result != -1) {
    fprintf(stdout,"%i\n", result);
    exit(EXIT_SUCCESS);
  } else {
    exit(EXIT_FAILURE);
  }
}

int compare_blocks(int block_size, char *filenameA, char *filenameB, int fdA, int fdB) {

  long sts = 0;
  struct log2phys physA;
  struct log2phys physB;

  for ( off_t offset = 0; sts >= 0; offset += block_size ) {
    // Seek files to current position
    sts = lseek(fdA, offset, SEEK_SET);
    if ( sts < 0 ) {
      fprintf(stderr,"%s: Cannot seek. %ld %s\n", filenameA, sts, strerror(errno));
      return -1;
    }
    sts = lseek(fdB, offset, SEEK_SET);
    if ( sts < 0 ) {
      fprintf(stderr,"%s: Cannot seek. %ld %s\n", filenameB, sts, strerror(errno));
      return -1;
    }

    // get current blocks physical location
    sts = fcntl(fdA, F_LOG2PHYS, &physA);
    if ( sts < 0 && errno == ERANGE ) {
      sts = fcntl(fdB, F_LOG2PHYS, &physB);
      if ( sts < 0 && errno == ERANGE ) {
        // both files seeked to the end with same offsets
        return 0;
      } else if ( sts < 0 ) {
        fprintf(stderr,"%s: Cannot LOG2PHYS. %ld %s\n", filenameB, sts, strerror(errno));
        return -1;
      }
      break;
    } else if ( sts < 0 ) {
      fprintf(stderr,"%s: Cannot LOG2PHYS. %ld %s\n", filenameA, sts, strerror(errno));
      return -1;
    }

    sts = fcntl(fdB, F_LOG2PHYS, &physB);
    if ( sts < 0 && errno == ERANGE ) {
      // insanity check, size of files already verified before
      break;
    } else if ( sts < 0 ) {
      return -1;
    }

    if ( physA.l2p_devoffset != physB.l2p_devoffset ) {
      // found a diff block
      break;
    }
  }

  // not a clone (check loop breaked)
  return 1;
}

void check_disk_fs(char *filename) {
  struct statfs fs;
  if( statfs(filename, &fs) == 0 ) {
    if( strcmp(fs.f_fstypename, "apfs") != 0) {
      fprintf(stderr, "%s: Only APFS is supported: %s\n", filename, fs.f_fstypename);
      exit(EXIT_FAILURE);
    }
  }
}

struct stat check_file(char *filename) {
  struct stat st;
  if ( stat(filename, &st) < 0) {
    fprintf(stderr, "%s: No such file\n", filename);
    exit(EXIT_FAILURE);
  }

  if ( (st.st_mode & S_IFMT) != S_IFREG ) {
    fprintf(stderr, "%s: Not a regular file\n", filename);
    exit(EXIT_FAILURE);
  }
  return st;
}
