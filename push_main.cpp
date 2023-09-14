#include "Rrconfig.h"
#include "streaming_p.h"

#include <iostream>
#include <string>

int main(int argc, int* argv[])
{
    bool ret;
    int res;
    std::string rtmp_url;
    std::string Filepath;
    std::string Protocol;
    std::string HostName;
    std::string Port;
    std::string Key;
    std::string Pwd;
    char *input_url, *file_path;
    int count = 0;
    int i = 0;
    rr::RrConfig config;
    PushStreaming stream;

    ret = config.ReadConfig("config.ini");
    if(ret == false)
    {
        printf("Failed to read config config.ini");
        return 1;
    }
    Filepath = config.ReadString("PUSHSTREAMING", "Filepath", "");
	Protocol = config.ReadString("PUSHSTREAMING", "Protocol", "");
	HostName = config.ReadString("PUSHSTREAMING", "HostName", "");
	Port = config.ReadString("PUSHSTREAMING", "Port", "");
	Key = config.ReadString("PUSHSTREAMING", "Key", "");
	Pwd = config.ReadString("PUSHSTREAMING", "Pwd", "");
	int Pushtimes = config.ReadInt("PUSHSTREAMING", "PushTimes", 0);
	int KeepPush = config.ReadInt("PUSHSTREAMING", "KeepPush", 0);
    rtmp_url = Protocol+"://"+HostName+":"+Port+"/"+Key+"/"+Pwd;
    input_url = (char*)rtmp_url.c_str();
	file_path = (char*)Filepath.c_str();
    printf("%s\n",input_url);
    while(i < Pushtimes)
    {
        if(KeepPush==1){
			i++;
			printf("第 %d 次播放\n",i);
			fflush(stdout); // 强制刷新缓存区
		}
		else{
			count +=1;
			printf("第 %d 次循环播放\n",count);
		}
		res = stream.PushStream(file_path, input_url);
        printf("--------- %d\n",res);
        if(res < 0)
        {
            printf("Error: push streaming failed !!!\n");
            break;
        }
    }
    return 0;
}