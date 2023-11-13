#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#pragma pack(1)    //!!!改变结构体的对齐方式，防止读取错位

#define pi 3.14159265358
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffsets;
}BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t Planners;
    uint16_t biBitCount;     //decide whether use the palette or not
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;    //2^bitcount
    uint32_t biClrImportant;
}BITMAPINFORMATIONHEADER;

///////////////////////////

//把要进行输出的数组与padding相结合


void rgb_dataOut(unsigned char* out_data, unsigned char* rgb_data, int width, int height, int bytesCount, int padding )
{
    for(int i = 0; i < (width * bytesCount + padding) * height; i++) out_data[i] = 0;
    
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            out_data[bytesCount * x + y * (width * bytesCount + padding) + 0] = rgb_data[bytesCount * (x + y * width) + 0]; //b
            out_data[bytesCount * x + y * (width * bytesCount + padding) + 1] = rgb_data[bytesCount * (x + y * width) + 1]; //g
            out_data[bytesCount * x + y * (width * bytesCount + padding) + 2] = rgb_data[bytesCount * (x + y * width) + 2]; //r
        }
        
    }   
}

void writeBMP(const char* file_name, unsigned char* out_data, int width, int height, int bytesCount, int padding, BITMAPFILEHEADER* bitmapfileheader, BITMAPINFORMATIONHEADER* bitmapinformationheader)
{
    FILE* fp;
    fp = fopen(file_name, "wb");
    if(fp == NULL){
        printf("write gray image error!");
        return ;
    }
    
    //更新文件信息
    bitmapinformationheader->biSizeImage = height * (width * bytesCount + padding);
    bitmapfileheader->bfSize = 54 + bitmapinformationheader->biSizeImage;

    bitmapinformationheader->biWidth = width;
    bitmapinformationheader->biHeight = height;


    fwrite(bitmapfileheader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(bitmapinformationheader, sizeof(BITMAPINFORMATIONHEADER), 1, fp);
    fwrite(out_data, (width * bytesCount + padding) * height, 1, fp);  
    fclose(fp);

}

void printBitmapFileHeader(BITMAPFILEHEADER fileHeader) {
    printf("bfType: %02x\n", fileHeader.bfType);
    printf("bfSize: %d\n", fileHeader.bfSize);
    printf("bfReserved1: %hu\n", fileHeader.bfReserved1);
    printf("bfReserved2: %hu\n", fileHeader.bfReserved2);
    printf("bfOffsets: %u\n", fileHeader.bfOffsets);
}

void printBitmapInfoHeader(BITMAPINFORMATIONHEADER infoHeader) {
    printf("biSize: %02x\n", infoHeader.biSize);
    printf("biWidth: %d\n", infoHeader.biWidth);
    printf("biHeight: %d\n", infoHeader.biHeight);
    printf("Planners: %hu\n", infoHeader.Planners);
    printf("biBitCount: %hu\n", infoHeader.biBitCount);
    printf("biCompression: %u\n", infoHeader.biCompression);
    printf("biSizeImage: %d\n", infoHeader.biSizeImage);
    printf("biXPelsPerMeter: %u\n", infoHeader.biXPelsPerMeter);
    printf("biYPelsPerMeter: %u\n", infoHeader.biYPelsPerMeter);
    printf("biClrUsed: %u\n", infoHeader.biClrUsed);
    printf("biClrImportant: %u\n", infoHeader.biClrImportant);
}

void translation(unsigned char *rgb_data, int width, int height, int bytesCount, int delta_x, int delta_y, BITMAPFILEHEADER bitmapfileheader, BITMAPINFORMATIONHEADER bitmapinformationheader)
{
    
    //剩下的用白色填补
    int new_width = width + delta_x;
    int new_height = height + delta_y;
    int new_padding = (4-(new_width * bytesCount) % 4) % 4;
    //int padding = (4-(width * bytesCount) % 4) % 4;
    
    unsigned char *new_rgb_data = malloc(new_width * new_height * bytesCount);
    for(int i = 0; i < new_width * new_height * bytesCount; i++) new_rgb_data[i] = 255;  //背景色为白
    
    //映射关系：
    //(x,y) -> (x',y')  x' = x + delta_x, y' = y + delta_y
    //3 * (x + y * w) -> 3*(x' + y' * (w + delta_x))

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            new_rgb_data[bytesCount * ((x+delta_x) + (y + delta_y) * new_width) + 0] = rgb_data[bytesCount * (x + y * width) + 0];
            new_rgb_data[bytesCount * ((x+delta_x) + (y + delta_y) * new_width) + 1] = rgb_data[bytesCount * (x + y * width) + 1];
            new_rgb_data[bytesCount * ((x+delta_x) + (y + delta_y) * new_width) + 2] = rgb_data[bytesCount * (x + y * width) + 2];
        }
        
    }
    //printf("OK!\n");  死在循环里面了  原来是一开始x y的条件写成了<new,这里做映射范围还是原来的
    
    unsigned char *out_data = malloc(new_height * (new_width * bytesCount + new_padding));
    rgb_dataOut(out_data, new_rgb_data, new_width, new_height, bytesCount, new_padding);

    writeBMP("translation.bmp", out_data, new_width, new_height, bytesCount, new_padding, &bitmapfileheader, &bitmapinformationheader);
    
    free(new_rgb_data);
    free(out_data);
}

