#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define LOG_TAG "DEBUG"
#include <log_util.h>
#include <elf.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
//#include <sys/exec_elf.h>
#include <time.h>
//#include <linux/elf.h>
#include <linux/ioctl.h>
#include <linux/binder.h>
#include "linux/videodev2.h"
#include <linux/soundcard.h>
//#include <rpc/rpc.h>
#include "asound.h"
//#include "hardware/sensors.h"
#include "my_msg_q_rcv.h"
#include "binder.h"
#include "mxml/mxml.h"
//#include "mpu.h"
#include <sys/syscall.h>

#define gettid() syscall(__NR_gettid)

#define IS_ELF(ehdr) ((ehdr).e_ident[EI_MAG0] == ELFMAG0 && \
                      (ehdr).e_ident[EI_MAG1] == ELFMAG1 && \
                      (ehdr).e_ident[EI_MAG2] == ELFMAG2 && \
                      (ehdr).e_ident[EI_MAG3] == ELFMAG3)

#define LIBBINDER_PATH "/system/lib/libbinder.so"
#define LIBHARDWARE_PATH "/system/lib/libhardware.so"

#define BIO_F_SHARED    0x01  /* needs to be buffer freed */
#define BIO_F_OVERFLOW  0x02  /* ran out of space */
#define BIO_F_IOERROR   0x04
#define BIO_F_MALLOCED  0x08  /* needs to be free()'d */

char* hook_func_str;

struct microphone_usage {
    int audio_size;
    struct timeval microphone_start_time;
    struct timeval microphone_end_time;
    int uid;
    int is_recording;
}mu;

struct camera_usage {
    int video_size;
    int preview_frame_count;
    struct timeval preview_start_time;
    struct timeval preview_end_time;
    struct timeval take_picture_time;
    struct timeval record_start_time;
    struct timeval record_end_time;
    int uid;
    int is_recording;
    int is_previewing;
    int preview_callback_enabled;
}cu;

struct sensors_usage {
    int handle;

    struct timeval acc_start_time;
    struct timeval acc_end_time;
    int acc_uid;

    struct timeval mag_start_time;
    struct timeval mag_end_time;
    int mag_uid;

    struct timeval prox_start_time;
    struct timeval prox_end_time;
    int prox_uid;

    struct timeval light_start_time;
    struct timeval light_end_time;
    int light_uid;

    struct timeval gyro_start_time;
    struct timeval gyro_end_time;
    int gyro_uid;

    struct timeval press_start_time;
    struct timeval press_end_time;
    int press_uid;
}su;

FILE *fSensorUsage;
mxml_node_t *tree;
mxml_node_t *node;
mxml_node_t *time_start;
mxml_node_t *time_end;
mxml_node_t *usage;
mxml_node_t *uid;
mxml_node_t *package_name;

#define MY_BUFFER_SIZE 1024
char filename_buffer[MY_BUFFER_SIZE] = "";
char pathfile[20];
/*
int get_file_info_from_fd(int fd) {
    int result=-1;
    sprintf(pathfile,"/proc/self/fd/%d",fd);
    memset(filename_buffer,'\0',MY_BUFFER_SIZE);
    result = readlink(pathfile,filename_buffer,sizeof(filename_buffer)) ;
    if (result != -1) {
        LOGD(" filename for %d is: %s",fd,filename_buffer);
    }
    else {
        LOGE(" Error finding filename for %d :   %s",fd,strerror( errno ) ) ;
    }
    return result;
}

void log_fd_info(int file, struct stat fileStat, int mode) {
    time_t clock;
    LOGD("success write %d ",file);
    LOGD("  File Size: \t\t\t\t\t%d bytes",fileStat.st_size);
    LOGD("  Number of Links: \t\t\t\t%d",fileStat.st_nlink);
    LOGD("  File inode: \t\t\t\t\t%d",fileStat.st_ino);
    LOGD("  File UID: \t\t\t\t\t%d",fileStat.st_uid);
    LOGD("  File GID: \t\t\t\t\t%d",fileStat.st_gid);
    LOGD("  File Device ID: \t\t\t\t%d",fileStat.st_dev);
    LOGD("  Protection Mode is: \t\t\t\t%d",fileStat.st_mode);
    LOGD("  Device IS (special file): \t\t\t%d",fileStat.st_rdev);
    LOGD("  blocksize for file system I/O: \t\t%d",fileStat.st_blksize);
    LOGD("  number of 512B blocks allocated: \t\t%d",fileStat.st_blocks);
    clock = fileStat.st_atime;
    LOGD("  time of last access: \t\t\t\t%d \t%s",clock,ctime(&clock));
    clock = fileStat.st_mtime;
    LOGD("  time of last modification: \t\t\t%d \t%s",clock,ctime(&clock));
    clock = fileStat.st_ctime;
    LOGD("  time of last status change: \t\t\t%d \t%s",clock,ctime(&clock));

    switch (fileStat.st_mode & S_IFMT) {
        case S_IFBLK:  LOGD("  %s.....file type.. %d ..block device","w/r",file);            break;
        case S_IFCHR:  LOGD("  %s.....file type.. %d ..character device","w/r",file);        break;
        case S_IFDIR:  LOGD("  %s.....file type.. %d ..directory","w/r",file);               break;
        case S_IFIFO:  LOGD("  %s.....file type.. %d ..FIFO/pipe","w/r",file);               break;
        case S_IFLNK:  LOGD("  %s.....file type.. %d ..symlink","w/r",file);                 break;
        case S_IFREG:  LOGD("  %s.....file type.. %d ..regular file","w/r",file);            break;
        case S_IFSOCK: LOGD("  %s.....file type.. %d ..socket","w/r",file);                  break;
        default:       LOGD("  %s.....file type.. %d ..unknown?","w/r",file);                break;
    }
}

int my_libc_read(int file, void *buffer, size_t length) {
    struct stat fileStat;
    int status;
    char log_info[4096];
    char temp[4096];
    int result = -1;
    sprintf(temp, "read %d, length: %d, tid: %ld, ", file, length, gettid());
    strcpy(log_info, temp);
    sprintf(pathfile,"/proc/self/fd/%d",file);
    memset(filename_buffer,'\0',MY_BUFFER_SIZE);
    result = readlink(pathfile,filename_buffer,sizeof(filename_buffer)) ;
    if (result != -1) {
        sprintf(temp, "filename for %d is: %s, ", file,filename_buffer);
        strcat(log_info, temp);
    }
    else {
        sprintf(temp, "Error finding filename for %d :   %s, ", file, strerror( errno ) ) ;
        strcat(log_info, temp);
    }
    status = read(file, buffer, length);

    int i;
    for (i = 0; i < length; i += 4) {
        sprintf(temp, "buffer[%d]: %x, ", i, *(int*)(buffer + i));
        strcat(log_info, temp);
    }

    LOGD("%s", log_info);

    return status;
}

int my_libc_write(int file, void *buffer, size_t length) {
    LOGD("write %s, tid: %ld", file, gettid());
    get_file_info_from_fd(file);
    int i;
    for (i = 0; i < length; i += 4) {
        LOGD("buffer[%d]: %x", i, *(int*)(buffer + i));
    }
    return write(file, buffer, length);
}

int my_libc_open(const char *file, int flags, int mode) {
    int result;
    LOGD("open %s, tid: %ld", file, gettid());
    return open(file, flags, mode);
}

FILE* my_libc_fopen(char* file, char* mode) {
    FILE* result;
    LOGD("fopen %s, tid: %ld", file, gettid());
    return fopen(file, mode);
}
*/
void bio_init_from_tr(struct binder_io *bio, struct binder_transaction_data *tr)
{
    bio->data = bio->data0 = tr->data.ptr.buffer;
    bio->offs = bio->offs0 = tr->data.ptr.offsets;
    bio->data_avail = tr->data_size;
    bio->offs_avail = tr->offsets_size / 4;
    bio->flags = BIO_F_SHARED;
}

