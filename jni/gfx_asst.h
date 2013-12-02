/*
 * gfx_asst.h
 *
 *  Created on: 2013/09/30
 *      Author: Michael
 */

#ifndef GFX_ASST_H_
#define GFX_ASST_H_







#define MAXSIZE  1024 * 1024 * 4

static unsigned char g_bmpbuffer[MAXSIZE];





#pragma pack(push,1)

typedef struct tagBITMAPFILEHEADER {
    unsigned short bfType;
    unsigned int   bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
    unsigned int   biSize;
    int            biWidth;
    int            biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    int            biXPixPerMeter;
    int            biYPixPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImporant;
} BITMAPINFOHEADER;

#pragma pack(pop)

typedef struct {
    int  fsize;
    unsigned char *pdata;    // �摜�t�@�C���̃s�N�Z���f�[�^
    unsigned char *TexData;  // �e�N�X�`���̃s�N�Z���f�[�^
    BITMAPFILEHEADER *bmpheader;
    BITMAPINFOHEADER *bmpinfo;
    int  BmpSize;
    int  BmpOffBits;
    int  BmpWidth;           // �摜�̕�
    int  BmpHeight;          // �摜�̍����i���Ȃ�Δ��]�j

    float bitmap_ratio;

    float alpha;

    int  BmpBit;             // �摜�̃r�b�g�[�x
    int  BmpLine;
//    int  initial_alpha;
    GLuint  texname;
} texture_type;


typedef struct {

	int size;
	char* path;
	unsigned char* buffer;
	texture_type tt;
} texture_file;

extern texture_file textures[];
extern int sizeof_textures_array;
extern int sizeof_textures;

void setup_texture(texture_file *tf, float init_alpha);
int load_bitmap(char *filename, void *buffer);
int check_bitmap(texture_type *tt, void* buffer);
void make_texture(texture_type *tt, int alpha);


#endif /* GFX_ASST_H_ */
