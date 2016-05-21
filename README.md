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
2. Run `SLACK_GTK_TOKEN=... ./build/slack-gtk`
