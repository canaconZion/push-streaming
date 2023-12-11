ffmpeg -re -stream_loop -1 -i screen.mp4 -c:v libx264 -f flv rtmp://localhost:1935/live/screen
ffmpeg -re -i ./doc/source.flv -c copy -f flv rtmp://localhost/live/livestream
ffmpeg -f dshow  -i video="screen-capture-recorder"  -c:v libx264 -f flv rtmp://localhost:1935/live/screen