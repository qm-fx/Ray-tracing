#pragma once
#include <cstdint>
#include <fstream>
#include <vector_types.h>
#include <cuda_runtime.h>
#include <vector>
//#include <windows.h>

#pragma pack(push, 1) // 严格1字节对齐
struct BMPHeader {
    uint16_t signature = 0x4D42;      // 'BM'
    uint32_t fileSize;                // 文件总大小
    uint32_t reserved = 0;            // 保留字段
    uint32_t dataOffset = 54;         // 像素数据偏移量
    uint32_t headerSize = 40;         // 信息头大小
    int32_t  width;                   // 图像宽度
    int32_t  height;                  // 图像高度（负值表示从上到下存储）
    uint16_t planes = 1;              // 颜色平面数
    uint16_t bpp = 24;                // 每像素位数（24位色）
    uint32_t compression = 0;         // 压缩方式（0=不压缩）
    uint32_t imageSize;               // 像素数据大小
    int32_t  xPixelsPerMeter = 0;     // 水平分辨率
    int32_t  yPixelsPerMeter = 0;     // 垂直分辨率
    uint32_t colorsUsed = 0;          // 实际使用的颜色数
    uint32_t importantColors = 0;     // 重要颜色数
};
#pragma pack(pop) // 恢复默认对齐

void saveBMP(const char* filename, uchar3* pixels, int width, int height) {
    // 1. 准备BMP头
    BMPHeader header;
    header.width = width;
    header.height = height;
    header.fileSize = sizeof(BMPHeader) + width * height * 3; // RGB格式

    // 2.写入BMP文件(BMP要求BGR顺序且无Alpha)
    uint8_t* rgb_data = (uint8_t*)malloc(width * height * 3*sizeof(uint8_t));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx =y * width +x;
            rgb_data[idx*3 + 2] = pixels[idx].x; // B
            rgb_data[idx*3 + 1] = pixels[idx].y; // G
            rgb_data[idx*3 + 0] = pixels[idx].z; // R
        }
    }

    // 3. 写入文件
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<char*>(&header), sizeof(header));
    file.write(reinterpret_cast<char*>(rgb_data), width * height * 3);
}

    //void displayBMP(HDC hdc) 
    //{
    //    BITMAPINFO bmi{};
    //    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    //    bmi.bmiHeader.biWidth = width;
    //    bmi.bmiHeader.biHeight = -height; // 负值表示从上到下存储
    //    bmi.bmiHeader.biPlanes = 1;
    //    bmi.bmiHeader.biBitCount = 24;

    //    SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height,
    //        pixels.data(), &bmi, DIB_RGB_COLORS);
    //}

    /*void display() {
        HDC hdc = GetDC(NULL);
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0,
            0, height, &pixels[0], &bmi, DIB_RGB_COLORS);
        ReleaseDC(NULL, hdc);
    }*/