const char *str8(uint16_t *x)
{
    static char buf[128];
    unsigned max = 127;
    char *p = buf;

    if (x) {
        while (*x && max--) {
            *p++ = *x++;
        }
    }
    *p++ = 0;
    return buf;
}

enum {
    CAMERA_PREVIEW,
    CAMERA_TAKE_PICTURE,
    CAMERA_RECORD,
    MICROPHONE,
    GPS,
    SENSORS,
};

//IAudioRecord
enum {
    GET_CBLK = 1,
    AUDIO_RECORD_START,
    AUDIO_RECORD_STOP
};

//IMediaRecorder
enum {
    RELEASE = 1,
    INIT,
    CLOSE,
    QUERY_SURFACE_MEDIASOURCE,
    RESET,
    MEDIA_RECORDER_STOP,
    MEDIA_RECORDER_START,
    PREPARE,
    GET_MAX_AMPLITUDE,
    SET_VIDEO_SOURCE,
    SET_AUDIO_SOURCE,
    SET_OUTPUT_FORMAT,
    SET_VIDEO_ENCODER,
    SET_AUDIO_ENCODER,
    SET_OUTPUT_FILE_PATH,
    SET_OUTPUT_FILE_FD,
    SET_VIDEO_SIZE,
    SET_VIDEO_FRAMERATE,
    MEDIA_RECORDER_SET_PARAMETERS,
    SET_PREVIEW_SURFACE,
    SET_CAMERA,
    SET_LISTENER,
};

int media_recorder_audio;
int media_recorder_video;

//ICamera
enum {
    DISCONNECT = 1,
    SET_PREVIEW_DISPLAY,
    SET_PREVIEW_TEXTURE,
    SET_PREVIEW_CALLBACK_FLAG,
    START_PREVIEW,
    STOP_PREVIEW,
    AUTO_FOCUS,
    CANCEL_AUTO_FOCUS,
    TAKE_PICTURE,
    SET_PARAMETERS,
    GET_PARAMETERS,
    SEND_COMMAND,
    CONNECT,
    LOCK,
    UNLOCK,
    PREVIEW_ENABLED,
    START_RECORDING,
    STOP_RECORDING,
    RECORDING_ENABLED,
    RELEASE_RECORDING_FRAME,
    STORE_META_DATA_IN_BUFFERS,
};

enum {
    CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK = 0x01,
    CAMERA_FRAME_CALLBACK_FLAG_ONE_SHOT_MASK = 0x02,
    CAMERA_FRAME_CALLBACK_FLAG_COPY_OUT_MASK = 0x04,
    /** Typical use cases */
    CAMERA_FRAME_CALLBACK_FLAG_NOOP = 0x00,
    CAMERA_FRAME_CALLBACK_FLAG_CAMCORDER = 0x01,
    CAMERA_FRAME_CALLBACK_FLAG_CAMERA = 0x05,
    CAMERA_FRAME_CALLBACK_FLAG_BARCODE_SCANNER = 0x07
};

//ILocationManager
enum {
    REQUEST_LOCATION_UPDATES = 1,
    REMOVE_UPDATES,
};

//ISensorEventConnection
enum {
    GET_SENSOR_CHANNEL = 1,
    ENABLE_DISABLE,
    SET_EVENT_RATE
};

#define ACCELEROMETER   0
#define MAGNETIC        1
#define PROXIMITY       4
#define LIGHT           5
#define GYROSCOPE       6
#define PRESSURE        7

const char * whitespace_cb(mxml_node_t *node, int where) {
    return "\n";
}

const char* get_package_by_uid(int uid) {
    mxml_node_t *packages;
    mxml_node_t *package;
    char strUid[8];
    FILE* fPackageList = NULL;
    if (NULL == (fPackageList = fopen("/data/system/packages.xml", "r"))) {
        LOGD("Open file failed: %s.\n", strerror(errno));
    }
    packages = mxmlLoadFile(NULL, fPackageList, MXML_TEXT_CALLBACK);
    fclose(fPackageList);
    sprintf(strUid, "%d", uid);
    package = mxmlFindElement(packages, packages, "package", "userId", strUid, MXML_DESCEND);
    return mxmlElementGetAttr(package, "name");
}

