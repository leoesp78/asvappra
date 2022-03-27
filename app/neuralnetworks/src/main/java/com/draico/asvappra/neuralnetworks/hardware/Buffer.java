package com.draico.asvappra.neuralnetworks.hardware;

import android.hardware.HardwareBuffer;

public class Buffer {
    private int currentBuffer;
    private boolean isCreated;
    public static long USAGE_CPU_READ_NEVER = 0;
    public static long USAGE_CPU_READ_RARELY = 2;
    public static long USAGE_CPU_READ_OFTEN = 3;
    public static long USAGE_CPU_READ_MASK = 15;
    public static long USAGE_CPU_WRITE_NEVER = 0 << 4;
    public static long USAGE_CPU_WRITE_RARELY = 2 << 4;
    public static long USAGE_CPU_WRITE_OFTEN = 3 << 4;
    public static long USAGE_CPU_WRITE_MASK = 15 << 4;
    public static long USAGE_GPU_SAMPLED_IMAGE = 15;
    public static long USAGE_GPU_FRAMEBUFFER = 1 << 9;
    public static long USAGE_GPU_COLOR_OUTPUT = 1 << 9;
    public static long USAGE_COMPOSER_OVERLAY = 1 << 11;
    public static long USAGE_PROTECTED_CONTENT = 1 << 14;
    public static long USAGE_VIDEO_ENCODE = 1 << 16;
    public static long USAGE_SENSOR_DIRECT_DATA = 1 << 23;
    public static long USAGE_GPU_DATA_BUFFER = 1 << 24;
    public static long USAGE_GPU_CUBE_MAP = 1 << 25;
    public static long USAGE_GPU_MIPMAP_COMPLETE = 1 << 26;
    public static long USAGE_VENDOR_0 = 1 << 28;
    public static long USAGE_VENDOR_1 = 1 << 29;
    public static long USAGE_VENDOR_2 = 1 << 30;
    public static long USAGE_VENDOR_3 = 1 << 31;
    public static long USAGE_VENDOR_4 = 1 << 48;
    public static long USAGE_VENDOR_5 = 1 << 49;
    public static long USAGE_VENDOR_6 = 1 << 50;
    public static long USAGE_VENDOR_7 = 1 << 51;
    public static long USAGE_VENDOR_8 = 1 << 52;
    public static long USAGE_VENDOR_9 = 1 << 53;
    public static long USAGE_VENDOR_10 = 1 << 54;
    public static long USAGE_VENDOR_11 = 1 << 55;
    public static long USAGE_VENDOR_12 = 1 << 56;
    public static long USAGE_VENDOR_13 = 1 << 57;
    public static long USAGE_VENDOR_14 = 1 << 58;
    public static long USAGE_VENDOR_15 = 1 << 59;
    public static long USAGE_VENDOR_16 = 1 << 60;
    public static long USAGE_VENDOR_17 = 1 << 61;
    public static long USAGE_VENDOR_18 = 1 << 62;
    public static long USAGE_VENDOR_19 = 1 << 63;
    public static native Buffer getBuffer();
    public static native Buffer allocate(BufferDescription bufferDescription);
    public static native Buffer fromHardwareBuffer(HardwareBuffer hardwareBuffer);
    public native BufferDescription getDescription();
    public static native boolean isSupported(BufferDescription bufferDescription);
    public native void delete();
    public native void lock(Object memoryVirtual, BufferArea bufferArea, long usageType, int fence);
    public native void lockAndGetInfo(Object memoryVirtual, BufferArea bufferArea, int usageType, int fence, int bytesPerPixel, int bytesPerStride);
    public native HardwareBuffer getHardwareBuffer();
    public native Plane[] lockPlanes(BufferArea bufferArea, long usageType, int fence);
    public native void sendHandleToUnixSocket(int sockedFd);
    public native void unlock(int fence);
}