void mirrorX(unsigned char *rgb_data, int width, int height, int bytesCount, int padding, BITMAPFILEHEADER bitmapfileheader, BITMAPINFORMATIONHEADER bitmapinformationheader)
{
    //（x,y) -> (x, -y + height -1)
    unsigned char *new_rgb_data = malloc(width * height * bytesCount);
    for(int i = 0; i < width * height * bytesCount; i++) new_rgb_data[i] = 255;  //背景色为白

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            new_rgb_data[bytesCount * (x + (-y + height -1) * width) + 0] = rgb_data[bytesCount * (x + y * width) + 0];
            new_rgb_data[bytesCount * (x + (-y + height -1) * width) + 1] = rgb_data[bytesCount * (x + y * width) + 1];
            new_rgb_data[bytesCount * (x + (-y + height -1) * width) + 2] = rgb_data[bytesCount * (x + y * width) + 2];
        }
    }

    unsigned char *out_data = malloc(height * (bytesCount * width + padding));
    rgb_dataOut(out_data, new_rgb_data, width, height, bytesCount, padding);
    writeBMP("mirrorX.bmp", out_data, width, height, bytesCount, padding, &bitmapfileheader, &bitmapinformationheader);
    
    free(new_rgb_data);
    free(out_data);
}

void mirrorY(unsigned char *rgb_data, int width, int height, int bytesCount, int padding, BITMAPFILEHEADER bitmapfileheader, BITMAPINFORMATIONHEADER bitmapinformationheader)
{
    //（x,y) -> (x, -y + height -1)
    unsigned char *new_rgb_data = malloc(width * height * bytesCount);
    for(int i = 0; i < width * height * bytesCount; i++) new_rgb_data[i] = 255;  //背景色为白

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            new_rgb_data[bytesCount * (-x + width - 1 + y * width) + 0] = rgb_data[bytesCount * (x + y * width) + 0];
            new_rgb_data[bytesCount * (-x + width - 1 + y * width) + 1] = rgb_data[bytesCount * (x + y * width) + 1];
            new_rgb_data[bytesCount * (-x + width - 1 + y * width) + 2] = rgb_data[bytesCount * (x + y * width) + 2];
        }
    }

    unsigned char *out_data = malloc(height * (bytesCount * width + padding));
    rgb_dataOut(out_data, new_rgb_data, width, height, bytesCount, padding);
    writeBMP("mirrorY.bmp", out_data, width, height, bytesCount, padding, &bitmapfileheader, &bitmapinformationheader);
    
    free(new_rgb_data);
    free(out_data);
}

