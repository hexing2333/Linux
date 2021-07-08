#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#define MAX_LENGTH 255

struct Option
{
    bool r;
    bool a;
    bool l;
    bool h;
    bool m;
    int llength;
    int hlength;
    int mdays;
    int fileAndDirSum;
    int fileAndDir[MAX_LENGTH];
};

struct Option opt;

struct Option optInit()
{
    struct Option opt;
    opt.r = false;
    opt.a = false;
    opt.l = false;
    opt.h = false;
    opt.m = false;
    opt.llength = 0;
    opt.hlength = 0;
    opt.mdays = 0;
    opt.fileAndDirSum = 0;
    return opt;
};

void getOpt(int argc, char *argv[])
{
    int i = 0;
    bool optCancle = false;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            optCancle = true;
            continue;
        }
        /*如果有--选项 则所有自定义选项命令终止分析*/
        if (argv[i][0] == '-' && !optCancle)
        {
            switch (argv[i][1])
            {
            case 'r':
                opt.r = true;
                break;
            case 'a':
                opt.a = true;
                break;
            case 'l':
                opt.l = true;
                i++;
                opt.llength = atoi(argv[i]);
                break;
            case 'h':
                opt.h = true;
                i++;
                opt.hlength = atoi(argv[i]);
                break;
            case 'm':
                opt.m = true;
                i++;
                opt.mdays = atoi(argv[i]);
                break;
            default:
                break;
            }
        }
        else
        {
            opt.fileAndDir[opt.fileAndDirSum] = i;//将目录路径存入数组
            opt.fileAndDirSum++;
        }
    }
};

void printS(char *path)
{
    struct stat st;
    time_t t_now;
    char *fileName;
    time(&t_now);
    int ret = stat(path, &st);
    if (ret == -1)
    {
        printf("%s: No such file or directory\n", path);
    }
    else
    {
        //printf("filename:%s\n",fileName);

        if (opt.l)
        {
            if (st.st_size < opt.llength)
                return;
        }
        if (opt.h)
        {
            if (st.st_size > opt.hlength)
                return;
        }
        if (opt.m)
        {
            if (t_now - st.st_mtime > opt.mdays * 24 * 60 * 60)
                return;
        }
        printf("%10ld  %s\n", st.st_size, path);
    }
};

void scanDir(char *dir, int depth) // 定义目录扫描函数
{
    DIR *dp;              // 定义子目录流指针
    struct dirent *entry; // 定义dirent结构指针保存后续目录
    struct stat statbuf;  // 定义statbuf结构保存文件属性
    char path[512] = {0};
    if ((dp = opendir(dir)) == NULL) // 打开目录，获取子目录流指针，判断操作是否成功
    {
        printf("%s\n:No such file or directory\n", dir);
        return;
    }
    while ((entry = readdir(dp)) != NULL) // 获取下一级目录信息，如果未否则循环
    {
        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) //如果遇到.文件夹..文件夹就跳过进入下一个循环
            continue;
        if (entry->d_name[0] == '.' && opt.a == false)//如果目录或文件名以圆点开头且无a选项 则不输出信息
            continue;
        if (depth == 0)//若是第一次进入该函数，则将当前目录/文件名赋值给path
        {
            sprintf(path, "%s", entry->d_name);
        }
        else
        {
            sprintf(path, "%s/%s", dir, entry->d_name);//若是递归调用或确定该文件不是目录，则将dir/entry->d_name赋值给path
        }
        lstat(path, &statbuf); // 获取下一级成员属性
        if (S_ISDIR(statbuf.st_mode))
        {
            if (opt.r)
            {
                scanDir(path, depth + 1); // 若是目录且有-r选项，则递归调用自身，扫描下一级目录的内容
            }
        }
        else
            printS(path);//否则输出该文件信息即可
    }             // 回到上级目录
    closedir(dp); // 关闭子目录流
};

int main(int argc, char *argv[])
{
    opt = optInit();
    getOpt(argc, argv);
    /*不跟具体文件目录*/
    if (opt.fileAndDirSum == 0)
    {
        char _path[MAX_LENGTH];
        getcwd(_path, MAX_LENGTH);//当前目录路径
        scanDir(_path, 0);
    }
    else
    {
        for (int i = 0; i < opt.fileAndDirSum; i++)
        {
            struct stat st;
            lstat(argv[opt.fileAndDir[i]], &st);
            if (S_ISDIR(st.st_mode))
            {
                scanDir(argv[opt.fileAndDir[i]], 100);
            }
            else
            {
                printS(argv[opt.fileAndDir[i]]);
            }
        }
    }
    return 0;
}
