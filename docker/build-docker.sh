#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..

DOCKER_IMAGE=${DOCKER_IMAGE:-absolutecommunity/docker-absoluted}
DOCKER_TAG=${DOCKER_TAG:-latest}

BUILD_DIR=${BUILD_DIR:-.}

rm docker/bin/*
mkdir docker/bin
cp $BUILD_DIR/src/absoluted docker/bin/
cp $BUILD_DIR/src/absolute-cli docker/bin/
cp $BUILD_DIR/src/absolute-tx docker/bin/
strip docker/bin/absoluted
strip docker/bin/absolute-cli
strip docker/bin/absolute-tx

docker build --pull -t $DOCKER_IMAGE:$DOCKER_TAG -f docker/Dockerfile docker