void shearX(unsigned char *rgb_data, int width, int height, int bytesCount, double a, BITMAPFILEHEADER bitmapfileheader, BITMAPINFORMATIONHEADER bitmapinformationheader)
{
    //x = x + a * y
    int new_width = (int)(width + a * height) + 1;
    int new_height = height;
    int new_padding = (4-(new_width * bytesCount) % 4) % 4;

    unsigned char *new_rgb_data = malloc(new_width * new_height * bytesCount);
    for(int i = 0; i < new_width * new_height * bytesCount; i++) new_rgb_data[i] = 255;  //背景色为白

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            new_rgb_data[bytesCount * (x + (int)(a * y) + y * new_width) + 0] = rgb_data[bytesCount * (x + y * width) + 0];
            new_rgb_data[bytesCount * (x + (int)(a * y) + y * new_width) + 1] = rgb_data[bytesCount * (x + y * width) + 1];
            new_rgb_data[bytesCount * (x + (int)(a * y) + y * new_width) + 2] = rgb_data[bytesCount * (x + y * width) + 2];
        }
    }

    unsigned char *out_data = malloc(new_height * (bytesCount * new_width + new_padding));

    rgb_dataOut(out_data, new_rgb_data, new_width, new_height, bytesCount, new_padding);
    writeBMP("shearX.bmp", out_data, new_width, new_height, bytesCount, new_padding, &bitmapfileheader, &bitmapinformationheader);

    free(new_rgb_data);
    free(out_data);
}

void shearY(unsigned char *rgb_data, int width, int height, int bytesCount, double a, BITMAPFILEHEADER bitmapfileheader, BITMAPINFORMATIONHEADER bitmapinformationheader)
{
    //y = y + a * x
    int new_width = width;
    int new_height = height + (int)(a * width);
    int new_padding = (4-(new_width * bytesCount) % 4) % 4;

    unsigned char *new_rgb_data = malloc(new_width * new_height * bytesCount);
    for(int i = 0; i < new_width * new_height * bytesCount; i++) new_rgb_data[i] = 255;  //背景色为白

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            new_rgb_data[bytesCount * (x + (int)(y + a * x) * new_width) + 0] = rgb_data[bytesCount * (x + y * width) + 0];
            new_rgb_data[bytesCount * (x + (int)(y + a * x) * new_width) + 1] = rgb_data[bytesCount * (x + y * width) + 1];
            new_rgb_data[bytesCount * (x + (int)(y + a * x) * new_width) + 2] = rgb_data[bytesCount * (x + y * width) + 2];
        }
    }

    unsigned char *out_data = malloc(new_height * (bytesCount * new_width + new_padding));

    rgb_dataOut(out_data, new_rgb_data, new_width, new_height, bytesCount, new_padding);
    writeBMP("shearY.bmp", out_data, new_width, new_height, bytesCount, new_padding, &bitmapfileheader, &bitmapinformationheader);

    free(new_rgb_data);
    free(out_data);
}

