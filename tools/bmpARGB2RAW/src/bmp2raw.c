#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "klist/klist.h"

#define DEBUG	0

#define	LIST		"list.txt"
#define	INBITMAP	"in/"
#define	OUTRAW		"../output/out/"
#define	HFILE		"h/"
#define	HFILE_NAME	"parking_line.h"

#define	LIST_RESULT	"result.txt"

#define WORD	unsigned short
#define	DWORD	unsigned int
#define	LONG	unsigned int
#define BYTE	unsigned char

#define	WIDTHBYTES(bits)	((bits + 31) / 32 * 4)

typedef	struct tagBITMAPFILEHEADER {
	WORD	bfType;
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	DWORD	biSize;
	LONG	biWidth;
	LONG	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	LONG	biXPelsPerMeter;
	LONG	biYPerlsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	BYTE	rgbBlue;
	BYTE	regGreen;
	BYTE	regRed;
	BYTE	rebReserved1;
} RGBQUAD;

struct list_data {
	char raw_file[512];
	unsigned int width;
	unsigned int height;
};

FILE *in = NULL, *out = NULL, *list_out = NULL;

int SaveRawFromFilelist(struct list *file_list, int *list_count);
int SaveBitmap32ToRaw(char *file_name, DWORD *width, DWORD *height);
void MakeH_Data(struct list *file_list, int list_count);

int main(int argc, char *argv[])
{
	int list_count = 0;
	struct list *file_list = NULL;

	file_list  = (struct list *)malloc(sizeof(struct list));
	if (!file_list) {
		printf("failed to memory alloction!!\n");
		return -2;
	}

	init(file_list);

	if (SaveRawFromFilelist(file_list, &list_count)) {
		printf("Raw file save error!!\n");
		return -1;
	}

	MakeH_Data(file_list, list_count);

	delete_all(file_list);

	if (file_list)
		free(file_list);

	return 0;
}

int SaveRawFromFilelist(struct list *file_list, int *list_count)
{
	unsigned int i;
	int ret = 0;

	FILE *list = NULL;

	char file[256], file_name[256];
	char line[256];

	DWORD width = 0, height = 0;

	struct list_data *data;

	if (!(list = fopen(LIST, "r"))) {
		printf("failed to file open! : %s\n", LIST);
		return -1;
	}

	if (!(list_out = fopen(LIST_RESULT, "w"))) {
		printf("failed to file open! : %s\n", LIST_RESULT);
		return -1;
	}

	fprintf(list_out, "[file name]\t[width]\t[height]\t[bits]\t[description]\n");

	while (fscanf(list, "%s", line) != EOF) {
#if DEBUG
		printf("start : %s\n", line);
#endif
		strcpy(file_name, line);
		for (i = 0; i<strlen(file_name); i++)
			if ( file_name[i] == '.')
				break;
		file_name[i] = '\0';

		strcpy(file, INBITMAP);
		strcat(file, line);

		if ((in = fopen(file, "rb")) == NULL) {
			printf("input file(%s) open fail!", file);
			exit(1);
		}

		fprintf(list_out, "%s", file);
		ret = SaveBitmap32ToRaw(file_name, &width, &height);
		if (ret < 0)
			fprintf(list_out, "\tNot support File format!\n");
		else
			fprintf(list_out, "\n");

		if (in)
			fclose(in);

		if (ret >= 0) {
			data = (struct list_data *)malloc
				(sizeof(struct list_data));
			sprintf(data->raw_file, "%s.raw", file_name);
			data->width = width;
			data->height = height;

			*list_count = insert(file_list, (void *)data);
		}
	}

	if (list)
		fclose(list);

	if (list_out)
		fclose(list_out);

	return 0;
}

