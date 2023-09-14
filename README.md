# push-streaming
![Github stars](https://img.shields.io/github/stars/canaconZion/push-streaming.svg) 
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/canaconZion/push-streaming)
将视频文件推送为rtmp流

## Usage
```bash
sudo apt update && apt install ffmpeg &&
git clone  &&
make && ./rtmpPush
```
## Config
config.ini
```ini
[PUSHSTREAMING]
# 视频原文件
Filepath=/home/video/videos/mecha.mp4 
# 推流协议
Protocol=rtmp 
# 流媒体服务器地址
HostName=localhost 
# 推流端口
Port=1935 
# 应用程序名
Key=mecha
# 流名
Pwd=test
# 循环推流次数
PushTimes=2 
# KeepPush=0 不限制次数循环播放
# KeepPush=1 按次数播放
KeepPush=1
```
