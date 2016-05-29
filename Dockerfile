FROM ubuntu:16.04

RUN mkdir -p /app/build
RUN apt-get update && apt-get install -y cmake clang gtkmm-3.0-dev libsoup2.4-dev libjsoncpp-dev libboost-dev libnotify-dev
COPY . /app
WORKDIR /app/build
