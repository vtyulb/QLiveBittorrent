#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static char *mountPoint;
static int mountLen;
static char *buffer;

void recreateBuf(char *path) {
    int i;
    for (i = mountLen; path[i - mountLen] != 0; i++)
        buffer[i] = path[i - mountLen];

    buffer[i] = 0;
}

static int qlive_getattr(const char *path, struct stat *stbuf)
{
    recreateBuf(path);
    return stat(buffer, stbuf);
}

static int qlive_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;

    DIR *dir;
    recreateBuf(path);
    if ((dir = opendir(buffer)) == NULL)
        return -ENOENT;

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL)
        filler(buf, dp->d_name, NULL, 0);

    return 0;
}

static int qlive_open(const char *path, struct fuse_file_info *fi)
{
    recreateBuf(path);
    FILE *test = fopen(buffer, "r");
    if (test == NULL)
        return -ENOENT;
    else
        fclose(test);

    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int qlive_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi)
{
    (void) fi;
    printf("%d %d\n", offset + 1, size);
    fflush(stdout);
    int success;
    scanf("%d", &success);
    assert(success == 1);
    recreateBuf(path);
    FILE *f = fopen(buffer, "rb");
    if (f == NULL)
        return -EACCES;

    fseek(f, offset, SEEK_SET);
    int res = fread(buf, 1, size, f);
    fclose(f);

    return res;
}

void qlive_init(char *path) {
    mountPoint = path;
    buffer = malloc(1000);
    for (mountLen = 0; mountPoint[mountLen] != 0; mountLen++)
        buffer[mountLen] = mountPoint[mountLen];
}

static struct fuse_operations qlive_oper = {
    .getattr	= qlive_getattr,
    .readdir	= qlive_readdir,
    .open		= qlive_open,
    .read		= qlive_read,
};

int main(int argc, char *argv[])
{
    qlive_init(argv[argc - 1]);
    return fuse_main(argc - 1, argv, &qlive_oper, NULL);
}
