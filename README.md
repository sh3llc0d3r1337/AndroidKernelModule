# AndroidKernelModule
PoC project for using android kernel module to bypass root detection

Many root detection algorithm checks the existence of a file or binary on the phone. For example the existence of a su binary is a good indication, that the phone is rooted. This check can be bypassed many ways, like patching the binary or using Xposed framework or Frida to hook and modify methods. However the application can be protected against such manipulation.

Another way to bypass this check is creating a kernel module, hook system calls and hide the files from the application. This technique can also be used for other more sophisticated things. This PoC project demonstrates this technique.

## testapp
The sample application checks the existence of a test.txt file in the /data/local/tmp folder. If the file exists, the phone is considered rooted.

## kernel
I used android emulator on Mac. I compiled kernel image with the config files of the original image.
The original image can be downloaded from /proc/config.gz.

The emulator can be started with this image with this command line:

```
  $ ~/Library/Android/sdk/emulator/emulator @a27_x64 -show-kernel -kernel bzImage-3.18
```

a27_x64 is the name of the AVD, which was created with AVD manager

-show-kernel displays kernel messages

## module
The module hooks onto the *faccessat* system call. This is used when File.fileexists() is called in Java.

The used system call function can be figured out with strace. For more information check this link:

https://source.android.com/devices/tech/debug/strace

The module can be inserted into the kernel the following way:

```
$ adb root
$ adb push android_module.ko /data/local/tmp
$ adb shell
# insmod /data/local/tmp/android_module.ko uid=10028
```

uid is the user id of the test application. List the /data/data folder to find this value.
