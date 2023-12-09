#!/bin/bash

VERSION=${1:-latest}
docker rm -f openresty

docker run -itd \
    --restart=always \
    --name openresty \
    --user=root \
    --privileged=true \
    -p 80:80 \
    -v /home/zion/workdir/ffmpeg/push-streaming/server/conf/nginx.conf:/opt/bitnami/openresty/nginx/conf/nginx.conf:ro \
    -v /home/zion/workdir/blog/turtles-live:/web \
    bitnami/openresty:${VERSION}