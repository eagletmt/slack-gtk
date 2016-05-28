# slack-gtk
Native desktop client for slack.com

## Dependencies
- jsoncpp
- gtkmm 3
- libsoup
- libnotify

## Build
```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

## Run
1. Issue test token https://api.slack.com/docs/oauth-test-tokens
2. Run `SLACK_GTK_TOKEN=... SLACK_GTK_EMOJI_DIRECTORY=emoji-data GSETTINGS_SCHEMA_DIR=build/schemas ./build/slack-gtk`

## Configuration
```sh
gsettings set cc.wanko.slack-gtk notification-timeout 10000
```
