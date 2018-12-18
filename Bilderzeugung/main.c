#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "ftfont.h"

void write_file(Bitmap *b, char *filename) {	// zyklisch durchummerieren des Dateinamens (ohne ".bmp")
	static short counter = 0;
	char outfile[30];

	sprintf(outfile, "%s%1d.bmp", filename, counter%3);

	// Endung bestimmt Format (hier Bitmap)
	bm_save(b, outfile);
	++counter;
}

void print_therm(Bitmap *d, int fsize, int x0, int y0) {
  int h = fsize;
  int r_bcircle = h / 8;
  int r_redcircle = r_bcircle / 3;
  int b = 4 * r_bcircle / 3;


  //circle coordinates
  int x_circle = x0 - r_bcircle - 10;
  int y_bcircle = y0 - r_bcircle;
  int y_tcircle = y0 - (h - b / 2);

  //line coordinates
  int x_l_line = x_circle - b / 2;
  int x_r_line = x_circle + b / 2;
  int y_b_line = y0 - r_bcircle - sqrt(pow(r_bcircle, 2) - pow(b / 2, 2));
  int y_t_line = y0 - (h - b / 2);

	//short line
	int y_b_shline = y_b_line - (y_b_line - y_t_line) / 16;
	int x_l_shline = x_r_line - b / 4;

  // drawing circle & lines with coordinates
  bm_fillcircle(d, x_circle, y_bcircle, r_bcircle);
  //bm_line(d, x_l_line, y_b_line, x_l_line, y_t_line);
  //bm_line(d, x_r_line, y_b_line, x_r_line, y_t_line);
	bm_fillrect(d, x_l_line, y_b_line, x_r_line, y_t_line);
  bm_fillcircle(d, x_circle, y_tcircle, (b / 2));

  bm_set_color(d, bm_atoi("black"));

  //deleting parts of the circle
	bm_fillcircle(d, x_circle, y_bcircle, (r_bcircle - 2));
	bm_fillrect(d, (x_l_line) + 2, y_b_line, (x_r_line - 2), y_t_line);
	bm_fillcircle(d, x_circle, y_tcircle, (b / 2 - 2));
  bm_fillcircle(d, x_circle, (y_bcircle - r_bcircle), (b / 2 - 2));
  bm_fillcircle(d, x_circle, (y_tcircle + b / 2), (b / 2 - 2));
  bm_fillcircle(d, x_circle, (y_tcircle + b / 4), (b/2 - 2));

	//drawing short lines
	bm_set_color(d, bm_atoi("white"));

	for(int i = 0; i < 8; i++) {
		bm_fillrect(d, x_l_shline, y_b_shline, x_r_line, y_b_shline - 3);

		y_b_shline = y_b_shline - (y_b_line - y_t_line) / 8;
	}

  bm_set_color(d, bm_atoi("red"));

  bm_fillcircle(d, x_circle, y_bcircle, r_redcircle);
}

void print_hum(Bitmap *b, int x0, int y0, int hum) {
  char str[80];

  sprintf(str, "%d%%", hum);
  bm_prinf(b, x0, y0, "%s", str);

}

int main(int argc, char *argv[]) {
  int currentTemp = 0;  //measured temperature
  int printTemp = 0;  //last printed temperature

  int currentHum = 0; //measured humidity
  int printHum = 0; //last printed humidity

  if(currentTemp /= printTemp || currentHum /= printHum) { //if a value changed reprint the whole picture
    printTemp = currentTemp;
    printHum = currentHum;

    bm_free(p);
  }
}