void update_xml(int update_type) {
    if (NULL == (fSensorUsage = fopen("/data/senDroid/sensor_usage.xml", "r"))) {
        LOGD("Open file failed: %s.\n", strerror(errno));
    }
    tree = mxmlLoadFile(NULL, fSensorUsage, MXML_TEXT_CALLBACK);
    fclose(fSensorUsage);
    switch (update_type) {
        case CAMERA_PREVIEW:
            node = mxmlFindElement(tree, tree, "camera", NULL, NULL, MXML_DESCEND);
            node = mxmlNewElement(node, "preview");
            time_start = mxmlNewElement(node, "time_start");
            mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&cu.preview_start_time.tv_sec), cu.preview_start_time.tv_usec);
            time_end = mxmlNewElement(node, "time_end");
            mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&cu.preview_end_time.tv_sec), cu.preview_end_time.tv_usec);
            usage = mxmlNewElement(node, "preview_callback");
            mxmlNewInteger(usage, cu.preview_callback_enabled);
            usage = mxmlNewElement(node, "preview_frame_count");
            mxmlNewInteger(usage, cu.preview_frame_count);
            uid = mxmlNewElement(node, "uid");
            mxmlNewInteger(uid, cu.uid);
            package_name = mxmlNewElement(node, "package_name");
            LOGD("Package name: %s", get_package_by_uid(cu.uid));
            mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(cu.uid));
            cu.video_size = 0;
            cu.preview_frame_count = 0;
            cu.preview_callback_enabled = 0;
            break;
        case CAMERA_TAKE_PICTURE:
            node = mxmlFindElement(tree, tree, "camera", NULL, NULL, MXML_DESCEND);
            node = mxmlNewElement(node, "take_picture");
            time_start = mxmlNewElement(node, "time");
            mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&cu.take_picture_time.tv_sec), cu.take_picture_time.tv_usec);
            uid = mxmlNewElement(node, "uid");
            mxmlNewInteger(uid, cu.uid);
            package_name = mxmlNewElement(node, "package_name");
            LOGD("Package name: %s", get_package_by_uid(cu.uid));
            mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(cu.uid));
            break;
        case CAMERA_RECORD:
            node = mxmlFindElement(tree, tree, "camera", NULL, NULL, MXML_DESCEND);
            node = mxmlNewElement(node, "record");
            time_start = mxmlNewElement(node, "time_start");
            mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&cu.record_start_time.tv_sec), cu.record_start_time.tv_usec);
            time_end = mxmlNewElement(node, "time_end");
            mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&cu.record_end_time.tv_sec), cu.record_end_time.tv_usec);
            usage = mxmlNewElement(node, "usage_video");
            mxmlNewInteger(usage, cu.video_size);
            uid = mxmlNewElement(node, "uid");
            mxmlNewInteger(uid, cu.uid);
            package_name = mxmlNewElement(node, "package_name");
            LOGD("Package name: %s", get_package_by_uid(cu.uid));
            mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(cu.uid));
            cu.video_size = 0;
            break;
        case MICROPHONE:
            node = mxmlFindElement(tree, tree, "microphone", NULL, NULL, MXML_DESCEND);
            node = mxmlNewElement(node, "record");
            time_start = mxmlNewElement(node, "time_start");
            mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&mu.microphone_start_time.tv_sec), mu.microphone_start_time.tv_usec);
            time_end = mxmlNewElement(node, "time_end");
            mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&mu.microphone_end_time.tv_sec), mu.microphone_end_time.tv_usec);
            usage = mxmlNewElement(node, "usage");
            mxmlNewInteger(usage, mu.audio_size);
            uid = mxmlNewElement(node, "uid");
            mxmlNewInteger(uid, mu.uid);
            package_name = mxmlNewElement(node, "package_name");
            LOGD("Package name: %s", get_package_by_uid(mu.uid));
            mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(mu.uid));
            mu.audio_size = 0;
            break;
        case GPS:
            node = mxmlFindElement(tree, tree, "gps", NULL, NULL, MXML_DESCEND);
            node = mxmlNewElement(node, "access");
            time_start = mxmlNewElement(node, "time_start");
            mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&gu.gps_start_time.tv_sec), gu.gps_start_time.tv_usec);
            time_end = mxmlNewElement(node, "time_end");
            mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&gu.gps_end_time.tv_sec), gu.gps_end_time.tv_usec);
            usage = mxmlNewElement(node, "usage");
            mxmlNewInteger(usage, gu.gps_count);
            uid = mxmlNewElement(node, "uid");
            mxmlNewInteger(uid, gu.uid);
            package_name = mxmlNewElement(node, "package_name");
            LOGD("Package name: %s", get_package_by_uid(gu.uid));
            mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(gu.uid));
            break;
        case SENSORS:
            node = mxmlFindElement(tree, tree, "standard_sensors", NULL, NULL, MXML_DESCEND);
            switch (su.handle) {
                case ACCELEROMETER:
                    node = mxmlNewElement(node, "accelerometer");
                    time_start = mxmlNewElement(node, "time_start");
                    mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&su.acc_start_time.tv_sec), su.acc_start_time.tv_usec);
                    time_end = mxmlNewElement(node, "time_end");
                    mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&su.acc_end_time.tv_sec), su.acc_end_time.tv_usec);
                    uid = mxmlNewElement(node, "uid");
                    mxmlNewInteger(uid, su.acc_uid);
                    package_name = mxmlNewElement(node, "package_name");
                    LOGD("Package name: %s", get_package_by_uid(su.acc_uid));
                    mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(su.acc_uid));
                    break;
                case MAGNETIC:
                    node = mxmlNewElement(node, "magnetic");
                    time_start = mxmlNewElement(node, "time_start");
                    mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&su.mag_start_time.tv_sec), su.mag_start_time.tv_usec);
                    time_end = mxmlNewElement(node, "time_end");
                    mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&su.mag_end_time.tv_sec), su.mag_end_time.tv_usec);
                    uid = mxmlNewElement(node, "uid");
                    mxmlNewInteger(uid, su.mag_uid);
                    package_name = mxmlNewElement(node, "package_name");
                    LOGD("Package name: %s", get_package_by_uid(su.mag_uid));
                    mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(su.mag_uid));
                    break;
                case PROXIMITY:
                    node = mxmlNewElement(node, "proximity");
                    time_start = mxmlNewElement(node, "time_start");
                    mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&su.prox_start_time.tv_sec), su.prox_start_time.tv_usec);
                    time_end = mxmlNewElement(node, "time_end");
                    mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&su.prox_end_time.tv_sec), su.prox_end_time.tv_usec);
                    uid = mxmlNewElement(node, "uid");
                    mxmlNewInteger(uid, su.prox_uid);
                    package_name = mxmlNewElement(node, "package_name");
                    LOGD("Package name: %s", get_package_by_uid(su.prox_uid));
                    mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(su.prox_uid));
                    break;
                case LIGHT:
                    node = mxmlNewElement(node, "light");
                    time_start = mxmlNewElement(node, "time_start");
                    mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&su.light_start_time.tv_sec), su.light_start_time.tv_usec);
                    time_end = mxmlNewElement(node, "time_end");
                    mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&su.light_end_time.tv_sec), su.light_end_time.tv_usec);
                    uid = mxmlNewElement(node, "uid");
                    mxmlNewInteger(uid, su.light_uid);
                    package_name = mxmlNewElement(node, "package_name");
                    LOGD("Package name: %s", get_package_by_uid(su.light_uid));
                    mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(su.light_uid));
                    break;
                case GYROSCOPE:
                    node = mxmlNewElement(node, "gyroscope");
                    time_start = mxmlNewElement(node, "time_start");
                    mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&su.gyro_start_time.tv_sec), su.gyro_start_time.tv_usec);
                    time_end = mxmlNewElement(node, "time_end");
                    mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&su.gyro_end_time.tv_sec), su.gyro_end_time.tv_usec);
                    uid = mxmlNewElement(node, "uid");
                    mxmlNewInteger(uid, su.gyro_uid);
                    package_name = mxmlNewElement(node, "package_name");
                    LOGD("Package name: %s", get_package_by_uid(su.gyro_uid));
                    mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(su.gyro_uid));
                    break;
                case PRESSURE:
                    node = mxmlNewElement(node, "pressure");
                    time_start = mxmlNewElement(node, "time_start");
                    mxmlNewTextf(time_start, 0, "%s.%06d", ctime(&su.press_start_time.tv_sec), su.press_start_time.tv_usec);
                    time_end = mxmlNewElement(node, "time_end");
                    mxmlNewTextf(time_end, 0, "%s.%06d", ctime(&su.press_end_time.tv_sec), su.press_end_time.tv_usec);
                    uid = mxmlNewElement(node, "uid");
                    mxmlNewInteger(uid, su.press_uid);
                    package_name = mxmlNewElement(node, "package_name");
                    LOGD("Package name: %s", get_package_by_uid(su.press_uid));
                    mxmlNewTextf(package_name, 0, "%s", get_package_by_uid(su.press_uid));
                    break;
                default:
                    break;
            }
        default:
            break;
    }
    if (NULL == (fSensorUsage = fopen("/data/senDroid/sensor_usage.xml", "w"))) {
        LOGD("Open file failed: %s.\n", strerror(errno));
    }
    mxmlSaveFile(tree, fSensorUsage, MXML_NO_CALLBACK);
    fclose(fSensorUsage);
}