int SaveBitmap32ToRaw(char *file_name, DWORD *width, DWORD *height)
{
	BITMAPFILEHEADER hf;
	BITMAPINFOHEADER hInfo;
	BYTE *bmp;
	int i, j;
	char file[256];

	fread(&hf.bfType, sizeof(WORD), 1, in);
	fread(&hf.bfSize, sizeof(DWORD), 1, in);
	fread(&hf.bfReserved1, sizeof(WORD), 1, in);
	fread(&hf.bfReserved2, sizeof(WORD), 1, in);
	fread(&hf.bfOffBits, sizeof(DWORD), 1, in);

#if DEBUG
	printf("%x %x %x %x %x\n", hf.bfType, hf.bfSize, hf.bfReserved1,
			hf.bfReserved2, hf.bfOffBits);
#endif
	if (hf.bfType != 0x4D42) {
		printf("This file is not match bitmap type code('BM')\n");
		return -1;
	}
	fread(&hInfo.biSize, sizeof(DWORD), 1, in);
	fread(&hInfo.biWidth, sizeof(LONG), 1, in);
	fread(&hInfo.biHeight, sizeof(LONG), 1, in);
	fread(&hInfo.biPlanes, sizeof(WORD), 1, in);
	fread(&hInfo.biBitCount, sizeof(WORD), 1, in);
	fread(&hInfo.biCompression, sizeof(DWORD), 1, in);

#if DEBUG
	printf("%d %d %d %d %d %d\n", hInfo.biSize, hInfo.biWidth,
			hInfo.biHeight, hInfo.biPlanes, hInfo.biBitCount,
			hInfo.biCompression);
#endif

	fprintf(list_out, "\t%d\t%d\t%d",
			hInfo.biWidth,
			hInfo.biHeight,
			hInfo.biBitCount
			);

	if (hInfo.biBitCount != 32)
		return -2;

	strcpy(file, OUTRAW);
	strcat(file, file_name);
	strcat(file, ".raw");

	if ((out = fopen(file, "wb")) == NULL) {
		printf("output file(%s) oepn fail!", file);
		return -3;
	}

	*width = hInfo.biWidth;
	*height = hInfo.biHeight;

	int width_size = WIDTHBYTES(32 * hInfo.biWidth);
	bmp = (BYTE *)malloc(width_size * hInfo.biHeight * 4);

	fseek(in, hf.bfOffBits, SEEK_SET);

	for (i = 0; (DWORD)i < hInfo.biHeight; i++)
		for (j = 0; j < width_size; j++)
			fread(&bmp[i * width_size + j], 1, 1, in);

	for (i = hInfo.biHeight - 1; i >= 0; i--)
		for (j = 0; (DWORD)j < hInfo.biWidth * 4; j += 4) {
			fwrite(&bmp[i * width_size + j + 3], 1, 1, out);
			fwrite(&bmp[i * width_size + j + 2], 1, 1, out);
			fwrite(&bmp[i * width_size + j + 1], 1, 1, out);
			fwrite(&bmp[i * width_size + j], 1, 1, out);
		}

	free(bmp);

	if (out)
		fclose(out);

	return 0;
}

void MakeH_Data(struct list *file_list, int list_count)
{
	FILE *fin = NULL, *fout = NULL;
	char file[256], name[256], resolution[256];
	int i, j, l, tmp, size;
	unsigned char ca, cr, cg, cb;
	unsigned int width = 0;
	unsigned int height = 0;
	struct list_data *data;
	int exist_dir;

	exist_dir = access( HFILE, 0);
	if (exist_dir)
		mkdir(HFILE, 0755);

	sprintf(file, "%s%s", HFILE, HFILE_NAME);

	if ((fout = fopen(file, "w+")) == NULL) {
		printf("%s file write fail!!\n", file);
		return;
	}

	sprintf(resolution, "%s", "parkingline_resolution");
	fprintf(fout, "const unsigned int %s[%d][2] = {\n",
				resolution, list_count);

	for (i = 0; i < list_count; i++) {
		data = (struct list_data *)get_data(file_list, i + 1);

		width = data->width;
		height = data->height;

		fprintf(fout, "\t{");
		fprintf(fout, " %d, %d ", width, height);
		fprintf(fout, "}\n");

		if (i != (list_count - 1))
			fprintf(fout, ",\n");
	}
	fprintf(fout, "};\n\n");


	for (i = 0; i < list_count; i++) {
		data = (struct list_data *)get_data(file_list, i + 1);
#if DEBUG
		printf("file name : %s\n", data->raw_file);
#endif
		width = data->width;

		strcpy(file, OUTRAW);
		strcat(file, data->raw_file);
		for (l = (strlen(file)-1); l >= 0; l--)
			if (file[l] == '.') break;
		file[l] = '\0';
		strcat(file, ".raw");

		sprintf(name, "%s", data->raw_file);
		for (l = (strlen(name)-1); l >= 0; l--)
			if (name[l] == '.') break;
		name[l] = '\0';

		fprintf(fout, "const unsigned int parkingline_%s[] = {\n",
				name);
		if ((fin = fopen(file, "rb")) == NULL) {
#if DEBUG
			printf("%s file read fail!!\n", file);
#endif
			continue;
		}

		fseek(fin, 0L, SEEK_END);
		size = ftell(fin);
		rewind(fin);

		//fprintf(fout, "\t{\n");
		fprintf(fout, "\t");

		for (j = 0; j < (size / 4); j++) {
			fread(&ca, 1, 1, fin);
			fread(&cr, 1, 1, fin);
			fread(&cg, 1, 1, fin);
			fread(&cb, 1, 1, fin);

			tmp = ((ca << 24) | (cr << 16) | (cg << 8) | cb);

			fprintf(fout, "0x%08X", tmp);

			if (size != ((j + 1) * 4))
				fprintf(fout, ", ");

			if ((((DWORD)j + 1) % width) == 0) {
				if (j != ((size / 4) - 1))
					fprintf(fout, "\n\t");
			}
		}
		//fprintf(fout, "\n\t}");

		if (i != (list_count - 1))
			fprintf(fout, ",\n");

		fclose(fin);

		fprintf(fout, "\n};\n");
	}

	if (fout)
		fclose(fout);
}
