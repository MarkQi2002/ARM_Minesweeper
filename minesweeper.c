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

/* Mode Of Operation */
#define SVC_MODE 0x00000013
#define INT_ENABLE 0b01000000

/* Interrupt controller (GIC) CPU interface(s) */
#define ENABLE                0x00000001
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg

/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs

// Constants For Information Row
#define INFORMATION_ROW_HEIGHT 10

// Constants For Game Frame
#define GAME_FRAME_UPPER_Y 10
#define GAME_FRAME_BOTTOM_Y 240
#define GAME_FRAME_LEFT_X 0
#define GAME_FRAME_RIGHT_X 320
#define GAME_FRAME_WIDTH 10

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
void draw_square(int x0, int y0, int size, short int box_color);
void draw_rectangle(int x0, int y0, int x1, int y1, short int rectangle_color);
void plot_pixel(int x, int y, short int pixel_color);

void draw_game_frame();

void config_GIC(void);

// Data Structure Declaration

int main(void)
{   
    // Variable Declaration For Time Keeping


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
    draw_game_frame();
    
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    draw_game_frame();
    
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
void draw_square(int x0, int y0, int size, short int box_color){
	int row_num, col_num;
    for (row_num = 0; row_num < size; row_num++){
        for (col_num = 0; col_num < size; col_num++){
            plot_pixel(x0 + col_num, y0 + row_num, box_color);
        }
    }
}

// Function For Drawing A Rectangle
void draw_rectangle(int x0, int y0, int x1, int y1, short int rectangle_color){
    // Important Information (x0 < x1, y0 < y1)
    int row_num, col_num;
    for (row_num = y0; row_num < y1; row_num++){
        for (col_num = x0; col_num < x1; col_num++){
            plot_pixel(col_num, row_num, rectangle_color);
        }
    }
}

// Function Used For Drawing A Single Pixel On the Screen
void plot_pixel(int x, int y, short int pixel_color){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = pixel_color;
}

// Function Used For Drawing A Frame Around The Game
void draw_game_frame(){
    // Top Row And Bottom Row
    draw_rectangle(GAME_FRAME_LEFT_X, GAME_FRAME_UPPER_Y, GAME_FRAME_RIGHT_X, GAME_FRAME_UPPER_Y + GAME_FRAME_WIDTH, GREY);
    draw_rectangle(GAME_FRAME_LEFT_X, GAME_FRAME_BOTTOM_Y - GAME_FRAME_WIDTH, GAME_FRAME_RIGHT_X, GAME_FRAME_BOTTOM_Y, GREY);

    // Left Column And Right Column
    draw_rectangle(GAME_FRAME_LEFT_X, GAME_FRAME_UPPER_Y + GAME_FRAME_WIDTH, GAME_FRAME_LEFT_X + GAME_FRAME_WIDTH, GAME_FRAME_BOTTOM_Y - GAME_FRAME_WIDTH, GREY);
    draw_rectangle(GAME_FRAME_RIGHT_X - GAME_FRAME_WIDTH, GAME_FRAME_UPPER_Y + GAME_FRAME_WIDTH, GAME_FRAME_RIGHT_X, GAME_FRAME_BOTTOM_Y - GAME_FRAME_WIDTH, GREY);
}

// Turn On Interrupts in the ARM processor
void enable_A9_interrupts(void){
    int status = SVC_MODE | INT_ENABLE;
    asm("msr cpsr, %[ps]" : : [ps] "r"(status));
}

// Function Used For Configuring The Generic Interrupt Controller (GIC)
void config_GIC(void){
    // Used To Calculate Register Addresses
    int address;

    // Configure the FPGA interval timer and KEYS interrupts
    *((int *) 0xFFFED848) = 0x00000101;
    *((int *) 0xFFFED108) = 0x00000300;

    // Set Interrupt Priority Mask Register (ICCPMR). Enable Interrupts of all priorities
    address = MPCORE_GIC_CPUIF + ICCPMR;
    *((int *) address) = 0xFFFF;

    // Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts
    address = MPCORE_GIC_CPUIF + ICCICR;
    *((int *) address) = ENABLE;

    // Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
    address = MPCORE_GIC_CPUIF + ICDDCR;
    *((int *) address) = ENABLE;
}