void rotate(unsigned char *rgb_data, int width, int height, int bytesCount, double a, BITMAPFILEHEADER bitmapfileheader, BITMAPINFORMATIONHEADER bitmapinformationheader)
{
    //y = y + a * x
    int new_width = (int)(width * fabs(cos(a)) + height * fabs(sin(a))) + 1;
    int new_height = (int)(width * fabs(sin(a)) + height * fabs(cos(a))) + 1;
    int new_padding = (4-(new_width * bytesCount) % 4) % 4;
    
    unsigned char *new_rgb_data = malloc(new_width * new_height * bytesCount);
    for(int i = 0; i < new_width * new_height * bytesCount; i++) new_rgb_data[i] = 255;  //背景色为白
    
    //x'= xcosa - ysina  y'= xsina + ycosa
    
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            int new_x = x * cos(a) - y * sin(a) + height * sin(a);
            int new_y = x * sin(a) + y * cos(a);
            //为什么这里用double就出问题 rgb是在每个像素的基础之上，我们要首先保证每个像素的坐标正确，即先对坐标取整，否则进行三个通道分配的时候会出问题
            //printf("int: new_x = %d, new_y = %d\n", new_x, new_y);
            //printf("double: new_x = %.2lf, new_y = %.2lf\n", x * cos(a) - y * sin(a) + height * sin(a), x * sin(a) + y * cos(a));

            new_rgb_data[bytesCount * (new_x + new_y * new_width) + 0] = rgb_data[bytesCount * (x + y * width) + 0];
            new_rgb_data[bytesCount * (new_x + new_y * new_width) + 1] = rgb_data[bytesCount * (x + y * width) + 1];
            new_rgb_data[bytesCount * (new_x + new_y * new_width) + 2] = rgb_data[bytesCount * (x + y * width) + 2];

            //printf("int: index = %d  ", bytesCount * (new_x + new_y * new_width));
            //printf("double: index = %lf\n", bytesCount * ((x * cos(a) - y * sin(a) + height * sin(a)) + (x * sin(a) + y * cos(a)) * new_width));
            //差好多
        }
    }
    //此时我们发现有好多空洞
    //进行逆变换插值

    for(int y = 0; y < new_height; y++)
    {
        for(int x = 0; x < new_width; x++)
        {
            int old_x = (x - height * sin(a))  * cos(2 * pi - a) - y * sin(2 * pi - a);
            int old_y = (x - height * sin(a)) * sin(2 * pi - a) + y * cos(2 * pi - a);
            
            if (old_x < 0 || old_x >= width || old_y < 0 || old_y >=height) continue;
            
            int index = bytesCount * (x + y * new_width);            
            if (new_rgb_data[index + 0] == 255 && new_rgb_data[index + 1] ==255 && new_rgb_data[index + 2] == 255)  //空洞
            {
                new_rgb_data[index + 0] = rgb_data[bytesCount * (old_x + old_y * width) + 0];
                new_rgb_data[index + 1] = rgb_data[bytesCount * (old_x + old_y * width) + 1];
                new_rgb_data[index + 2] = rgb_data[bytesCount * (old_x + old_y * width) + 2];
            }
        }
    }

    unsigned char *out_data = malloc(new_height * (bytesCount * new_width + new_padding));

    rgb_dataOut(out_data, new_rgb_data, new_width, new_height, bytesCount, new_padding);
    writeBMP("rotate.bmp", out_data, new_width, new_height, bytesCount, new_padding, &bitmapfileheader, &bitmapinformationheader);

    free(new_rgb_data);
    free(out_data);
}

void scale(unsigned char *rgb_data, int width, int height, int bytesCount, double c, double d, BITMAPFILEHEADER bitmapfileheader, BITMAPINFORMATIONHEADER bitmapinformationheader)
{
    int new_width = (int)(c * width) + 1;
    int new_height = (int)(d * height) + 1;    //+1不要忘了，不然会一会能过一会不能过
    int new_padding = (4-(new_width * bytesCount) % 4) % 4;

    unsigned char *new_rgb_data = malloc(new_width * new_height * bytesCount);
    for(int i = 0; i < new_width * new_height * bytesCount; i++) new_rgb_data[i] = 255;  //背景色为白

    //需要做一个映射

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            int new_x = c * x;
            int new_y = d * y;
    
            new_rgb_data[bytesCount * (new_x + new_y * new_width) + 0] = rgb_data[bytesCount * (x + y * width) + 0];
            new_rgb_data[bytesCount * (new_x + new_y * new_width) + 1] = rgb_data[bytesCount * (x + y * width) + 1];
            new_rgb_data[bytesCount * (new_x + new_y * new_width) + 2] = rgb_data[bytesCount * (x + y * width) + 2];

        }
    }
    
    unsigned char *old_rgb_data = malloc(new_width * new_height * bytesCount);
    //printf("ok");
    int count = 0;
    for(int y = 0; y < new_height; y++)
    {
        // count++;
        // printf("%d", count);
        for(int x = 0; x < new_width; x++)
        {
            int old_x = x / c;
            int old_y = y / d;
            //printf("ok");
            if (old_x < 0 || old_x >= width || old_y < 0 || old_y >=height) continue;
            
            int index = bytesCount * (x + y * new_width);
            if (new_rgb_data[index + 0] == 255 && new_rgb_data[index + 1] ==255 && new_rgb_data[index + 2] == 255)  //空洞
            {
                new_rgb_data[index + 0] = rgb_data[bytesCount * (old_x + old_y * width) + 0];
                new_rgb_data[index + 1] = rgb_data[bytesCount * (old_x + old_y * width) + 1];
                new_rgb_data[index + 2] = rgb_data[bytesCount * (old_x + old_y * width) + 2];
            }    
        }
    }

    //printf("ok");
    unsigned char *out_data = malloc(new_height * (bytesCount * new_width + new_padding));

    rgb_dataOut(out_data, new_rgb_data, new_width, new_height, bytesCount, new_padding);
    writeBMP("scale.bmp", out_data, new_width, new_height, bytesCount, new_padding, &bitmapfileheader, &bitmapinformationheader);
    //printf("success write");
    free(new_rgb_data);
    free(out_data);
}


