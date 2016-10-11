#!/bin/sh

ndk-build NDK_PROJECT_PATH=./inject NDK_APPLICATION_MK=./inject/Application.mk
ndk-build NDK_PROJECT_PATH=./hook NDK_APPLICATION_MK=./hook/Application.mk

adb push ./inject/libs/armeabi-v7a/inject data
adb push ./hook/libs/armeabi-v7a/libhook.so data

adb shell /data/inject /system/bin/mediaserver /data/libhook.so
adb shell /data/inject system_server /data/libhook.so

adb shell chmod 777 /data/system/packages.xml