int handler(struct binder_txn *txn, struct binder_io *msg)
{
    uint16_t *s;
    unsigned len;
    int i, j;
    char c;
    char name[256];
    char handle, enabled, cb_flag;

    //LOGD("code: %d", txn->code);
    len = *(msg->data + 4);
    i = 8;
    j = 0;
    while (len) {
        name[j++] = *(msg->data + i);
        i += 2;
        len --;
    }
    name[j] = '\0';
    //LOGD("name: %s", name);

    if (strcmp(name, "android.hardware.ICamera") == 0) {
        switch (txn->code) {
            case START_PREVIEW:
                LOGD("is_previewing: %d", cu.is_previewing);
                cu.is_previewing = 1;
                LOGD("is_previewing: %d", cu.is_previewing);
                cu.uid = txn->sender_euid;
                gettimeofday(&cu.preview_start_time, 0);
                LOGD("preview_start_time: %s.%06d", ctime(&cu.preview_start_time.tv_sec), cu.preview_start_time.tv_usec);
                break;
            case STOP_PREVIEW:
                if (cu.is_previewing) {
                    LOGD("is_previewing: %d", cu.is_previewing);
                    cu.is_previewing = 0;
                    LOGD("is_previewing: %d", cu.is_previewing);
                    cu.uid = txn->sender_euid;
                    gettimeofday(&cu.preview_end_time, 0);
                    LOGD("preview_end_time: %s.%06d", ctime(&cu.preview_end_time.tv_sec), cu.preview_end_time.tv_usec);
                    update_xml(CAMERA_PREVIEW);
                }
                break;
            case TAKE_PICTURE:
                cu.uid = txn->sender_euid;
                gettimeofday(&cu.take_picture_time, 0);
                LOGD("take_picture_time: %s.%06d", ctime(&cu.take_picture_time.tv_sec), cu.take_picture_time.tv_usec);
                update_xml(CAMERA_TAKE_PICTURE);
                break;
            case START_RECORDING:
                cu.video_size = 0;
                LOGD("is_recording: %d", cu.is_recording);
                cu.is_recording = 1;
                LOGD("is_recording: %d", cu.is_recording);
                cu.uid = txn->sender_euid;
                gettimeofday(&cu.record_start_time, 0);
                LOGD("record_start_time: %s.%06d", ctime(&cu.record_start_time.tv_sec), cu.record_start_time.tv_usec);
                break;
            case STOP_RECORDING:
                if (cu.is_recording) {
                    LOGD("is_recording: %d", cu.is_recording);
                    cu.is_recording = 0;
                    LOGD("is_recording: %d", cu.is_recording);
                    cu.uid = txn->sender_euid;
                    gettimeofday(&cu.record_end_time, 0);
                    LOGD("record_end_time: %s.%06d", ctime(&cu.record_end_time.tv_sec), cu.record_end_time.tv_usec);
                    update_xml(CAMERA_RECORD);
                    mu.audio_size = 0;
                }
                break;
            case DISCONNECT:
                if (cu.is_previewing) {
                    cu.is_previewing = 0;
                    cu.uid = txn->sender_euid;
                    gettimeofday(&cu.preview_end_time, 0);
                    LOGD("preview_end_time: %s.%06d", ctime(&cu.preview_end_time.tv_sec), cu.preview_end_time.tv_usec);
                    update_xml(CAMERA_PREVIEW);
                } else if (cu.is_recording) {
                    cu.is_recording = 0;
                    cu.uid = txn->sender_euid;
                    gettimeofday(&cu.record_end_time, 0);
                    LOGD("record_end_time: %s.%06d", ctime(&cu.record_end_time.tv_sec), cu.record_end_time.tv_usec);
                    update_xml(CAMERA_RECORD);
                }
                break;
            case SET_PREVIEW_CALLBACK_FLAG:
                cb_flag = *(msg->data + i + 4);
                if (cb_flag == CAMERA_FRAME_CALLBACK_FLAG_CAMERA ||
                    cb_flag == CAMERA_FRAME_CALLBACK_FLAG_BARCODE_SCANNER) {
                    cu.preview_callback_enabled = 1;
                }
                break;
            default:
                break;
        }
    } else if (strcmp(name, "android.media.IAudioRecord") == 0) {
        switch (txn->code) {
            case AUDIO_RECORD_START:
                LOGD("is_recording: %d", mu.is_recording);
                mu.is_recording = 1;
                LOGD("is_recording: %d", mu.is_recording);
                mu.uid = txn->sender_euid;
                gettimeofday(&mu.microphone_start_time, 0);
                LOGD("microphone_start_time: %s.%06d", ctime(&mu.microphone_start_time.tv_sec), mu.microphone_start_time.tv_usec);
                break;
            case AUDIO_RECORD_STOP:
                if (mu.is_recording) {
                    LOGD("is_recording: %d", mu.is_recording);
                    mu.is_recording = 0;
                    LOGD("is_recording: %d", mu.is_recording);
                    mu.uid = txn->sender_euid;
                    gettimeofday(&mu.microphone_end_time, 0);
                    LOGD("microphone_end_time: %s.%06d", ctime(&mu.microphone_end_time.tv_sec), mu.microphone_end_time.tv_usec);
                    update_xml(MICROPHONE);
                }
                break;
            default:
                break;
        }
    } else if (strcmp(name, "android.media.IMediaRecorder") == 0) {
        switch (txn->code) {
            case MEDIA_RECORDER_START:
                if (media_recorder_audio) {
                    mu.is_recording = 1;
                    mu.uid = txn->sender_euid;
                    gettimeofday(&mu.microphone_start_time, 0);
                    LOGD("microphone_start_time: %s.%06d", ctime(&mu.microphone_start_time.tv_sec), mu.microphone_start_time.tv_usec);
                }
                if (media_recorder_video) {
                    cu.is_recording = 1;
                    cu.uid = txn->sender_euid;
                    gettimeofday(&cu.record_start_time, 0);
                    LOGD("record_start_time: %s.%06d", ctime(&cu.record_start_time.tv_sec), cu.record_start_time.tv_usec);
                }
                break;
            case MEDIA_RECORDER_STOP:
                if (mu.is_recording) {
                    mu.is_recording = 0;
                    mu.uid = txn->sender_euid;
                    gettimeofday(&mu.microphone_end_time, 0);
                    LOGD("microphone_end_time: %s.%06d", ctime(&mu.microphone_end_time.tv_sec), mu.microphone_end_time.tv_usec);
                    update_xml(MICROPHONE);
                    media_recorder_audio = 0;
                }
                if (cu.is_recording) {
                    cu.is_recording = 0;
                    cu.uid = txn->sender_euid;
                    gettimeofday(&cu.record_end_time, 0);
                    LOGD("record_end_time: %s.%06d", ctime(&cu.record_end_time.tv_sec), cu.record_end_time.tv_usec);
                    update_xml(CAMERA_RECORD);
                    media_recorder_video = 0;
                }
                break;
            case SET_AUDIO_SOURCE:
                media_recorder_audio = 1;
                break;
            case SET_VIDEO_SOURCE:
                media_recorder_video = 1;
                break;
            default:
                break;
        }
    } else if (strcmp(name, "android.location.ILocationManager") == 0) {
        switch (txn->code) {
            case REQUEST_LOCATION_UPDATES:
                gu.uid = txn->sender_euid;
                gu.gps_count = 0;
                gettimeofday(&gu.gps_start_time, 0);
                LOGD("gps_start_time: %s.%06d", ctime(&gu.gps_start_time.tv_sec), gu.gps_start_time.tv_usec);
                break;
            case REMOVE_UPDATES:
                gettimeofday(&gu.gps_end_time, 0);
                LOGD("gps_end_time: %s.%06d", ctime(&gu.gps_end_time.tv_sec), gu.gps_end_time.tv_usec);
                update_xml(GPS);
                break;
            default:
                break;
        }
    } else if (strcmp(name, "android.gui.SensorEventConnection") == 0) {
        switch (txn->code) {
            case ENABLE_DISABLE:
                handle = *(msg->data + i + 2);
                enabled = *(msg->data + i + 6);
                LOGD("handle: %d, enabled: %d", handle, enabled);
                switch (handle) {
                    case ACCELEROMETER:
                        if (enabled) {
                            su.acc_uid = txn->sender_euid;
                            gettimeofday(&su.acc_start_time, 0);
                            LOGD("acc_start_time: %s.%06d", ctime(&su.acc_start_time.tv_sec), su.acc_start_time.tv_usec);
                        } else {
                            su.handle = ACCELEROMETER;
                            gettimeofday(&su.acc_end_time, 0);
                            LOGD("acc_end_time: %s.%06d", ctime(&su.acc_end_time.tv_sec), su.acc_end_time.tv_usec);
                            update_xml(SENSORS);
                        }
                        break;
                    case MAGNETIC:
                        if (enabled) {
                            su.mag_uid = txn->sender_euid;
                            gettimeofday(&su.mag_start_time, 0);
                            LOGD("mag_start_time: %s.%06d", ctime(&su.mag_start_time.tv_sec), su.mag_start_time.tv_usec);
                        } else {
                            su.handle = MAGNETIC;
                            gettimeofday(&su.mag_end_time, 0);
                            LOGD("mag_end_time: %s.%06d", ctime(&su.mag_end_time.tv_sec), su.mag_end_time.tv_usec);
                            update_xml(SENSORS);
                        }
                        break;
                    case PROXIMITY:
                        if (enabled) {
                            su.prox_uid = txn->sender_euid;
                            gettimeofday(&su.prox_start_time, 0);
                            LOGD("prox_start_time: %s.%06d", ctime(&su.prox_start_time.tv_sec), su.prox_start_time.tv_usec);
                        } else {
                            su.handle = PROXIMITY;
                            gettimeofday(&su.prox_end_time, 0);
                            LOGD("prox_end_time: %s.%06d", ctime(&su.prox_end_time.tv_sec), su.prox_end_time.tv_usec);
                            update_xml(SENSORS);
                        }
                        break;
                    case LIGHT:
                        if (enabled) {
                            su.light_uid = txn->sender_euid;
                            gettimeofday(&su.light_start_time, 0);
                            LOGD("light_start_time: %s.%06d", ctime(&su.light_start_time.tv_sec), su.light_start_time.tv_usec);
                        } else {
                            su.handle = LIGHT;
                            gettimeofday(&su.light_end_time, 0);
                            LOGD("light_end_time: %s.%06d", ctime(&su.light_end_time.tv_sec), su.light_end_time.tv_usec);
                            update_xml(SENSORS);
                        }
                        break;
                    case GYROSCOPE:
                        if (enabled) {
                            su.gyro_uid = txn->sender_euid;
                            gettimeofday(&su.gyro_start_time, 0);
                            LOGD("gyro_start_time: %s.%06d", ctime(&su.gyro_start_time.tv_sec), su.gyro_start_time.tv_usec);
                        } else {
                            su.handle = GYROSCOPE;
                            gettimeofday(&su.gyro_end_time, 0);
                            LOGD("gyro_end_time: %s.%06d", ctime(&su.gyro_end_time.tv_sec), su.gyro_end_time.tv_usec);
                            update_xml(SENSORS);
                        }
                        break;
                    case PRESSURE:
                        if (enabled) {
                            su.press_uid = txn->sender_euid;
                            gettimeofday(&su.press_start_time, 0);
                            LOGD("press_start_time: %s.%06d", ctime(&su.press_start_time.tv_sec), su.press_start_time.tv_usec);
                        } else {
                            su.handle = PRESSURE;
                            gettimeofday(&su.press_end_time, 0);
                            LOGD("press_end_time: %s.%06d", ctime(&su.press_end_time.tv_sec), su.press_end_time.tv_usec);
                            update_xml(SENSORS);
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    return 0;
}

int my_ioctl (int __fd, unsigned long int __request, void* arg) {
    int i;
    int result;
    int write_size;
    uint16_t *s;
    unsigned len;
    struct v4l2_capability* cap;
    struct v4l2_format* fmt;
    struct v4l2_requestbuffers* reqbuf;
    struct v4l2_buffer* buffer;
    struct v4l2_plane* plane;
    struct v4l2_streamparm* parm;
    struct v4l2_control* ctl;
    struct binder_write_read* bwr;
    struct binder_io bio;
    struct binder_transaction_data* tr;
    unsigned long int type_command;
    unsigned long int type_return;
    struct snd_xferi* sxi;
    //LOGD("tid: %ld", gettid());
    //get_file_info_from_fd(__fd);
    //LOGD("tid: %ld, fd: %d, request: %x (dir: %x, type: %x, nr: %d, size: %x)", gettid(), __fd, __request, _IOC_DIR(__request), _IOC_TYPE(__request), _IOC_NR(__request), _IOC_SIZE(__request));

    switch (_IOC_TYPE(__request)) {

        case 'V':
            LOGD("nr: %d", _IOC_NR(__request));
            //get_file_info_from_fd(__fd);
            switch (__request) {
                case VIDIOC_QUERYCAP:
                    LOGD("Query capability");
                    result = ioctl(__fd, __request, arg);
                    cap = (struct v4l2_capability*) arg;
                    LOGD("  driver: %s\n", cap->driver);
                    LOGD("  card: %s\n", cap->card);
                    LOGD("  bus_info: %s\n", cap->bus_info);
                    LOGD("  version: %08X\n", cap->version);
                    LOGD("  capabilities: %08X\n", cap->capabilities);
                    return result;
                    break;
                case VIDIOC_ENUM_FMT:
                    LOGD("Enum format");
                    break;
                case VIDIOC_G_FMT:
                    LOGD("Get format");
                    result = ioctl(__fd, __request, arg);
                    fmt = (struct v4l2_format*) arg;
                    LOGD("  type: %d\n", fmt->type);
                    LOGD("  width: %d\n", fmt->fmt.pix.width);
                    LOGD("  height: %d\n", fmt->fmt.pix.height);
                    char fmtstr[8];
                    memset(fmtstr, 0, 8);
                    memcpy(fmtstr, &fmt->fmt.pix.pixelformat, 4);
                    LOGD("  pixelformat: %s\n", fmtstr);
                    LOGD("  field: %d\n", fmt->fmt.pix.field);
                    LOGD("  bytesperline: %d\n", fmt->fmt.pix.bytesperline);
                    LOGD("  sizeimage: %d\n", fmt->fmt.pix.sizeimage);
                    LOGD("  colorspace: %d\n", fmt->fmt.pix.colorspace);
                    LOGD("  priv: %d\n", fmt->fmt.pix.priv);
                    LOGD("  raw_date: %s\n", fmt->fmt.raw_data);
                    return result;
                    break;
                case VIDIOC_S_FMT:
                    LOGD("Set format:");
                    break;
                case VIDIOC_REQBUFS:
                    LOGD("Require buffers");
                    reqbuf = (struct v4l2_requestbuffers*) arg;
                    LOGD("  type:%d, memory:%d, count:%d", reqbuf->type, reqbuf->memory, reqbuf->count);
                    break;
                case VIDIOC_QUERYBUF:
                    LOGD("Query buffer");
                    break;
                case VIDIOC_G_FBUF:
                    LOGD("Get frame buffer");
                    break;
                case VIDIOC_S_FBUF:
                    LOGD("Set frame buffer");
                    break;
                case VIDIOC_OVERLAY:
                    LOGD("Overlay");
                    break;
                case VIDIOC_QBUF:
                    //buffer = (struct v4l2_buffer*) arg;
                    //LOGD("Queue buffer [type:%d, offset:%x, length:%d]", buffer->type, buffer->m.offset, buffer->length);
                    break;
                case VIDIOC_DQBUF:
                    LOGD("Dequeue buffer");
                    result = ioctl(__fd, __request, arg);
                    buffer = (struct v4l2_buffer*) arg;
                    //LOGD("sizeof(struct v4l2_buffer): %x", sizeof(struct v4l2_buffer));
                    //LOGD("type: %d", buffer->type);
                    if (buffer->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
                        for (i = 0; i < buffer->length; i ++) {
                            plane = buffer->m.planes + i;
                            LOGD("Dequeue buffer %d: plane[%d][type:%d, memory:%d, bytesused:%d, length:%d, mem_offset: %x, userptr:%x, fd: %x, data_offset:%x]", buffer->index, i, buffer->type, buffer->memory, plane->bytesused, plane->length, plane->m.mem_offset, plane->m.userptr, plane->m.fd, plane->data_offset);
                            cu.video_size += plane->bytesused - plane->data_offset;
                            cu.preview_frame_count ++;
                            LOGD("video_size: %d", cu.video_size);
                            LOGD("preview_frame_count: %d", cu.preview_frame_count);
                        }
                    } else if (buffer->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
                        LOGD("Dequeue buffer %d [type:%d, memory:%d, bytesused:%d, length:%d, userptr:%x, offset:%x]", buffer->index, buffer->type, buffer->memory, buffer->bytesused, buffer->length, buffer->m.userptr, buffer->m.offset);
                        cu.video_size += buffer->bytesused;
                        cu.preview_frame_count ++;
                        LOGD("video_size: %d", cu.video_size);
                        LOGD("preview_frame_count: %d", cu.preview_frame_count);
                    }
                    return result;
                    break;
                case VIDIOC_STREAMON:
                    LOGD("Stream on");
                    break;
                case VIDIOC_STREAMOFF:
                    LOGD("Stream off");
                    break;
                case VIDIOC_G_PARM:
                    LOGD("VIDIOC_G_PARM");
                    break;
                case VIDIOC_S_PARM:
                    LOGD("VIDIOC_S_PARM");
                    parm = (struct v4l2_streamparm*) arg;
                    LOGD("  type:%d", parm->type);
                    LOGD("  captureparm:");
                    LOGD("      capability:%d, capturemode:%d, extendedmode:%d, readbuffers:%d", parm->parm.capture.capability, parm->parm.capture.capturemode, parm->parm.capture.extendedmode, parm->parm.capture.readbuffers);
                    break;
                case VIDIOC_G_STD:
                    LOGD("VIDIOC_G_STD");
                    break;
                case VIDIOC_S_STD:
                    LOGD("VIDIOC_S_STD");
                    break;
                case VIDIOC_ENUMSTD:
                    LOGD("VIDIOC_ENUMSTD");
                    break;
                case VIDIOC_ENUMINPUT:
                    LOGD("VIDIOC_ENUMINPUT");
                    break;
                case VIDIOC_G_CTRL:
                    LOGD("VIDIOC_G_CTRL");
                    break;
                case VIDIOC_S_CTRL:
                    LOGD("VIDIOC_S_CTRL");
                    ctl = (struct v4l2_control*) arg;
                    LOGD("  id:%d, value:%d", ctl->id, ctl->value);
                    break;
                case VIDIOC_G_PRIORITY:
                    LOGD("VIDIOC_G_PRIORITY");
                    break;
                case VIDIOC_S_PRIORITY:
                    LOGD("VIDIOC_S_PRIORITY");
                    break;
                default:
                    break;
            }
            break;

        case 'A':
            //get_file_info_from_fd(__fd);
            switch(__request) {
                case SNDRV_PCM_IOCTL_PVERSION:
                    LOGD("SNDRV_PCM_IOCTL_PVERSION");
                    break;
                case SNDRV_PCM_IOCTL_INFO:
                    LOGD("SNDRV_PCM_IOCTL_INFO");
                    break;
                case SNDRV_PCM_IOCTL_TSTAMP:
                    LOGD("SNDRV_PCM_IOCTL_TSTAMP");
                    break;
                case SNDRV_PCM_IOCTL_TTSTAMP:
                    LOGD("SNDRV_PCM_IOCTL_TTSTAMP");
                    break;
                case SNDRV_PCM_IOCTL_HW_REFINE:
                    LOGD("SNDRV_PCM_IOCTL_HW_REFINE");
                    break;
                case SNDRV_PCM_IOCTL_HW_PARAMS:
                    LOGD("SNDRV_PCM_IOCTL_HW_PARAMS");
                    break;
                case SNDRV_PCM_IOCTL_HW_FREE:
                    LOGD("SNDRV_PCM_IOCTL_HW_FREE");
                    break;
                case SNDRV_PCM_IOCTL_SW_PARAMS:
                    LOGD("SNDRV_PCM_IOCTL_SW_PARAMS");
                    break;
                case SNDRV_PCM_IOCTL_STATUS:
                    LOGD("SNDRV_PCM_IOCTL_STATUS");
                    break;
                case SNDRV_PCM_IOCTL_DELAY:
                    LOGD("SNDRV_PCM_IOCTL_DELAY");
                    break;
                case SNDRV_PCM_IOCTL_HWSYNC:
                    LOGD("SNDRV_PCM_IOCTL_HWSYNC");
                    break;
                case SNDRV_PCM_IOCTL_SYNC_PTR:
                    LOGD("SNDRV_PCM_IOCTL_SYNC_PTR");
                    break;
                case SNDRV_PCM_IOCTL_CHANNEL_INFO:
                    LOGD("SNDRV_PCM_IOCTL_CHANNEL_INFO");
                    break;
                case SNDRV_PCM_IOCTL_PREPARE:
                    LOGD("SNDRV_PCM_IOCTL_PREPARE");
                    break;
                case SNDRV_PCM_IOCTL_RESET:
                    LOGD("SNDRV_PCM_IOCTL_RESET");
                    break;
                case SNDRV_PCM_IOCTL_START:
                    LOGD("SNDRV_PCM_IOCTL_START");
                    break;
                case SNDRV_PCM_IOCTL_DROP:
                    LOGD("SNDRV_PCM_IOCTL_DROP");
                    break;
                case SNDRV_PCM_IOCTL_DRAIN:
                    LOGD("SNDRV_PCM_IOCTL_DRAIN");
                    break;
                case SNDRV_PCM_IOCTL_PAUSE:
                    LOGD("SNDRV_PCM_IOCTL_PAUSE");
                    break;
                case SNDRV_PCM_IOCTL_REWIND:
                    LOGD("SNDRV_PCM_IOCTL_REWIND");
                    break;
                case SNDRV_PCM_IOCTL_RESUME:
                    LOGD("SNDRV_PCM_IOCTL_RESUME");
                    break;
                case SNDRV_PCM_IOCTL_XRUN:
                    LOGD("SNDRV_PCM_IOCTL_XRUN");
                    break;
                case SNDRV_PCM_IOCTL_FORWARD:
                    LOGD("SNDRV_PCM_IOCTL_FORWARD");
                    break;
                case SNDRV_PCM_IOCTL_WRITEI_FRAMES:
                    //LOGD("SNDRV_PCM_IOCTL_WRITEI_FRAMES");
                    break;
                case SNDRV_PCM_IOCTL_READI_FRAMES:
                    LOGD("SNDRV_PCM_IOCTL_READI_FRAMES");
                    result = ioctl(__fd, __request, arg);
                    sxi = (struct snd_xferi*)arg;
                    LOGD("result: %d, frames: %d", sxi->result, sxi->frames);
                    mu.audio_size += sxi->frames * 4;
                    LOGD("audio_size: %d", mu.audio_size);
                    return result;
                    break;
                case SNDRV_PCM_IOCTL_WRITEN_FRAMES:
                    LOGD("SNDRV_PCM_IOCTL_WRITEN_FRAMES");
                    break;
                case SNDRV_PCM_IOCTL_READN_FRAMES:
                    LOGD("SNDRV_PCM_IOCTL_READN_FRAMES");
                    break;
                case SNDRV_PCM_IOCTL_LINK:
                    LOGD("SNDRV_PCM_IOCTL_LINK");
                    break;
                case SNDRV_PCM_IOCTL_UNLINK:
                    LOGD("SNDRV_PCM_IOCTL_UNLINK");
                    break;
                default:
                    break;
            }
            break;

        case 'b':

            switch (__request) {
                case BINDER_WRITE_READ:

                    result = ioctl(__fd, __request, arg);
                    bwr = (struct binder_write_read*) arg;

                    binder_parse(0, bwr->read_buffer, bwr->read_consumed, handler);

                    return result;
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
        //get_file_info_from_fd(__fd);
    }
    return ioctl(__fd, __request, arg);
}
/*
void *my_memcpy(void *dest, const void *src, size_t n) {
    struct binder_transaction_data* tr;
    size_t size = sizeof(struct binder_transaction_data);
    //LOGD("size:%d, n:%d", size, n);
    memcpy(dest, src, n);
    if (n == size) {
        tr = (struct binder_transaction_data*) src;
        LOGD("code:%x, sender_pid:%d, sender_euid:%d", tr->code, tr->sender_pid, tr->sender_euid);
    }
}
*/
void *my_bio_get_ref(struct binder_io *bio) {
    uint16_t *s;
    unsigned len;
    void *ptr;

    ptr = bio_get_ref(bio);
    if (ptr) {
        s = bio_get_string16(bio, &len);
        LOGD("name: %s", str8(s));
        LOGD("ptr: %x", ptr);
    }
    return ptr;
}

/*
int my_memcmp(const void *buf1, const void *buf2, unsigned int count) {
    int i, j;
    char name[128], c;
    for (i = 0, j = 0; i < count; ++i) {
        c = *((char*)(buf1+i));
        if (c) name[j++] = c;
    }
    name[j] = '\0';
    LOGD("name: %s", name);
    return memcmp(buf1, buf2, count);
}
*/
extern msq_q_err_type my_msg_q_rcv(void* msg_q_data, void** msg_obj);

/* Hook functions  */
struct hook_func {
    const char* so_filename;
    const char* func_name;
    void * original_ptr;
    void * hook_ptr;
};

struct hook_func hook_funcs[] = {
//      {"",        "bio_get_ref",  NULL,   (void*)my_bio_get_ref},
//      {"",        "memcmp",       NULL,   (void*)my_memcmp},
//      {"",        "memcpy",       NULL,   (void*)my_memcpy},
        {"",        "ioctl",        NULL,   (void*)my_ioctl},
//      {"",        "poll",         NULL,   (void*)my_poll},
//      {"libc.so", "read",         NULL,   (void*)my_libc_read},
//      {"libc.so", "getaddrinfo",  NULL,   (void*)my_libc_getaddrinfo},
//      {"libc.so", "connect",      NULL,   (void*)my_libc_connect},
//      {"libc.so", "dlopen",       NULL,   (void*)my_libc_dlopen},
//      {"libc.so", "fork",         NULL,   (void*)my_libc_fork},
//      {"libc.so", "execvp",       NULL,   (void*)my_libc_execvp},
//      {"libc.so", "close",        NULL,   (void*)my_libc_close},
//      {"libc.so", "write",        NULL,   (void*)my_libc_write},
//      {"libc.so", "read",         NULL,   (void*)my_libc_read},
//      {"libc.so", "fopen",        NULL,   (void*)my_libc_fopen},
//      {"libc.so", "open",         NULL,   (void*)my_libc_open},
//      {"libc.so", "__system_property_read", NULL, (void*)my__system_property_read},
        {"",        "msg_q_rcv",    NULL,   (void*)my_msg_q_rcv},
        {NULL, NULL, NULL, NULL}
};

unsigned dvmRelroStart, dvmRelroSize;

#define FALSE   0
#define  TRUE   1

/* Process memory map data */
struct mem_map {
    unsigned int start;
    unsigned int  end;
    char permission[4];
    char* name;
};

struct mem_map memmap[4096];
int memmap_count;

/* elf file information */
struct soinfo
{
    Elf32_Ehdr *ehdr;
    Elf32_Phdr *phdr;
    int phnum;
    unsigned base;
    unsigned size;

    unsigned *dynamic;


    const char *strtab;
    Elf32_Sym *symtab;

    unsigned *plt_got;

    Elf32_Rel *plt_rel;
    unsigned plt_rel_count;

    Elf32_Rel *rel;
    unsigned rel_count;
};


static int skipchars(const char* str, int i, int skipWhiteSpace)
{
    if (skipWhiteSpace)
    {
        while ( str[i] && ((str[i] == ' ') || (str[i] == '\t'))) i++;
        return i;
    }
    else
    {
        while ( str[i] && ((str[i] != ' ') && (str[i] != '\t'))) i++;
        return i;
    }
}
static int nextField(const char* str, int i)
{
    i = skipchars(str, i, 0);
    i = skipchars(str, i, 1);
    return i;
}
int readMemMap(void)
{
    LOGD("read memory map");
    FILE* fd = fopen("/proc/self/maps", "rb");
    if (fd == NULL) {
        LOGD("fopen error");
    } else {
        LOGD("fopen success fd: %d", fd);
    }
    char line[1024];
    int i;
    memmap_count = 0;
    while (1)
    {
        if (fgets(line, sizeof(line), fd) == NULL) {
            if (feof(fd)) {
                break;
            } else {
                LOGD("ferror: %d", ferror(fd));
            }
        }
        i = strlen(line) - 1;
        while((line[i] == '\r') || (line[i] == '\n')) line[i--] = '\0';

        i = 0;
        sscanf(line, "%8x-%8x", &memmap[memmap_count].start, &memmap[memmap_count].end);
        i = nextField(line, i);

        strncpy(memmap[memmap_count].permission, line + i, 4);
        i = nextField(line, i);

        i = nextField(line, i);
        i = nextField(line, i);
        i = nextField(line, i);

        memmap[memmap_count].name = strdup(line + i);
        //LOGD("start: %8x, end: %8x, perm: %s, name: %s", memmap[memmap_count].start, memmap[memmap_count].end, memmap[memmap_count].permission, memmap[memmap_count].name);
        memmap_count++;
        //LOGD("memmap_count: %d", memmap_count);
    }
    if (feof(fd)) {
        LOGD("end of file");
    } else {
        LOGD("not end of file");
    }
    LOGD("fclose");
    fclose(fd);

    LOGD("read memory map return");
    return memmap_count;
}

void printMemMap(void)
{
    int i;
    for (i = 0; i < memmap_count; i++)
    {
        LOGD("%.8X-%.8X   %.4s   %s", memmap[i].start, memmap[i].end, memmap[i].permission, memmap[i].name);
    }

}

int load_elf(Elf32_Ehdr* ehdr, struct soinfo* si)
{
    LOGD("load elf");
    memset(si, 0, sizeof(*si));

    si->base = (unsigned) ehdr;
    si->ehdr = ehdr;
    si->phdr = (Elf32_Phdr *)((unsigned char *)ehdr + ehdr->e_phoff);
    si->phnum = ehdr->e_phnum;
    si->dynamic = (unsigned *)-1;

    int is_exec = (ehdr->e_type == ET_EXEC);
    if (is_exec)
        si->base = 0;

    Elf32_Phdr * phdr = si->phdr;
    int phnum = si->phnum;
    for(; phnum > 0; --phnum, ++phdr) {
        if (phdr->p_type == PT_DYNAMIC) {
            if (si->dynamic != (unsigned *)-1) {
                LOGD("multiple PT_DYNAMIC segments found. "
                      "Segment at 0x%08x, previously one found at 0x%08x",
                      si->base + phdr->p_vaddr,
                      (unsigned)si->dynamic);
                return FALSE;
            }
            si->dynamic = (unsigned *) (si->base + phdr->p_vaddr);
        }
        else if (phdr->p_type == PT_LOAD) {
            if (phdr->p_vaddr + phdr->p_memsz > si->size)
                si->size = phdr->p_vaddr + phdr->p_memsz;
        }
    }


    if (si->dynamic == (unsigned *)-1) {
        LOGE("missing PT_DYNAMIC?!");
        return FALSE;
    }

    LOGD("dynamic = %p\n", si->dynamic);
    LOGD("test1");
    //LOGD("*dynamic = %d\n", *(si->dynamic));
    //LOGD("test2");
    unsigned *d;
    /* extract useful information from dynamic section */
    for(d = si->dynamic; *d; d++){
        LOGD("d = %p, d[0] = 0x%08x d[1] = 0x%08x\n", d, d[0], d[1]);
        switch(*d++){
        case DT_STRTAB:
            si->strtab = (const char *) (si->base + *d);
            break;
        case DT_SYMTAB:
            si->symtab = (Elf32_Sym *) (si->base + *d);
            break;
        case DT_PLTREL:
            LOGD("DT_PLTREL");
            if(*d != DT_REL) {
                // Only support implicit addends for relocation table
                LOGE("DT_RELA not supported");
                return FALSE;
            }
            break;

        case DT_JMPREL:
            si->plt_rel = (Elf32_Rel*) (si->base + *d);
            LOGD("DT_JMPREL %.8X", si->plt_rel);
            break;
        case DT_PLTRELSZ:
            si->plt_rel_count = *d / 8;
            LOGD("DT_PLTRELSZ %.8X", si->plt_rel_count);
            break;
//        case DT_RELA:
//           LOGE("DT_RELA not supported");
//           return FALSE;
        case DT_REL:
            si->rel = (Elf32_Rel*) (si->base + *d);
            LOGD("DT_REL %.8X", si->rel);
            break;
        case DT_RELENT:
            if (*d != 8)
            {
                LOGE("DT_RELENT != 8 !");
                return FALSE;
            }
            break;
        case DT_RELSZ:
            si->rel_count = *d / 8;
            LOGD("DT_RELSZ %.8X", si->rel_count);
            break;
        case DT_PLTGOT:
            /* Save this in case we decide to do lazy binding. We don't yet. */
            si->plt_got = (unsigned *)(si->base + *d);
            LOGD("DT_PLTGOT %.8X", si->plt_got);
            break;
        case DT_TEXTREL:
            /* TODO: make use of this. */
            /* this means that we might have to write into where the text
             * segment was loaded during relocation... Do something with
             * it.
             */
            LOGD("Text segment should be writable during relocation.");
            break;
        }
    }
    //LOGD("test2");

    LOGD("si->strtab = %p, si->symtab = %p\n",
            si->strtab, si->symtab);

    if((si->strtab == 0) || (si->symtab == 0)) {
        LOGE("missing essential tables");
        return FALSE;
    }
    return TRUE;
}

// Returns if addr belongs to si
int containsAddr(struct soinfo* si, unsigned addr)
{
    return (addr >= si->base) && (addr - si->base < si->size);
}
/*
 * Replace symbolic references of SymbolName in module si with newAddr
 * if *oldAddr != 0, only hook places where the old value equals to *oldAddr
 * Otherwise substitute the value at first occurrence into oldAddr.
 * if oldAddr == 0 then do not apply this check at all.
 * Returns the number of hooked places.
 */
int patchReloc(struct soinfo* si,  const char* SymbolName, unsigned* pOldAddr, unsigned NewAddr)
{
    LOGD("patchReloc");
    Elf32_Sym *symtab = si->symtab;
    const char *strtab = si->strtab;
    unsigned idx, i, patched_count = 0;
    unsigned SymbolValue = 0;
    int temp;
    if (pOldAddr)
        SymbolValue = *pOldAddr;


    Elf32_Rel* reloc_entry[] = {si->rel, si->plt_rel};
    unsigned reloc_count[] = {si->rel_count, si->plt_rel_count};
    for(i=0;i<2;i++)
    {
        //LOGD("i: %d", i);
        Elf32_Rel * rel = reloc_entry[i];
        if (!rel) continue;
        //LOGD("reloc_count[%d]: %d", i, reloc_count[i]);
        for (idx = 0; idx < reloc_count[i]; ++idx, ++rel) {

            unsigned type = ELF32_R_TYPE(rel->r_info);
            unsigned sym = ELF32_R_SYM(rel->r_info);
            unsigned reloc = (unsigned)(rel->r_offset + si->base);
            unsigned sym_addr = 0;
            char *sym_name = NULL;
            if(sym != 0) {
                sym_name = (char *)(strtab + symtab[sym].st_name);
                unsigned reloc_v =  *((unsigned*)reloc);

                // Check if this is the relocation we want to patch
                // First compare if the old value matches the target (succeed by default if we don't know the target value yet.)
                // Then do the string comparison to be sure.
                if ( (pOldAddr == 0) || (SymbolValue == 0) || (reloc_v == SymbolValue) )
                {

                    //temp = strcmp(SymbolName, sym_name);
                    //LOGD("temp: %d", temp);
                    if (!strcmp(SymbolName, sym_name))
                    {
                        //LOGD("test if");
                        SymbolValue = reloc_v;
                        *((unsigned*)reloc) = NewAddr;
                        patched_count++;
                    }
                    else
                    {
                        //LOGD("test else");
                        if ((!SymbolValue) && reloc_v == SymbolValue )
                            LOGD("sym_name mismatch: %s, %.8x", sym_name, reloc_v);
                    }
                }
            }
/*
            switch(type){
            case R_ARM_JUMP_SLOT:
                LOGE("R_ARM_JUMP_SLOT %s %.8x", sym_name, reloc);
//              *((unsigned*)reloc) = sym_addr;
                break;
            case R_ARM_GLOB_DAT:
                LOGE("R_ARM_GLOB_DAT %s %.8x", sym_name, reloc);
//              *((unsigned*)reloc) = sym_addr;
                break;
            case R_ARM_ABS32:
                LOGE("R_ARM_ABS32 %s %.8x", sym_name, reloc);
//            *((unsigned*)reloc) += sym_addr;
                break;
            case R_ARM_REL32:
                LOGE("R_ARM_REL32 %s %.8x", sym_name, reloc);
//            *((unsigned*)reloc) += sym_addr - rel->r_offset;
                break;
            case R_ARM_RELATIVE:
//              if(sym){
//                  DL_ERR("%5d odd RELATIVE form...", pid);
//                  return -1;
//              }
                LOGE("R_ARM_RELATIVE %s %.8x", sym_name, reloc);
//              *((unsigned*)reloc) += si->base;
                break;
            case R_ARM_COPY:
                LOGE("R_ARM_COPY %s %.8x", sym_name, reloc);
//              memcpy((void*)reloc, (void*)sym_addr, s->st_size);
                break;
            case R_ARM_NONE:
                LOGE("R_ARM_NONE %s %.8x", sym_name, reloc);
                break;
            default:
                LOGE("UNKNOWN Reloc %s %d %.8x", sym_name, type, reloc);
                break;
            }
*/
            //LOGD("idx: %d", idx);
        } // for idx in reloc
    } // for i in [0,1]
    if (pOldAddr)
        *pOldAddr = SymbolValue;
    return patched_count;
}
int hookAll()
{
    LOGD("hook all");
    struct hook_func* hc;
    int i;
    int total_count = 0;
    for (hc = hook_funcs; hc->so_filename; hc++)
    {
        hc->original_ptr = NULL;
    }
    for (i = 0; i < memmap_count; i++)
    {
        Elf32_Ehdr* ehdr = (Elf32_Ehdr*)memmap[i].start;
        LOGD("Processing %s(%.8X).", memmap[i].name, memmap[i].start);
        if(!memmap[i].name[0]) continue;
        if(memmap[i].permission[0] != 'r')
        {
            LOGE("No permission.");
            continue;
        }
        if(!IS_ELF(*ehdr))
        {
            LOGE("Bad header.");
            continue;
        }
        struct soinfo si;
        if (!load_elf(ehdr, &si))
        {
            LOGE("Cannot parse elf file.");
            continue;
        }
        if (containsAddr(&si, (unsigned)containsAddr))
        {
            LOGD("Skip patching self.");
            continue;
        }

        // Handle RELRO
        LOGD("Handle RELRO");
        int relro_start = 0, relro_size = 0;
        int j;
        for (j = i + 1; j < memmap_count; j++) {
            if ((unsigned)si.plt_got >= memmap[j].start && (unsigned)si.plt_got < memmap[j].end && memmap[j].permission[1] != 'w') {
                relro_start = memmap[j].start;
                relro_size = memmap[j].end - relro_start;
                if (mprotect((void*)relro_start, relro_size, PROT_READ | PROT_WRITE)) {
                    LOGE("Cannot unprotect RELRO.");
                }
                //libdvm's data section is only restored after hookReflectionMethods
                if (!strcmp(memmap[j].name, "/system/lib/libdvm.so")) {
                    dvmRelroStart = relro_start;
                    dvmRelroSize = relro_size;
                    relro_start = relro_size = 0;
                }
                break;
            }
        }

        LOGD("patch");
        for (hc = hook_funcs; hc->so_filename; hc++)
        {
            //LOGD("hook_func_str: %s, hc->func_name: %s", hook_func_str, hc->func_name);
            //if (strcmp(hook_func_str, hc->func_name) == 0) {
                int pcount = patchReloc(&si, hc->func_name, (unsigned *)&hc->original_ptr, (unsigned)hc->hook_ptr);
                LOGD("Hooking %s (%d places patched.)", hc->func_name, pcount);
                total_count += pcount;
                if (pcount)
                    LOGD("hookAll: %s %s.", memmap[i].name, hc->func_name);
            //}
        }

        // Restore relro
        LOGD("Restore relro");
        if (relro_start && relro_size) {
            mprotect((void*)relro_start, relro_size, PROT_READ);
        }

    }
    LOGD("hookAll: %d places patched.", total_count);
    return TRUE;
}

int update(char * a) {

}

int hook_entry(char * a){
    LOGD("Hook success, pid=%d, uid=%d, euid=%d, gid=%d, egid=%d\n", getpid(), getuid(), geteuid(), getgid(), getegid());
    LOGD("Hook %s\n", a);
    hook_func_str = a;
    LOGD("readMemMap: %d", readMemMap());
    LOGD("after read before hook");
    hookAll();
    return 0;
}
