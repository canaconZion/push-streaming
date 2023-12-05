#include <stdio.h>
#include <string.h>
#include <string>
using namespace std;

#define CMD_RESULT_BUF_SIZE 1024

/*
 * cmd：待执行命令
 * result：命令输出结果
 * 函数返回：0 成功；-1 失败；
 */
int ExecuteCMD(const char *cmd, char *result)
{
    int iRet = -1;
    char buf_ps[CMD_RESULT_BUF_SIZE];
    char ps[CMD_RESULT_BUF_SIZE] = {0};
    FILE *ptr;

    strcpy(ps, cmd);

    if ((ptr = popen(ps, "r")) != NULL)
    {
        while (fgets(buf_ps, sizeof(buf_ps), ptr) != NULL)
        {
            strcat(result, buf_ps);
            if (strlen(result) > CMD_RESULT_BUF_SIZE)
            {
                break;
            }
        }
        pclose(ptr);
        ptr = NULL;
        iRet = 0; // 处理成功
    }
    else
    {
        printf("popen %s error\n", ps);
        iRet = -1; // 处理失败
    }

    return iRet;
}

int push_cmd(char *file, char *cmd)
{
    char *base_cmd = "ffmpeg -re -stream_loop -1 -i";
    char *format = "flv";
    char *output = "rtmp://localhost:1935/live/screen";
}

string splic_cmd(string media_file)
{

    string base_cmd = "ffmpeg -re -stream_loop -1 -i";
    string format = "flv";
    string output = "rtmp://localhost:1935/live/screen";
    string a = base_cmd + " " + media_file + " -f " + format + " " + output;
    printf("%s\n", a.c_str());
    return a;
}

int main()
{
    char result[CMD_RESULT_BUF_SIZE] = {0};
    string command = "screen.mp4";
    string res = splic_cmd(command);

    ExecuteCMD(res.c_str(), result);

    return 0;
}