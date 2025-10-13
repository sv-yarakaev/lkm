// brd_test.c — тестирование ioctl для brd (RAM disk)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>


#define BRD_IOC_MAGIC 'R'
#define BRD_RESET_STATS     _IO(BRD_IOC_MAGIC, 0)
#define BRD_GET_UPTIME_MS  _IOR(BRD_IOC_MAGIC, 1, __u64)
#define BRD_DUMP           _IOWR(BRD_IOC_MAGIC, 3, struct brd_dump_args)

struct brd_dump_args {
    __u64 sector;
    __u32 count;
    __u32 __pad;   // ← явный padding!
    __u64 buf_ptr;
};

int dump_sectors_to_file(int fd, const char *outfile, __u64 start_sector, __u32 count) {
    if (count == 0 || count > 1024) {
        fprintf(stderr, "Invalid sector count: %u (max 1024)\n", count);
        return -1;
    }

    size_t buf_size = count * 512;
    char *buffer = malloc(buf_size);
    if (!buffer) {
        perror("malloc");
        return -1;
    }

    struct brd_dump_args args = {
        .sector = start_sector,
        .count = count,
        .buf_ptr = (uintptr_t)buffer
    };

    if (ioctl(fd, BRD_DUMP, &args) != 0) {
        perror("ioctl BRD_DUMP");
        free(buffer);
        return -1;
    }

    FILE *f = fopen(outfile, "wb");
    if (!f) {
        perror("fopen output");
        free(buffer);
        return -1;
    }

    fwrite(buffer, 1, buf_size, f);
    fclose(f);
    free(buffer);
    printf("Dumped %u sectors (starting at %llu) to '%s'\n", count, start_sector, outfile);
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s /dev/ramN [dump <outfile> <start_sector> <count>]\n", argv[0]);
        fprintf(stderr, "       %s /dev/ramN reset\n", argv[0]);
        fprintf(stderr, "       %s /dev/ramN uptime\n", argv[0]);
        return 1;
    }

    const char *dev = argv[1];
    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        perror("open device");
        return 1;
    }

    if (argc == 2) {
        // По умолчанию — показать uptime
        __u64 uptime;
        if (ioctl(fd, BRD_GET_UPTIME_MS, &uptime) == 0) {
            printf("Uptime: %llu ms\n", uptime);
        } else {
            perror("ioctl BRD_GET_UPTIME_MS");
        }
    } else if (strcmp(argv[2], "reset") == 0) {
        if (ioctl(fd, BRD_RESET_STATS) == 0) {
            printf("Statistics reset.\n");
        } else {
            perror("ioctl BRD_RESET_STATS");
        }
    } else if (strcmp(argv[2], "uptime") == 0) {
        __u64 uptime;
        if (ioctl(fd, BRD_GET_UPTIME_MS, &uptime) == 0) {
            printf("Uptime: %llu ms\n", uptime);
        } else {
            perror("ioctl BRD_GET_UPTIME_MS");
        }
    } else if (strcmp(argv[2], "dump") == 0) {
        if (argc != 6) {
            fprintf(stderr, "Usage: %s %s dump <outfile> <start_sector> <count>\n", argv[0], dev);
            close(fd);
            return 1;
        }
        const char *outfile = argv[3];
        __u64 start_sector = strtoull(argv[4], NULL, 10);
        __u32 count = (__u32)strtoul(argv[5], NULL, 10);
        dump_sectors_to_file(fd, outfile, start_sector, count);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[2]);
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}