int main()
{
    FILE* fp;
    fp = fopen("shousi.bmp","rb");      //open the source file in binary form
    if(fp == NULL) {
        printf("Error!");
        exit(0);
    }
    //
    BITMAPFILEHEADER bitmapfileheader;
    BITMAPINFORMATIONHEADER bitmapinformationheader;  //define the head of bmp

    //
    fread(&bitmapfileheader, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&bitmapinformationheader, sizeof(BITMAPINFORMATIONHEADER), 1, fp);  //get the informatin into head struct
    
    //test infomation
    printBitmapFileHeader(bitmapfileheader);
    printBitmapInfoHeader(bitmapinformationheader);
    //check
    if(bitmapfileheader.bfType == 0X4D42)  printf("correct file type!\n");
    else {
        fclose(fp);
        exit(0);
    }
    
    //
    int width = bitmapinformationheader.biWidth;
    int height = bitmapinformationheader.biHeight;

    //
    if(bitmapinformationheader.biSizeImage == 0)bitmapinformationheader.biSizeImage = bitmapinformationheader.biSize - bitmapfileheader.bfOffsets;
    int dataSize = bitmapinformationheader.biSizeImage;

    //
    int bytesCount = bitmapinformationheader.biBitCount/8;
    int padding = (4-(width * bytesCount) % 4) % 4;   //to make the size of a line in the picture 4*k

    printf("padding = %d\n", padding);
    //create a space to store the actually useful data of the image, without the padding
    unsigned char* rgb_data = (unsigned char* ) malloc(width * height * bytesCount);  
    
    
    //get the actually useful data
    for(int i = 0; i < height; i++)
    {
        fread(&rgb_data[i * width * bytesCount], bytesCount, width, fp);   
        fseek(fp, padding, SEEK_CUR);  //jump the padding
    }

    fclose(fp);
 
    //  
      //out_data contains the useful data and paddings
    
    
    //write rgb BMP
    unsigned char* out_data = (unsigned char*)malloc((width * bytesCount + padding) * height);
    rgb_dataOut(out_data, rgb_data, width, height, bytesCount, padding);
    writeBMP("rgb_test.bmp", out_data, width, height, bytesCount, padding, &bitmapfileheader, &bitmapinformationheader);
    
    translation(rgb_data, width, height, bytesCount, 100, 100, bitmapfileheader, bitmapinformationheader);
    
    mirrorX(rgb_data, width, height, bytesCount, padding, bitmapfileheader, bitmapinformationheader);
    mirrorY(rgb_data, width, height, bytesCount, padding, bitmapfileheader, bitmapinformationheader);
    //为什么在寿司那张图这里一会可以一会不可以
    //发现y变换+width忘记-1了


    shearX(rgb_data, width, height, bytesCount, 0.5, bitmapfileheader, bitmapinformationheader);
    shearY(rgb_data, width, height, bytesCount, 0.5, bitmapfileheader, bitmapinformationheader);

    rotate(rgb_data, width, height, bytesCount, pi/6, bitmapfileheader, bitmapinformationheader);
    
    scale(rgb_data, width, height, bytesCount, 0.5, 0.5, bitmapfileheader, bitmapinformationheader);


}

