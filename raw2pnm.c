#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define GRAY  0x40
#define RED   0x44
#define GREEN 0x48
#define BLUE  0x4c

off_t file_size(const char *path) {
    struct stat st;
    stat(path, &st);
    return st.st_size;
}

void convert_ppm(unsigned int width, unsigned int height,
        FILE *raw_fp, char *name) {
    strcpy(strrchr(name, '.'), ".ppm");
    FILE *ppm_fp = fopen(name, "wb");
    fprintf(stderr, "Writing %s\n", name);
    fprintf(ppm_fp, "P6\n%d %d\n255\n", width, height);

    char *buf;
    int n;
    char h[3];
    char *r = NULL, *g = NULL, *b = NULL;
    int red_width, green_width, blue_width;

    fseek(raw_fp, 0, SEEK_SET);

    while (1) {
	memset(h, 0, sizeof(h));
    	n = fread(h, 1, 3, raw_fp);
    	if (n < 3) {
	    if (feof(raw_fp)) {
	    	fprintf(stderr, "Reached end of file. Done\n");
            	fclose(ppm_fp);
            	return;
	    }		
	
            fprintf(stderr, "Failed to read header, EOF\n");
	    fclose(ppm_fp);
	    return;
    	}

	unsigned int width = (h[2] << 8) + h[1];
        buf = malloc(width);
        if (buf == NULL) {
            fprintf(stderr, "Failed to allocate buffer\n");
	    fclose(ppm_fp);
	    return;
        }

        n = fread(buf, 1, width, raw_fp);
        if (n < width) {
            fprintf(stderr, "Unable to read a full line. Wanted to read %d but got only %d\n", width, n);
            fclose(ppm_fp);
            return;
        }

        switch (h[0]) {
            case RED:
                if (r != NULL) {
                    fprintf(stderr, "Got a new red line without that the previous red line was not written out. This implies an illegal file. Aborting!\n");
                    fclose(ppm_fp);
                    return;
                }
                r = buf;
                red_width = width;
                break;
 
            case GREEN:
                if (g != NULL) {
                    fprintf(stderr, "Got a new green line without that the previous green line was not written out. This implies an illegal file. Aborting!\n");
                    fclose(ppm_fp);
                    return;
                }
                g = buf;
                green_width = width;
                break;

            case BLUE:
                if (b != NULL) {
                    fprintf(stderr, "Got a new blue line whithout that the previous blue line was not written out. This implies an illegal file. Aborting!\n");
                    fclose(ppm_fp);
                    return;
                }
                b = buf;
                blue_width = width;
                break;

            default:
                fprintf(stderr, "Invalid header: 0x%02X\n", h[0]);
                fclose(ppm_fp);
                return;
        }

        if (r && g && b) {
            if (red_width != green_width || red_width != blue_width) {
                fprintf(stderr, "red, green and blue widths were different: %d, %d, %d\n", red_width, green_width, blue_width);
                fclose(ppm_fp);
            }

            for (int i = 0; i < red_width; i++) {
	//	printf("Writing pixel %d\n", i);
    		fputc(r[i], ppm_fp);
                fputc(g[i], ppm_fp);
                fputc(b[i], ppm_fp);
            }

            free(r);
            r = NULL;
            red_width = 0;
            free(g);
            g = NULL;
            green_width = 0;
            free(b);
            b = NULL;
            blue_width = 0;
        }
    }
}

void convert_pgm(unsigned int width, unsigned int height,
        FILE *raw_fp, char *name) {
    strcpy(strrchr(name, '.'), ".pgm");
    FILE *pgm_fp = fopen(name, "wb");
    fprintf(stderr, "Writing %s\n", name);
    fprintf(pgm_fp, "P5\n%d %d\n255\n", width, height);

    char h[3], g[width];
    for(int i = 0; i < height; i++) {
        fread(h, 1, 3, raw_fp);
        fread(g, 1, width, raw_fp);
        fwrite(g, 1, width, pgm_fp);
    }
    fclose(pgm_fp);
}

int main(int argc, char **argv) {
    for(int i = 1; i < argc; i++) {
        char *name = argv[i];
        FILE *raw_fp = fopen(name, "rb");
	int n;

	if (raw_fp == NULL) {
		fprintf(stderr, "Failed to open %s\n", name);
		continue;
	}

        unsigned char buf[3];
        n = fread(buf, sizeof(unsigned char), 3, raw_fp);
	if (n < 3) {
	    fprintf(stderr, "File %s is too small\n", name);
	    fclose(raw_fp);
	    continue;
	}

	unsigned char type = buf[0];
        unsigned int width = (buf[2] << 8) + buf[1];
        unsigned int height = file_size(name) / (width + 3);

	fprintf(stderr, "Calculated width to %d columns and and height to %d rows\n", width, height); 

        if(strchr("DHL", type)) { /* color */
	    fprintf(stderr, "Converting raw to color image\n");
            convert_ppm(width, height/3, raw_fp, name);
	} else if(type == '@') { /* gray */
	    fprintf(stderr, "Converting raw to gray scale image\n");
            convert_pgm(width, height, raw_fp, name);
	} else {
            fprintf(stderr, "ERROR: '%s' has unrecognised type '%c'\n", name, type);
        }
	fclose(raw_fp);
    }
    return 0;
}
