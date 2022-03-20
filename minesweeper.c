/* This files provides address values that exist in the system */
#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00
#define BLACK 0x0000

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0
#define TRUE 1

// Include Important Libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// Begin part3.c code for Lab 7
volatile int pixel_buffer_start; // global variable

// Funciton Decleration
void wait_for_vsync();
void clear_screen();
void swap(int *xp, int *yp);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void draw_box(int x0, int y0, short int box_color);
void plot_pixel(int x, int y, short int pixel_color);

// Data Structure Declaration

int main(void)
{
    // Variable Declaration
    srand(time(NULL));

    // Pointer To The VGA Controller Register
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    while (1)
    {
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

// Function For Waiting Vertical Sychronization
void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *) 0xFF203020;
    volatile int * pixel_status_register = (int *)0xFF20302C;

    * pixel_ctrl_ptr = 1;
    int status = *pixel_status_register;
    // Wait Until The S Bit In The Status Register Turns Into Zero
    while ((status & 0x00000001) != 0){
        status = *pixel_status_register;
    }
}

// Function For Clearing/Resetting The Entire Screen
void clear_screen(){
	int row_num, col_num;
    for (row_num = 0; row_num < RESOLUTION_Y; row_num++){
        for (col_num = 0; col_num < RESOLUTION_X; col_num++){
            plot_pixel(col_num, row_num, BLACK);
        }
    }
}

// Helper Function For Swapping Two Integer Variables (Used In draw_line)
void swap(int *xp, int *yp){
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// Function For Drawing A Straight Line Using Bresenham's Algorithm
void draw_line(int x0, int y0, int x1, int y1, short int line_color){
    bool is_steep = abs(y1 - y0) > abs(x1 - x0);
    if (is_steep){
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1){
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    // Variable Decleration
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
    int y_step;
    
    // Determine Positive Or Negative Y Increment
    if (y0 < y1) y_step = 1;
    else y_step = -1;
	
    // Loop To Draw The Line
	int x;
    for (x = x0; x < x1; x++){
        if (is_steep) plot_pixel(y, x, line_color);
        else plot_pixel(x, y, line_color);

        // Determining Increment Y Or Not
        error = error + deltay;
        if (error > 0){
            y = y + y_step;
            error = error - deltax;
        }
    }
}

// Function For Drawing A Box
void draw_box(int x0, int y0, short int box_color){
	int row_num, col_num;
    for (row_num = -1; row_num <= 1; row_num++){
        for (col_num = -1; col_num <= 1; col_num++){
            plot_pixel(x0 + col_num, y0 + row_num, box_color);
        }
    }
}

// Function Used For Drawing A Single Pixel On the Screen
void plot_pixel(int x, int y, short int pixel_color){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = pixel_color;
}
